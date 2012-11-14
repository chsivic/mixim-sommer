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
Decider80211x::Decider80211x(DeciderToPhyInterface* phy, double threshold,
        double sensitivity, int channel, int myIndex, bool debug) :
        Decider80211MultiChannel(phy, threshold, sensitivity, 0, channel,
                myIndex, debug) {
}

void Decider80211x::channelChanged(int newChannel) {
    assert(1 <= currentChannel);
    currentChannel = newChannel;
    centerFrequency = CENTER_FREQUENCIES[currentChannel];
    channelStateChanged();
}

/**
 * @brief Calculates a RSSI-Mapping (or Noise-Strength-Mapping) for a
 * Signal.
 *
 * This method can be used to calculate a RSSI-Mapping in case the parameter
 * exclude is omitted OR to calculate a Noise-Strength-Mapping in case the
 * AirFrame of the received Signal is passed as parameter exclude.
 */
Mapping* Decider80211x::calculateRSSIMapping(simtime_t_cref start,
        simtime_t_cref end, AirFrame* exclude) {
    AirFrameVector airFrames;

    // collect all AirFrames that intersect with [start, end]
    getChannelInfo(start, end, airFrames);

    //    // Si Chen added for debugging
    bool print_frames = false;
    if (debug && airFrames.size() > 2) {
        print_frames = true;
        deciderEV << airFrames.size() << " air frames: " << endl;
    }

// create an empty mapping
    Mapping* resultMap = MappingUtils::createMapping(Argument::MappedZero,
            DimensionSet::timeDomain);

    if (exclude) {
        deciderEV
                << "Creating RSSI map excluding AirFrame with id "
                        << exclude->getId() << endl;

//        // Si Chen added for debugging
        if (print_frames) {
            ev << "print resultMap before adding airframes" << endl;
            resultMap->print(ev.getOStream());
        }

// iterate over all AirFrames and sum up their receiving-power-mappings
        for (AirFrameVector::const_iterator it = airFrames.begin();
                it != airFrames.end(); ++it) {
            // the vector should not contain pointers to 0
            assert(*it != 0);

//            // Si Chen added for debugging
            if (print_frames) {
                ev << "AirFrame " << (*it)->getId() << endl;
                (*it)->getSignal().getReceivingPower()->print(ev.getOStream());
            }

// get the Signal and its receiving-power-mapping
            Signal& signal = (*it)->getSignal();
            const ConstMapping * const recvPowerMap =
                    signal.getReceivingPower();
            assert(recvPowerMap);

            if (*it == exclude) {

                continue;
                // move adding thermal to the end.

            } else {

                // add the signal's receiving power mapping to the resultMap
                deciderEV
                        << "Adding mapping of Airframe with ID "
                                << (*it)->getId() << ". Starts at "
                                << signal.getReceptionStart() << " and ends at "
                                << signal.getReceptionEnd() << endl;
                Mapping* resultMapNew = MappingUtils::add(*recvPowerMap,
                        *resultMap, Argument::MappedZero);
                delete resultMap;
                resultMap = resultMapNew;

//                // Si Chen added for debugging
                if (print_frames) {
                    ev << "print resultMap after adding non-exclude" << endl;
                    resultMap->print(ev.getOStream());
                }
            }

        }

        const ConstMapping * const recvPowerMap =
                exclude->getSignal().getReceivingPower();
        ConstMapping* thermalNoise = phy->getThermalNoise(start, end);

        if (thermalNoise) {
            Mapping* rcvPowerPlusThermalNoise = MappingUtils::add(*recvPowerMap,
                    *thermalNoise);
            Mapping* thermalNoiseInRecvBlock = MappingUtils::subtract(
                    *rcvPowerPlusThermalNoise, *recvPowerMap);

            // add the signal's receiving power mapping to the resultMap
            deciderEV
                    << "Adding mapping of thermalNoise of Airframe with ID "
                            << exclude->getId() << ". Starts at "
                            << exclude->getSignal().getReceptionStart()
                            << " and ends at "
                            << exclude->getSignal().getReceptionEnd() << endl;

            Mapping* resultMapNew = MappingUtils::add(*thermalNoiseInRecvBlock,
                    *resultMap, Argument::MappedZero);
            delete resultMap;
            resultMap = resultMapNew;

//            // Si Chen added for debugging
            if (print_frames) {
                ev << "print thermalNoiseInRecvBlock " << endl;
                thermalNoiseInRecvBlock->print(ev.getOStream());
            }

            delete rcvPowerPlusThermalNoise;
            delete thermalNoiseInRecvBlock;
            thermalNoiseInRecvBlock = 0;

        }

    } else {
        deciderEV << "Creating RSSI map of all AirFrames." << endl;

//      // Si Chen added for debugging
        if (print_frames) {
            ev << "print resultMap before adding airframes" << endl;
            resultMap->print(ev.getOStream());
        }

// iterate over all AirFrames and sum up their receiving-power-mappings
        for (AirFrameVector::const_iterator it = airFrames.begin();
                it != airFrames.end(); ++it) {
            // the vector should not contain pointers to 0
            assert(*it != 0);

//          // Si Chen added for debugging
            if (print_frames) {
                ev << "AirFrame " << (*it)->getId() << endl;
                (*it)->getSignal().getReceivingPower()->print(ev.getOStream());
            }

// otherwise get the Signal and its receiving-power-mapping
            Signal& signal = (*it)->getSignal();
            const ConstMapping * const recvPowerMap =
                    signal.getReceivingPower();
            assert(recvPowerMap);

            // add the signal's receiving power mapping to the resultMap
            deciderEV
                    << "Adding mapping of Airframe with ID " << (*it)->getId()
                            << ". Starts at " << signal.getReceptionStart()
                            << " and ends at " << signal.getReceptionEnd()
                            << endl;

            Mapping* resultMapNew = MappingUtils::add(*recvPowerMap, *resultMap,
                    Argument::MappedZero);
            delete resultMap;
            resultMap = resultMapNew;

//          // Si Chen added for debugging
            if (print_frames) {
                ev << "print resultMap after adding non-exclude" << endl;
                resultMap->print(ev.getOStream());
            }

        }

        //add thermal noise
        ConstMapping* thermalNoise = phy->getThermalNoise(start, end);
        if (thermalNoise) {
            Mapping* resultMapNew = MappingUtils::add(*thermalNoise, *resultMap,
                    Argument::MappedZero);
            delete resultMap;
            resultMap = resultMapNew;
        }

//        // Si Chen added for debugging
        if (debug && airFrames.size() > 2) {
            ev << "print thermalNoise" << endl;
            thermalNoise->print(ev.getOStream());
            ev << "print resultMap after adding thermal" << endl;
            resultMap->print(ev.getOStream());
        }

    }

//        // Si Chen added for debugging
//    if (airFrames.size()>2) {
//        ev << "print resultMap after adding all Air Frames" << endl;
//        resultMap->print(ev.getOStream());
//    }

    return resultMap;
}
