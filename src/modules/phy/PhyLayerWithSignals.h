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

#ifndef PHYLAYER80211X_H_
#define PHYLAYER80211X_H_

#include "PhyLayer.h"
#include "Mac80211pToPhy11pInterface.h"



/* Knows the following Deciders
 * - Decider80211x
 * - Decider80211p
 */
class PhyLayerWithSignals: public PhyLayer,
    public Mac80211pToPhy11pInterface
//    public Decider80211pToPhy80211pInterface
    /** this redundant iterface simply copy a function in the macToPhyInterface to DeciderToPhyInterface */
{
public:
    virtual ~PhyLayerWithSignals();

protected:
    int myIndex;

    // duration of all messages
    double myBusyTime;

    // duration of all known protocol messages
    double myKnownProtocolBusyTime;

    double myStartTime;

    /* this signal indicates the receiving of a new airframe,
     * can also serve as the indication of medium busy for channel sensing*/
    simsignal_t busyTimeSignalId;

    simsignal_t rcvPowerSignalId;

    enum ProtocolIds {
        IEEE_80211 = 12123,
        IEEE_80211p = 12124,
    };

    virtual void initialize(int stage);

    // added signal emission
    virtual void handleAirFrame(AirFrame* frame);

    void reportRcvPower(AirFrame* frame);

    /**
     * @brief Creates and returns an instance of the Decider with the specified
     * name.
     *
     * Is able to initialize the following Deciders:
     *
     * - Decider80211x
     * - Decider80211p
     * and all defined in PhyLayer
     */
    virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

    virtual Decider* initializeDecider80211x(ParameterMap& params) ;

    virtual Decider* initializeDecider80211p(ParameterMap& params) ;

    virtual void changeListeningFrequency(double freq);

    // check if the frame is on overlapping channel with the this current radio channel
    bool checkFreqOverlapping(AirFrame* frame);

    virtual void finish();
};

#endif /* PHYLAYER80211X_H_ */
