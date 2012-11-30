//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "SiChenWaveApplLayer.h"
#include "SiChenMac1609_4.h"
#include "ApplPkt_m.h"
#include <stdlib.h>
#include "Consts80211p.h"

const simsignalwrap_t SiChenWaveApplLayer::pktLostSignal = simsignalwrap_t(
        "packetLoss");
const simsignalwrap_t SiChenWaveApplLayer::SCHChannelBusyTime50msSignal =
        simsignalwrap_t("SCHChannelBusyTime50ms"); //sent from MAC layer (SiChenMac!609_4 line 69)

Define_Module(SiChenWaveApplLayer);

void SiChenWaveApplLayer::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);

    if (stage == 0) {
        lowerLayer2450In = findGate("lowerLayer2450In");
        lowerLayer2450Out = findGate("lowerLayer2450Out");
        lowerControl2450In = findGate("lowerControl2450In");
        lowerControl2450Out = findGate("lowerControl2450Out");

        myMacWifi2450 = FindModule<Mac80211MultiChannel*>::findSubModule(
                findHost());
        if (!myMacWifi2450) {
            opp_error("Could not find Mac80211MultiChannel.");
        }
        myWifi2450ChannelVecRecord.setName("myWifi2450Channel");
        myCurrentSCHVecRecord.setName("myCurrentSCH number");

        enableDsrcSCH = true;
        enableWifiSCH = false;

        /** Subscribe to signals */
        // subscribe to pkt loss signal emitted from Mac80211MultichannelWithSignal
        findHost()->subscribe(pktLostSignal, this);
        //
        findHost()->subscribe(SCHChannelBusyTime50msSignal, this);

        receivedWifi2450InterferPkts = 0;
        receivedWifi2450InterferPktsVecRecord.setName("receivedWifi2450InterferPktsVecRecord");
        receivedWifi2450Pkts = 0;
        receivedWifi2450PktsVecRecord.setName("receivedWifi2450Pkts");
        lostWifi2450Pkts = 0;
        lostWifi2450PktsVecRecord.setName("lostWifi2450Pkts");

        receivedBeacons = 0;
        receivedBeacons_last = 0;
        receivedSCHAnnounceBeacons = 0;
        receivedSCHAnnounceBeacons_last = 0;
        sentSCHAnnounceBeacons = 0;
        sentSCHAnnounceBeacons_last = 0;
        receivedDataNo = 0;
        receivedData_last = 0;

        recordingInterval = par("recordingInterval").doubleValue();
        lastRecordingTIme = 0;
        receivedDataVecRecord.setName("receivedDataCount");
        receivedBeaconVecRecord.setName("receivedBeaconCount");
        receivedSCHAnnounceBeaconsVecRecord.setName("received SCH announcements");
        sentSCHAnnounceBeaconsVecRecord.setName("sent SCH Announcements");

        /** Initialize channel selector */
        channSelector.initialize(int(par("lowestSCH")), int(par("highestSCH")),
                par("channelSelectionMode").stringValue(),
                par("explorationRate_epsilon").doubleValue(),
                par("learningRate_alpha").doubleValue(),
                par("dbFileName").str(),
                true);

//        /** Initialize database */
//        if (par("enableRealSniffingData").boolValue()) {
//            db = new SqliteAccess(
//                    "/home/summer2012/channel_sweeping_sniffering.sqlite3");
//        } else
//        {
//            db = 0;
//        }

        myIndex = findHost()->getIndex();
        iAmFrontCar = par("iAmFrontCar").boolValue();
        rearCarId = myId + 1; //TODO fix rcvId to car id collected from beacons

        if (iAmFrontCar) {
            announceSCHEvt = new cMessage("SCH announcement",
                    ANNOUNCE_SCH_EVT);
            scheduleAt(simTime() + 25e-3, announceSCHEvt);
        }

        sendRelayMsg = par("sendRelayMsg").boolValue();
        if (sendRelayMsg) {
            sendRelayMsgEvt = new cMessage("relay msg evt",
                    SEND_RELAY_MSG_EVT);
            relayMsgInterval = par("relayMsgInterval").doubleValue();
            relayMsgLengthBits = par("relayMsgLengthBits");
            relayMsgName = "sequential relay";
            currentRelayMsgSerial = 0;

            if (iAmFrontCar) {
                WaveAppEV
                        << "Will send relay msg every " << relayMsgInterval
                                << "s." << endl;
                scheduleAt(simTime() + relayMsgInterval, sendRelayMsgEvt);
            }

        }
    } else if (stage == 1) {

        setCurrentSCH(par("initSCH").longValue());

    }
}

