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

#include "Mac80211MultiChannelWithSignals.h"
#include "Decider80211.h"

const simsignalwrap_t Mac80211MultiChannelWithSignals::busyTimeSignalId = simsignalwrap_t(
        "airFrameBusyTime"); //subscribe, emitted from phylayerwithsignal

Define_Module(Mac80211MultiChannelWithSignals);

void Mac80211MultiChannelWithSignals::initialize(int stage){
    //call BasePhy's initialize
    Mac80211MultiChannel::initialize(stage);

    if(stage == 0) {
        pktLostSignalId= registerSignal("packetLoss");

        /** subscribe to signal emitted from dsrc phy layer*/
        findHost()->subscribe(busyTimeSignalId, this);
        channelBusyTime=0;

        this->myMacAddr+=9000;
    }
}

void Mac80211MultiChannelWithSignals::receiveSignal(cComponent* source,
        simsignal_t signalID, double d) {
    Enter_Method_Silent();
    if (signalID == busyTimeSignalId) {
        this->channelBusyTime+=d;
        this->nextFrameDuration=d;
    }
}

/** @brief Handle messages from lower layer
 * Override from Mac80211 by adding:
 * - signaling pkt collision*/
void Mac80211MultiChannelWithSignals::handleLowerControl(cMessage* msg){
    switch (msg->getKind()) {
    case Decider80211::COLLISION:
        /**Check if decider can differentiate signal weak from collision.
         * Normally not implemented since WiFi cards usually can't do that. */
    case Decider80211::BITERROR:
        /** Signal upper layer about frame lost */
        emit(pktLostSignalId, msg);
        break;
    default:
        break;
    }
    Mac80211::handleLowerControl(msg);
}

void Mac80211MultiChannelWithSignals::handleLowerMsg(cMessage* msg){
    Mac80211Pkt *af = static_cast<Mac80211Pkt *>(msg);
    if(af->getSrcAddr() >9000)
    {
        this->channelBusyTime-=this->nextFrameDuration;
    }

    Mac80211MultiChannel::handleLowerMsg(msg);
}
