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

Define_Module(Mac80211MultiChannelWithSignals);

void Mac80211MultiChannelWithSignals::initialize(int stage){
    //call BasePhy's initialize
    Mac80211MultiChannel::initialize(stage);

    if(stage == 0) {
        pktLostSignalId= registerSignal("packetLoss");
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