/** set current SCH to either DSRC or WiFi */
void SiChenWaveApplLayer::setCurrentSCH(int ch) {
    this->myCurrentSCH = ch;
    if (ch > 14) {
        enableDsrcSCH = true;
        enableWifiSCH = false;
        myMac->changeServiceChannel(ch); //wave channels
    } else {
        enableDsrcSCH = false;
        enableWifiSCH = true;
        myMacWifi2450->switchChannel(ch); //wifi2450 channels
    }

    myCurrentSCHVecRecord.record(myCurrentSCH);
}

/** General function for sending WSM in app layer, add random APP delay
 * WSMs don't have IP!!!
 * WAVE Short Messages (WSM), providing an efficient
 * WAVE-specific alternative to IP that can be directly
 * supported by applications
 */
void SiChenWaveApplLayer::sendWSM(WaveShortMessage* wsm) {
    switch (wsm->getKind()) {
    case RELAY_MSG:
        sendDelayedDownToChannel(wsm, uniform(500e-6, 5e-3), DSRC_CHANNEL);
        break;
    case SWITCH_CHANN_BEACON:
        iAmFrontCar ?
                sendDelayedDownToChannel(wsm, uniform(500e-6, 3e-3),
                        DSRC_CHANNEL) :
                sendDelayedDownToChannel(wsm, uniform(1e-3, 5e-3),
                        DSRC_CHANNEL);
        break;
    case BEACON:
        sendDelayedDownToChannel(wsm, uniform(100e-6, 200e-6), DSRC_CHANNEL);
        break;
    case BEACON_REPLY:
        sendDelayedDownToChannel(wsm, uniform(100e-6, 1e-3), DSRC_CHANNEL);
        break;
    case CCH_MSG:
    case SCH_MSG:
        sendDelayedDownToChannel(wsm, uniform(100e-6, 200e-6), DSRC_CHANNEL);
        break;
    default:
        ASSERT2(false, "WSM kind not configured for.");
        break;
    }
}

void SiChenWaveApplLayer::sendIpPacket(cPacket *pkt) {
    assert(pkt->getKind()!=CCH_MSG);
    //CCH should not use IP, but use WSMP
    sendDelayedDownToChannel(pkt, uniform(100e-6, 200e-6),
            myCurrentSCH >= 172 ? DSRC_CHANNEL : WIFI_CHANNEL);
}

void SiChenWaveApplLayer::sendDelayedDownToChannel(cMessage* msg,
        simtime_t_cref delay, SiChenWaveApplLayerChannelKinds channKind) {
    recordPacket(PassedMessage::OUTGOING, PassedMessage::LOWER_DATA, msg);
    WaveAppEV
                    <<
                    "sendDelayedDownToChannel ("
                    << ((channKind == DSRC_CHANNEL) ?
                            "DSRC" :
                            ((channKind == WIFI_CHANNEL) ? "WIFI" : "TVWS"))
                    << "): ClassName:" << msg->getClassName() << ", msg Name:"
                    << msg->getName() << endl;

    switch (channKind) {
    case DSRC_CHANNEL:
        sendDelayed(msg, delay, lowerLayerOut); //send msg via DSRC NIC
        break;
    case WIFI_CHANNEL:
        sendDelayed(msg, delay, lowerLayer2450Out); //send msg via DSRC NIC
        break;
    case TVWS_CHANNEL:
    default:
        assert(false);
        break;
    }
}

