//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "PhyLayerWithSignals.h"
#include "Decider80211p.h"
#include "Decider80211x.h"
#include "Consts80211.h"

Define_Module(PhyLayerWithSignals);

void PhyLayerWithSignals::initialize(int stage) {
    //call BasePhy's initialize
    PhyLayer::initialize(stage);

    if(stage == 0) {
        debug = par("debug").boolValue();
        myIndex = findHost()->getIndex();
        myBusyTime = 0;
        myKnownProtocolBusyTime = 0;
        myStartTime = simTime().dbl();
        busyTimeSignalId = registerSignal("airFrameBusyTime");
        rcvPowerSignalId = registerSignal("receivingPower");
    }
}

bool PhyLayerWithSignals::checkFreqOverlapping(AirFrame* frame)
{
//  since channel number are assigned every 5MHz, frequency values are implied by channel numbers

    if (frame->getProtocolId() == 12123)
    {//20 MHz channel
//        assert(frame->getChannel()==173 || frame->getChannel()==177 || frame->getChannel()==181);
        return abs(this->radio->getCurrentChannel() - frame->getChannel())<3;
    }
    else if (frame->getProtocolId() == 12124)//80211p
    {//10 MHz channel
        //assert(frame->getChannel() % 2 ==0);
        return abs(this->radio->getCurrentChannel() - frame->getChannel())<2;
    } else {
        opp_error("Unknown frame protocol ID %d of class %s coming from %s",
                frame->getProtocolId(), frame->getClassName(),
                frame->getSenderModule()->getFullName());
        return false;
    }
}

void PhyLayerWithSignals::handleAirFrame(AirFrame* frame) {
    if (frame->getState()==START_RECEIVE && !checkFreqOverlapping(frame))
    {   // do nothing on the frame if it's on non-overlapping frequency.
        debugEV << "[Host " << myIndex << "] - PhyLayerWithSignals: "
                << "My radio channel is " << this->radio->getCurrentChannel()
                << ", frame (protocolId "<<frame->getProtocolId()<<") on channel " << frame->getChannel()
                << ". handleAirFrame return without doing anything. \n";
        delete frame;
        frame=0;
        return;
    }

    {
        if (debug && frame->getState() == END_RECEIVE) {
            reportRcvPower(frame);
        }

        // get the receiving power of the Signal at start-time and center frequency
        Signal& signal = frame->getSignal();

        //measure communication density
        if (frame->getState() == START_RECEIVE) {
            if (this->isKnownProtocolId(frame->getProtocolId()))
                myKnownProtocolBusyTime += signal.getDuration().dbl();

            myBusyTime += signal.getDuration().dbl();
            emit(busyTimeSignalId, signal.getDuration().dbl());// for cross-layer information of channel busy time count
        } else {
            //------print the mappings----------------------
            debugEV << "[Host " << myIndex
                           << "] - PhyLayerWithSignals::handleAirFrame state:"
                           << frame->getState() << ", isKnownProtocolId:"
                           << isKnownProtocolId(frame->getProtocolId()) << endl;
            if (debug)
                dynamic_cast<Decider80211x*>(decider)->printMapping(
                        frame->getSignal().getReceivingPower());
        }

        PhyLayer::handleAirFrame(frame);
    }
}

/* print out receiving power of a frame */
void PhyLayerWithSignals::reportRcvPower(AirFrame* frame){

    Signal& signal = frame->getSignal();
    simtime_t receivingStart = MappingUtils::post(signal.getReceptionStart());

    Argument start(DimensionSet::timeFreqDomain);
    start.setTime(receivingStart);
    start.setArgValue(Dimension::frequency_static(), CENTER_FREQUENCIES[this->radio->getCurrentChannel()]);

    double recvPower = signal.getReceivingPower()->getValue(start);
    emit(rcvPowerSignalId, 10*log10(recvPower));
    debugEV<<"rcvPower = "<<recvPower<<" ("<<10*log10(recvPower)<<" dB)"<<endl;

}

Decider* PhyLayerWithSignals::getDeciderFromName(std::string name, ParameterMap& params) {

    if(name == "Decider80211x") {
        // for generic 80211 working in arbitrary frequency bands
        protocolId = IEEE_80211;
        return initializeDecider80211x(params);
    }
    else if(name == "Decider80211p"){
        // Decider for 802.11p, implemented by Veins
        protocolId = IEEE_80211p;
        return initializeDecider80211p(params);
    }


    return PhyLayer::getDeciderFromName(name, params);
}

Decider* PhyLayerWithSignals::initializeDecider80211x(ParameterMap& params) {

    ParameterMap::iterator it = params.find("threshold");
    if(it == params.end()){
        opp_error("[Host %d] - PhyLayer: ERROR: No threshold parameter defined for Decider80211x!", myIndex);
        return 0;
    }
    double threshold = params["threshold"];

//    it = params.find("centerFrequency");
//    if(it == params.end()){
//        opp_error("[Host %d] - PhyLayer: ERROR: No centerFrequency parameter defined for Decider80211x!", myIndex);
//        return 0;
//    }
//    double centerFreq = params["centerFrequency"];

    Decider80211x* dec = new Decider80211x(this, threshold, sensitivity,
            radio->getCurrentChannel(),
            findHost()->getIndex(), debug);

    return dec;
}

Decider* PhyLayerWithSignals::initializeDecider80211p(ParameterMap& params) {
//    double centerFreq = params["centerFrequency"];
    Decider80211p* dec = new Decider80211p(this, sensitivity, this->radio->getCurrentChannel(),
            findHost()->getIndex(), debug);
    dec->setPath(getParentModule()->getFullPath());
    return dec;
}

void PhyLayerWithSignals::changeListeningFrequency(double freq) {
    Decider80211p* dec = dynamic_cast<Decider80211p*>(decider);
    assert(dec);
    dec->changeFrequency(freq);
}

void PhyLayerWithSignals::finish(){
    double totalTime = simTime().dbl() - myStartTime;
    this->recordScalar("mybusyTime", myBusyTime);
    this->recordScalar("myKnownProtocolBusyTime", myKnownProtocolBusyTime);
}

PhyLayerWithSignals::~PhyLayerWithSignals(){

}
