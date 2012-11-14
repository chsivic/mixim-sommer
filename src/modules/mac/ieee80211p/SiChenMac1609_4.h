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

#ifndef SICHENMAC16094_H_
#define SICHENMAC16094_H_

#include "Mac1609_4.h"
#include "Consts80211.h"
#include <string>
#include <sstream>
#include "SiChenWaveApplLayer.h" //TODO replace this with a mac1609ToAppInterface class
#include <vector>


#ifndef Mac1609EV
#define Mac1609EV (ev.isDisabled()||!debug) ? ev : ev << "[Car " << myCarIndex << "]" << getParentModule()->getFullPath() << " - SiChenMac1609.4: "
#endif




class SiChenMac1609_4: public Mac1609_4 {
public:
    virtual ~SiChenMac1609_4(){};

    void updataChannelValueDB(Coord old, Coord cur);

protected:
    int myCarIndex;



    /** @brief Initialization of the module and some variables.*/
    virtual void initialize(int);

    double busyTimeSumOnCCH, busyTimeSumOnCCH_last_second, busyTimeSumOnCCH_last_100ms;
    double busyTimeSumOnSCH, busyTimeSumOnSCH_last_second, busyTimeSumOnSCH_last_100ms;

    double SCHChannelBusyTime50ms, CCHChannelBusyTime50ms;






    /* control message up to the app layer
     * now used to carry msg requesting switch group SCH broadcast */
    cMessage* controlMsgUp;

    static const simsignalwrap_t busyTimeSignalId;
    static const simsignalwrap_t CCHChannelBusyTime50msSignalId;
    static const simsignalwrap_t SCHChannelBusyTime50msSignalId;

    cOutVector mySCHVec;
    cOutVector SCHChannelFreeTime50msVec;

    virtual void receiveSignal(cComponent* source, simsignal_t signalID, double d);

    /** added channel sensing statistics*/
    virtual void handleSelfMsg(cMessage*);


    /* WaveAppToMac1609_4Interface
     * diff from Mac1609_4: relaxed channel constraint */
    virtual void changeServiceChannel(int channelNumber);

//    /* for APP to get which channel to switch to, when control msg arrives at appl*/
//    virtual int getNextSCH(void){ return this->myNextSCH;};

    virtual int getMySCH(void){ return this->mySCH;};

    virtual void finish(void);
};

#endif /* SICHENMAC16094_H_ */
