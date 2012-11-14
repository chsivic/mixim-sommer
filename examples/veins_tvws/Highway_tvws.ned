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

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

import org.mixim.modules.mobility.traci.TraCIScenarioManagerLaunchd;
import org.mixim.modules.obstacle.ObstacleControl;
import org.mixim.modules.world.annotations.AnnotationManager;
//import org.mixim.modules.world.WorldUtilityStats;

import org.mixim.modules.node.WirelessNode;

network Highway_tvws
{
    parameters:
        double playgroundSizeX @unit(m) = default(2500m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m) = default(2500m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        int numHosts = default(8); // total number of hosts in the network  
        @display("bgb=1625,916;bgi=bernado-map,s");

    submodules:
        host[numHosts]: WirelessNode {
            parameters:
                nicType = "Nic80211";
                applicationType = "TrafficGen";
                @display("p=429,341;b=28,60,oval,#ffffff,#ffffff,0;i=antennatower-centered;is=n");

        }
        obstacles: ObstacleControl {
            @display("p=39,210");
        }
        annotations: AnnotationManager {
            @display("p=39,152");
        }
        connectionManagerDsrc: ConnectionManager {
            parameters:
                @display("p=143,62;i=abstract/multicast");
        }
        connectionManager2450: ConnectionManager {
            parameters:
                @display("p=143,122;i=abstract/multicast");
        }
        connectionManagerTvws: ConnectionManager {
            parameters:
                @display("p=143,183;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=39,91;i=misc/globe");
        }
        manager: TraCIScenarioManagerLaunchd {
            parameters:
                @display("p=39,31");
        }

    connections allowunconnected:
}

