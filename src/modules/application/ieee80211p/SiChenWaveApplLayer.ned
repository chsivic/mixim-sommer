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

package org.mixim.modules.application.ieee80211p;
import org.mixim.base.modules.IBaseApplLayer;

simple SiChenWaveApplLayer extends BaseWaveApplLayer
{
    parameters:
        @class(SiChenWaveApplLayer);
        beaconInterval = 0.1s;
        dataLengthBits = 1024bit; // this is set to stress the system, creating packet loss, just for debugging purpose
        						  // AND packet drop.  8192 is too long for the designed backoff parameters
        bool sendRelayMsg = default(true); // starting from front car, relay message to following car
        double relayMsgInterval @unit(s) = default(0.01s);
        int relayMsgLengthBits @unit(bit) = default(1024bit);
        bool dataOnIsm = default(true); 
        
        int lowestSCH = default(172);
        int highestSCH = default(182);
        string channelSelectionMode = default("none");// none, random, learning
        double explorationRate_epsilon = default(0.1);
        double learningRate_alpha = default(0.3);
        string dbFileName = default("db/channelValueDB_trip0");
        
        bool enableRealSniffingData = default(false);
        
    gates:
        
        // additional gates other than for DSRC nic
        input lowerLayer2450In; // from mac layer
        output lowerLayer2450Out; // to mac layer
        input lowerControl2450In;
        output lowerControl2450Out;
}
