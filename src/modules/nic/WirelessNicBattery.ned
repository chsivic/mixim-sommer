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

package org.mixim.modules.nic;

//
// Basic wireless nic definition
//
module WirelessNicBattery extends WirelessNic
{
    parameters:
        phyType = default("org.mixim.modules.phy.PhyLayerBattery");

        double sleepCurrent @unit(mA) = default(0);
        double rxCurrent @unit(mA) = default(0);
        double decodingCurrentDelta @unit(mA) = default(0);
        double txCurrent @unit(mA) = default(0);
        double setupRxCurrent @unit(mA) = default(0);
        double setupTxCurrent @unit(mA) = default(0);
        double rxTxCurrent @unit(mA) = default(0);
        double txRxCurrent @unit(mA) = default(0);
}
