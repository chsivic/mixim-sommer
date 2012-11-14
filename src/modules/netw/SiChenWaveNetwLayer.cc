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

#include "SiChenWaveNetwLayer.h"
#include "SiChenWaveApplLayer.h"

#include <cassert>

Define_Module(SiChenWaveNetwLayer);

/** WSMs pass through untouched
 * others are processed with L3 addresses
 **/
void SiChenWaveNetwLayer::handleUpperMsg(cMessage* msg)
{
    if (dynamic_cast<WaveShortMessage*>(msg)) {
        sendDown(msg);
    } else  {
        BaseNetwLayer::handleUpperMsg(msg);
    }
}

/** WSMs pass through untouched
 * others are processed with L3 addresses
 **/
void SiChenWaveNetwLayer::handleLowerMsg(cMessage* msg)
{
    if (true){// for now, don't consider IPv6, but only WSMP
        sendUp(msg);
    } else {
        BaseNetwLayer::handleLowerMsg(msg);
    }
}
