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

#ifndef SICHENWAVENETWLAYER_H_
#define SICHENWAVENETWLAYER_H_

#include "BaseNetwLayer.h"

/** handle CCH and SCH pkts between WaveAppl and Mac1609_4.
 * CCH pkts pass through untouched, SCH pkts are processed as in baseNetwLayer.
 *
 * WSMs are passed by. WSMs can be either CCH or SCH.
 * Others are processed with L3 addrss*/
class SiChenWaveNetwLayer: public BaseNetwLayer {

public:
    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

};
#endif /* SICHENWAVENETWLAYER_H_ */
