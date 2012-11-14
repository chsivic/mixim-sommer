
#ifndef DECIDER80211x_H_
#define DECIDER80211x_H_

#include "MiXiMDefs.h"
#include "Decider80211MultiChannel.h"
#include "MappingBase.h"
#include <string>
#include <cmath>
#include <map>

/**
 * downstream with small tweaks
 */
class MIXIM_API Decider80211x: public Decider80211MultiChannel
{
    public:

    protected:



    public:


        /** Constructor using channel number, setting current channel*/
        Decider80211x(DeciderToPhyInterface* phy,
                      double threshold,
                      double sensitivity,
                      int channel,
                      int myIndex = -1,
                      bool debug = false);




        /**
         * @brief Quick and ugly printing of a two dimensional mapping.
         */
        void printMapping(ConstMapping* m){
            m->print(ev.getOStream(), 1000., 1e-6, "MHz\\ms", &Dimension::frequency);
        }

        void channelChanged(int newChannel);

};

#endif /* DECIDER80211x_H_ */
