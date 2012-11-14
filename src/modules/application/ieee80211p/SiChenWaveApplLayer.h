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

#ifndef SICHENWAVEAPPLLAYER_H_
#define SICHENWAVEAPPLLAYER_H_

#include "BaseModule.h"
#include <BaseWaveApplLayer.h>
#include "mobility/traci/TraCIMobility.h"
#include "Mac80211MultiChannel.h"

#include "ChannSelector.h"
//#include "SqliteAccess.h"

#ifndef WaveAppEV
#define WaveAppEV (ev.isDisabled()||!debug) ? ev : ev << "[Car " << myIndex << "]" << getParentModule()->getFullPath() << " - WaveAppEV: "
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath()

class SiChenWaveApplLayer: public BaseWaveApplLayer {
public:

    virtual ~SiChenWaveApplLayer();

    virtual void initialize(int stage);
    virtual void finish();
    enum SiChenWaveApplMessageKinds {
        SEND_RELAY_MSG_EVT = LAST_BASE_WAVE_APPL_MESSAGE_KIND,
        RELAY_MSG,
        ANNOUNCE_SCH_EVT,
        BEACON,
        BEACON_REPLY,
        SWITCH_CHANN_BEACON,
        SWITCH_CHANN_BEACON_REPLY,
        CCH_MSG,
        SCH_MSG
    };
    enum SiChenWaveApplLayerChannelKinds {
        DSRC_CHANNEL = 1, WIFI_CHANNEL, TVWS_CHANNEL
    };
    enum SiChenWaveApplControlKinds {
        PLEASE_SWITCH_SCH = LAST_BASE_APPL_CONTROL_KIND
    };
protected:
    int lowerLayer2450In;
    int lowerLayer2450Out;
    int lowerControl2450In;
    int lowerControl2450Out;
    Mac80211MultiChannel *myMacWifi2450;
    bool enableDsrcSCH;
    bool enableWifiSCH;
    int myCurrentSCH;
    int myIndex;
    char displayChar[64];
    int frontCarSCH;
    int rearCarId;

    bool sendRelayMsg;
    double relayMsgInterval;
    int relayMsgLengthBits;
    std::string relayMsgName;
    int currentRelayMsgSerial;
    bool iAmFrontCar;
    Coord roundedPosition;
    cMessage *sendRelayMsgEvt;
    cMessage *announceSCHEvt;
    uint32_t receivedBeacons, receivedBeacons_last;
    uint32_t receivedSCHAnnounceBeacons, receivedSCHAnnounceBeacons_last;
    uint32_t sentSCHAnnounceBeacons, sentSCHAnnounceBeacons_last;
    uint32_t receivedDataNo, receivedData_last;
    int receivedWifi2450Pkts, lostWifi2450Pkts;
    cOutVector receivedDataVecRecord;
    cOutVector receivedBeaconVecRecord;
    cOutVector receivedSCHAnnounceBeaconsVecRecord, sentSCHAnnounceBeaconsVecRecord;
    cOutVector receivedWifi2450PktsVecRecord, lostWifi2450PktsVecRecord;
    cOutVector myWifi2450ChannelVecRecord;
    cOutVector myCurrentSCHVecRecord;
    static const simsignalwrap_t pktLostSignal;
    static const simsignalwrap_t SCHChannelBusyTime50msSignal;
    void setCurrentSCH(int myCurrentSCH);
    virtual void sendWSM(WaveShortMessage *wsm);
    void sendIpPacket(cPacket *pkt);
    void sendDelayedDownToChannel(cMessage*, simtime_t_cref, SiChenWaveApplLayerChannelKinds);
    virtual void onBeacon(WaveShortMessage *wsm);
    virtual void onData(WaveShortMessage *wsm);
    virtual void onWifi2450Data(cMessage *msg);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d);
    virtual void handleMessage(cMessage*);
    virtual void handleSelfMsg(cMessage *msg);
    virtual void handleLowerControl(cMessage *msg);
    virtual void handleLowerMsg(cMessage *msg);
    void selectNextSchNumberGivenThisMeasure(double d);
    void announceSchNo(int);

protected:
    ChannSelector channSelector;
//    SqliteAccess *db;

    struct CrossLayerInfo {
        double SCHBusyTime[10];
    }CrossLayerInfo;

};

#endif /* SiChenWaveApplLayer_H_ */