void SiChenWaveApplLayer::onBeacon(WaveShortMessage* wsm) {

    switch (wsm->getKind()) {
    case BEACON:
        receivedBeacons++;
        WaveAppEV
                << "Received beacon priority  " << wsm->getPriority() << " at "
                        << simTime() << ". receivedBeacons= " << receivedBeacons
                        << std::endl;

        if (sendData) {
            t_channel channelForData = dataOnSch ? type_SCH : type_CCH;
            sendWSM(
                    prepareWSM("data", dataLengthBits, channelForData,
                            dataPriority, wsm->getSenderAddress(), 2,
                            BEACON_REPLY));
        }
        break;
    case SWITCH_CHANN_BEACON: //receive SWITCH_CHANN_BEACON from other nodes
        receivedSCHAnnounceBeacons++;
//        receivedSCHAnnounceBeaconsVecRecord.record(receivedSCHAnnounceBeacons);
        WaveAppEV
                << "Received SWITCH_CHANN_BEACON to CH "
                        << atoi(wsm->getWsmData()) << " from car (id="
                        << wsm->getSenderAddress() << ")" << endl;
        /* if coming from leading cars, forward to rear car and reply to front*/
        if (wsm->getSenderAddress() < this->myId) {

            WaveAppEV << "Broadcast SWITCH_CHANN_BEACON" << endl;

            frontCarSCH = atoi(wsm->getWsmData());

            this->setCurrentSCH(frontCarSCH); // following car changes SCH

            announceSchNo(atoi(wsm->getWsmData())); // following car broadcasts to following cars
        }

//        // display received serial on the car icon
//        if (ev.isGUI()) {
//            cDisplayString& dispStr = this->getParentModule()->getDisplayString();
//            sprintf(displayChar, "new SCH: %d", frontCarSCH);
//            dispStr.setTagArg("t", 0, displayChar);
//        }
        break;
    default:
        opp_error("Unknown received beacon wsm type: %d", wsm->getKind());
        break;
    }
}

void SiChenWaveApplLayer::onData(WaveShortMessage* wsm) {
    int recipientId = wsm->getRecipientAddress();

    if (wsm->getArrivalGateId() == lowerLayer2450In) {
        WaveAppEV
                << "Received a WiFi2450 " << wsm->getClassName()
                        << "on channel " << myMacWifi2450->getChannel()
                        << " from SrcAddr " << wsm->getSenderAddress()
                        << " of size " << wsm->getBitLength() << "." << ""
                        << endl;

        receivedWifi2450Pkts++;

        WaveAppEV <<"receivedWifi2450Pkts=" << receivedWifi2450Pkts << endl;
    } else {
        WaveAppEV
                << "Received a DSRC " << wsm->getClassName()
                        << " from SrcAddr " << wsm->getSenderAddress()
                        << " of size " << wsm->getBitLength() << "." << ""
                        << endl;
    }

    if (recipientId == myId) {
        WaveAppEV
                << "Received data priority  " << wsm->getPriority() << " at "
                        << simTime() << ". receivedDataNo= " << receivedDataNo
                        << std::endl;

        switch (wsm->getKind()) {

        case SCH_MSG: // received a relay message from other cars
            receivedDataNo++;

            // if the "sequential relay" app is on, forward what I received after 5ms
            if (sendRelayMsg) {
                currentRelayMsgSerial = wsm->getSerial();
                // following cars only relay
                WaveAppEV
                        << "Car " << myIndex << " relays msg with serial "
                                << currentRelayMsgSerial << " to " << rearCarId
                                << " on Ch " << myCurrentSCH << endl;
                WaveAppEV<<"WSM data: " << wsm->getWsmData() << endl;

                WaveShortMessage* newWsm = prepareWSM(relayMsgName,
                        relayMsgLengthBits, type_SCH, dataPriority,
                        this->rearCarId, currentRelayMsgSerial, SCH_MSG);
                newWsm->setWsmData(
                        strcat(const_cast<char *>(wsm->getWsmData()), "/**"));
                sendIpPacket(newWsm);

            }
            break;
        default:
            opp_error("Unknown received data wsm type: %d", wsm->getKind());
            break;
        }
    } else {
        WaveAppEV
        << "Got msg sent to id=" << recipientId << ", myId is" << myId << endl;
    }

    delete wsm;
}

void SiChenWaveApplLayer::onWifi2450Data(cMessage* msg) {

    assert(false);

}

/** @brief direct different msg from different gates to handle*Msg */
void SiChenWaveApplLayer::handleMessage(cMessage* msg) {
    if (msg->getArrivalGateId() == lowerLayer2450In) {
        recordPacket(PassedMessage::INCOMING, PassedMessage::LOWER_DATA, msg);
        handleLowerMsg(msg);
    } else if (msg->getArrivalGateId() == this->lowerControl2450In) {
        recordPacket(PassedMessage::INCOMING, PassedMessage::LOWER_CONTROL,
                msg);
        handleLowerMsg(msg);
    } else
        BaseLayer::handleMessage(msg);
}

