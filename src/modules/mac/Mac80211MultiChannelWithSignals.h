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

#ifndef MAC80211MULTICHANNELWITHSIGNALS_H_
#define MAC80211MULTICHANNELWITHSIGNALS_H_

#include "Mac80211MultiChannel.h"

class Mac80211MultiChannelWithSignals: public Mac80211MultiChannel {
protected:
    /* this signal indicates pkt loss due to either collision or weak signal*/
    simsignal_t pktLostSignalId;

    static const simsignalwrap_t busyTimeSignalId;
    double channelBusyTime, nextFrameDuration;


protected:
    virtual void initialize(int stage);

    void receiveSignal(cComponent* source, simsignal_t signalID, double d);

    /** @brief Handle messages from lower layer
     * Override from Mac80211 by adding:
     * - signaling pkt collision*/
    virtual void handleLowerControl(cMessage*);

    void handleLowerMsg(cMessage*);


public:
    double getChannelBusyTime(void){double ret=channelBusyTime; channelBusyTime=0; return ret;};
};

#endif /* MAC80211MULTICHANNELWITHSIGNALS_H_ */
