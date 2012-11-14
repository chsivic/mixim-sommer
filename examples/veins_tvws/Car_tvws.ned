//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package org.mixim.examples.veins_tvws;

import ned.IdealChannel;
import org.mixim.base.modules.*;
import org.mixim.modules.nic.*;
import org.mixim.modules.application.*.*;
import org.mixim.modules.mac.*;
import org.mixim.modules.phy.*;
import org.mixim.modules.netw.SiChenWaveNetwLayer;

module Car_tvws
{
    parameters:
        //        string applType; //type of the application layer
        string mobilityType; //type of the mobility module
        string networkType = default("BaseNetwLayer");       //type of the network layer
    gates:
        input radioDsrcIn; // gate for sendDirect
        input radio2450In;
    //        input radioTvwsIn;
    submodules:
        appl: SiChenWaveApplLayer {
            parameters:
                @display("p=104,24;is=l");
        }

        arp: BaseArp {
            parameters:
                @display("p=284,105");
        }

        netwlDsrc: SiChenWaveNetwLayer {

            @display("p=58,119;i=block/layer,lightGreen");
        }
        nicDsrc: Nic80211p {
            parameters:
                @display("p=58,202;i=block/ifcard,dodgerBlue");
                phy.nbRadioChannels = default(184); // from 172 to 184
                phy.initialRadioChannel = default(178); // CCH is 178
        }

        nic2450: Nic80211MultiChannel {
            parameters:
                @display("p=134,202");
                macType = "org.mixim.modules.mac.Mac80211MultiChannelWithSignals";
                phyType = "org.mixim.modules.phy.PhyLayerWithSignals";
        }
        netwl2450: <networkType> like IBaseNetwLayer {
            parameters:
                @display("p=134,119;i=block/layer");
                headerLength = 32bit;
                stats = false;
        }

        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=284,39;i=block/cogwheel");
        }
    connections:
        radioDsrcIn --> nicDsrc.radioIn;
        radio2450In --> nic2450.radioIn;

        netwl2450.lowerControlOut --> nic2450.upperControlIn;
        netwl2450.lowerLayerOut --> nic2450.upperLayerIn;
        nic2450.upperControlOut --> netwl2450.lowerControlIn;
        nic2450.upperLayerOut --> netwl2450.lowerLayerIn;
        netwl2450.upperControlOut --> appl.lowerControl2450In;
        netwl2450.upperLayerOut --> appl.lowerLayer2450In;
        appl.lowerControl2450Out --> netwl2450.upperControlIn;
        appl.lowerLayer2450Out --> netwl2450.upperLayerIn;
        appl.lowerLayerOut --> netwlDsrc.upperLayerIn;
        appl.lowerControlOut --> netwlDsrc.upperControlIn;
        netwlDsrc.upperControlOut --> appl.lowerControlIn;
        netwlDsrc.upperLayerOut --> appl.lowerLayerIn;
        netwlDsrc.lowerControlOut --> nicDsrc.upperControlIn;
        netwlDsrc.lowerLayerOut --> nicDsrc.upperLayerIn;
        nicDsrc.upperControlOut --> netwlDsrc.lowerControlIn;
        nicDsrc.upperLayerOut --> netwlDsrc.lowerLayerIn;
}