void SiChenWaveApplLayer::announceSchNo(int ch) {
    WaveShortMessage* newWsm;
    newWsm = prepareWSM("Announce SCH", beaconLengthBits, type_CCH,
            beaconPriority, -1, ch, SWITCH_CHANN_BEACON);
    char buffer[10];
    sprintf(buffer, "%d", ch);
    newWsm->setWsmData(buffer);
    sendWSM(newWsm);
    sentSCHAnnounceBeacons++;

}

void SiChenWaveApplLayer::handleSelfMsg(cMessage* msg) {
    WaveShortMessage* newWsm;
    switch (msg->getKind()) {
    case SEND_BEACON_EVT:

        WaveAppEV << "Car " << myIndex << " broadcasts beacon." << endl;

        sendWSM(
                prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority,
                        -1, -1, BEACON));
        scheduleAt(simTime() + par("beaconInterval").doubleValue(),
                sendBeaconEvt);

        break;

    case ANNOUNCE_SCH_EVT: //TODO combine this and beacon
        ASSERT(iAmFrontCar);

        WaveAppEV
                << "Car " << myIndex << " announces SCH is "
                        << this->myCurrentSCH << endl;
        announceSchNo(this->myCurrentSCH); // broadcast to following cars
        scheduleAt(simTime() + par("beaconInterval").doubleValue(),
                announceSCHEvt);
        break;

    case SEND_RELAY_MSG_EVT:
        ASSERT(iAmFrontCar);
        // only front car schedules next msg, following cars only relay

        WaveAppEV
                << "Car " << myIndex << "(id=" << myId
                        << ") starts new msg flow with serial "
                        << currentRelayMsgSerial + 1 << " to car (id="
                        << rearCarId << ")" << endl;
        newWsm = prepareWSM(relayMsgName, relayMsgLengthBits, type_SCH,
                dataPriority, rearCarId, currentRelayMsgSerial++, SCH_MSG);

        newWsm->setWsmData("**");
        sendIpPacket(newWsm);
        scheduleAt(simTime() + relayMsgInterval, sendRelayMsgEvt); //schedule when the NEXT msg flow starts

        break;

    default:
        BaseWaveApplLayer::handleSelfMsg(msg); // handle else
        break;

    }
}

/** @brief handle control msg from Mac1609_4
 * decide whether to initiate SCH switching operation in the car group */
void SiChenWaveApplLayer::handleLowerControl(cMessage* msg) {
    ASSERT(false);

    delete msg;
}

/** @brief handle messages from below, which are usually received packets
 * decide what kind of msg it is
 * delete the msg only if NOT relay msgs*/
void SiChenWaveApplLayer::handleLowerMsg(cMessage* msg) {

    if (msg->getArrivalGateId() == this->lowerLayerIn) { // if this msg is a DSRC message

        WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
        if (wsm != NULL) {
            switch (wsm->getKind()) {
            case BEACON:
            case SWITCH_CHANN_BEACON:
                onBeacon(wsm);
                break;
            case BEACON_REPLY:
            case RELAY_MSG:
                WaveAppEV<<"obslete RELAY_MSG msg kind" << endl;
                break;
            case SCH_MSG:
                onData(wsm);
                break;
            case SWITCH_CHANN_BEACON_REPLY:
                WaveAppEV<<"obslete SWITCH_CHANN_BEACON_REPLY msg kind" << endl;
                break;
            default:
                opp_error("Unknown received wsm type: %d", wsm->getKind());
                break;
            }
        } else {
            DBG << "unknown message received\n";
        }

        /* This is app layer already, always discard old msg. if relay, make another new msg*/
        delete (wsm);

    } else { // maybe a 2450 wifi msg
        ASSERT(msg->getArrivalGateId() == this->lowerLayer2450In);
        WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
        if (wsm != NULL) {
            onData(wsm);
        } else {
            receivedWifi2450InterferPkts++;
        }

    }

}

SiChenWaveApplLayer::~SiChenWaveApplLayer() {
//    if (db!=0) this->db->close();
}

