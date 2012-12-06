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

#include "SiChenMac1609_4.h"


const simsignalwrap_t SiChenMac1609_4::busyTimeSignalId = simsignalwrap_t(
        "airFrameBusyTime"); //subscribe, emitted from phylayerwithsignal line 85
const simsignalwrap_t SiChenMac1609_4::CCHChannelBusyTime50msSignalId =
        simsignalwrap_t("busyTimeIntervalSumCCH"); //emit
const simsignalwrap_t SiChenMac1609_4::SCHChannelBusyTime50msSignalId =
        simsignalwrap_t("SCHChannelBusyTime50ms"); //emit

Define_Module(SiChenMac1609_4);

void SiChenMac1609_4::initialize(int stage)
{
    Mac1609_4::initialize(stage);
    if (stage == 0){

        busyTimeSumOnCCH = 0;
        busyTimeSumOnSCH = 0;
        busyTimeSumOnSCH_last_100ms = busyTimeSumOnSCH;
        busyTimeSumOnCCH_last_100ms = busyTimeSumOnCCH;

        /** subscribe to signal emitted from dsrc phy layer*/
        findHost()->subscribe(busyTimeSignalId, this);

        myCarIndex = getParentModule()->getParentModule()->getIndex();

        mySCHVec.setName("my SCH");
    }
}

void SiChenMac1609_4::receiveSignal(cComponent* source, simsignal_t signalID, double d) {
    Enter_Method_Silent();
    if (signalID == busyTimeSignalId) {
        activeChannel == type_CCH ? busyTimeSumOnCCH += d : busyTimeSumOnSCH += d;
        Mac1609EV << "busyTimeSignalId received. busyTimeIntervalSumOnCCH = " << busyTimeSumOnCCH
                << ", busyTimeIntervalSumOnSCH = " << busyTimeSumOnSCH << endl;
    }
}

void SiChenMac1609_4::handleSelfMsg(cMessage* msg){
    Mac1609_4::handleSelfMsg(msg);// do channel switching, activeChannel updated

    if (msg==nextChannelSwitch) {
        if (activeChannel == type_CCH) {//switching to CCH?


            /* calculate the instant reward on this channel during the past interval
             * right now, it is calculated as the free time percentage */
            SCHChannelBusyTime50ms = busyTimeSumOnSCH - busyTimeSumOnSCH_last_100ms;
            CCHChannelBusyTime50ms = busyTimeSumOnCCH - busyTimeSumOnCCH_last_100ms;
            busyTimeSumOnSCH_last_100ms = busyTimeSumOnSCH;
            busyTimeSumOnCCH_last_100ms = busyTimeSumOnCCH;
            emit(SCHChannelBusyTime50msSignalId, SCHChannelBusyTime50ms);
            emit(CCHChannelBusyTime50msSignalId, CCHChannelBusyTime50ms);

        } else { //activeChannel == SCH
            // record vectors
            mySCHVec.record(mySCH);
        }


    }
}



/* Will change the Service Channel on which the mac layer is listening and sending
 * WaveAppToMac1609_4Interface
 * diff from Mac1609_4: relaxed channel constraint */
void SiChenMac1609_4::changeServiceChannel(int channelNumber) {
//    ASSERT(channelNumber == myNewSCH && channelNumber!=mySCH);//debugging check
    mySCH = channelNumber;

    if (activeChannel == type_SCH) {
        //change to new chann immediately if we are in a SCH slot,
        //otherwise it will switch to the new SCH upon next channel switch
        phy->setCurrentRadioChannel(mySCH);
    }

    // also record mySCH here when SCH changed during SCH interval
    mySCHVec.record(mySCH);
}

void SiChenMac1609_4::finish(void){
    Mac1609_4::finish();
}
