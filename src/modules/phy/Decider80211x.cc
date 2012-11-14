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



#include <cassert>

#include "Decider80211MultiChannel.h"
#include <Decider80211x.h>
#include <DeciderResult80211.h>
#include <Mac80211Pkt_m.h>
#include <Signal_.h>
#include "FWMath.h"
#include "Mapping.h"
#include <AirFrame_m.h>
#include <Consts80211.h>
#include <Consts80211p.h>


/** Constructor using channel number, setting current channel*/
Decider80211x::Decider80211x(DeciderToPhyInterface* phy,
                                double threshold,
                                double sensitivity,
                                int channel,
                                int myIndex,
                                bool debug) :
        Decider80211MultiChannel(phy, threshold, sensitivity,
                0, channel, myIndex, debug)
{
}


void Decider80211x::channelChanged(int newChannel) {
    assert(1 <= currentChannel);
    currentChannel = newChannel;
    centerFrequency = CENTER_FREQUENCIES[currentChannel];
    channelStateChanged();
}