void SiChenWaveApplLayer::receiveSignal(cComponent* source,
        simsignal_t signalID, cObject* obj) {

    BaseWaveApplLayer::receiveSignal(source, signalID, obj); // do handlePositionUpdate()

    if (signalID == mobilityStateChangedSignal) {
        WaveAppEV << "mobilityStateChangedSignal received." << endl;

        if (roundedPosition.x == 0 || roundedPosition.y == 0) {
            roundedPosition.x = round(this->curPosition.x / 50) * 50;
            roundedPosition.y = round(this->curPosition.y / 50) * 50;
        } else {
            // round curPosition to roundedPosition
            if (roundedPosition.x != round(this->curPosition.x / 50) * 50
                    || roundedPosition.y != round(this->curPosition.y / 50) * 50) {
                //update channel value database
                Coord oldRoundedPosition = roundedPosition;
                roundedPosition.x = round(this->curPosition.x / 50) * 50;
                roundedPosition.y = round(this->curPosition.y / 50) * 50;

                //                if (iAmFrontCar)
//                    (static_cast<SiChenMac1609_4*>(myMac))->chanupdataChannelValueDB(
//                            oldRoundedPosition, roundedPosition);
                //change to let channel selector update
            }
        }

        if ( simTime() - lastRecordingTIme >= recordingInterval ) {
            lastRecordingTIme = simTime();
            //do statistics
            receivedBeaconVecRecord.record(
                    receivedBeacons - receivedBeacons_last);
            receivedSCHAnnounceBeaconsVecRecord.record(
                    receivedSCHAnnounceBeacons
                            - receivedSCHAnnounceBeacons_last);
            sentSCHAnnounceBeaconsVecRecord.record(
                    sentSCHAnnounceBeacons - sentSCHAnnounceBeacons_last);
            receivedDataVecRecord.record(receivedDataNo - receivedData_last);

            receivedBeacons_last = receivedBeacons;
            receivedData_last = receivedDataNo;
            receivedSCHAnnounceBeacons_last = receivedSCHAnnounceBeacons;
            sentSCHAnnounceBeacons_last = sentSCHAnnounceBeacons;

            if (iAmFrontCar) {// then measure channel busy time and select next channel

//                if (db!=0)
//                {
//                    int time_in_second = floor(simTime().dbl());
//                    char buffer[100];
//                    sprintf(buffer, "SELECT * FROM data_for_simulation WHERE TIME=%d", time_in_second);
//                    db->query(buffer);
//                }

                double normalized_pkts_count = double(receivedWifi2450Pkts) / double(100.0);
                double channBusyRatio=0;
//                for (int i = 0; i <= 9; i++)
//                    channBusyRatio += CrossLayerInfo.SCHBusyTime[i];
                channBusyRatio = CrossLayerInfo.SCHBusyTime[0];
                selectNextSchNumberGivenThisMeasure(channBusyRatio*10);
            }

            receivedWifi2450InterferPktsVecRecord.record(receivedWifi2450InterferPkts);
            receivedWifi2450InterferPkts = 0;
            receivedWifi2450PktsVecRecord.record(receivedWifi2450Pkts);
            receivedWifi2450Pkts = 0;
            lostWifi2450PktsVecRecord.record(lostWifi2450Pkts);
            lostWifi2450Pkts = 0;
        }

        if (iAmFrontCar && fmod((simTime().dbl()), 5) <0.05) {
            this->channSelector.updateChannelValueDB(simTime().dbl(),5);
        }

    } else if (signalID == pktLostSignal) {
        WaveAppEV << "pktLostSignal received." << endl;
        lostWifi2450Pkts++;
    }
}

void SiChenWaveApplLayer::selectNextSchNumberGivenThisMeasure(double measure) {
    int channelNumber;
    channelNumber = channSelector.getNextSCH(myCurrentSCH, measure, ChannSelector::CHANNEL_BUSY_RATIO);
    setCurrentSCH(channelNumber);
    myWifi2450ChannelVecRecord.record(myMacWifi2450->getChannel());
}

void SiChenWaveApplLayer::receiveSignal(cComponent *source,
        simsignal_t signalID, double d) {
    Enter_Method_Silent();
    if (signalID == SCHChannelBusyTime50msSignal) { //every 100ms when switching to CCH
        int idx = int(floor(simTime().dbl()/0.1)) % 10;
        CrossLayerInfo.SCHBusyTime[0] = d;
    }
}

void SiChenWaveApplLayer::finish() {
    recordScalar("receivedBeacons", receivedBeacons);
    recordScalar("receivedData", receivedDataNo);

    this->channSelector.finish();
}
