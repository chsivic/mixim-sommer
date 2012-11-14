/*
 * Consts80211.cc
 *
 *  Created on: Mar 14, 2012
 *      Author: chensi
 */
#include <map>
#include "Consts80211.h"

struct CHANN_FREQ_MAP {
    static std::map<int, double> create_channel2frequency_map() {
        std::map<int, double> m;

        //2.4GHz
        for (int ch = 1; ch <= 11; ch++) { //1,2,3,4,5,6,7,8,9,10,11,12,13
            m[ch] = 2412e6 + 5e6 * (ch-1);
        }
        m[14] = 2484e6;

        //5GHz
        for (int ch = 36; ch <= 64; ch+=4){ //36, 40, 44, 48, 52, 56, 60, 64
            m[ch] = 5.18e9 + 5e6 * (ch-36);
        }
        for (int ch = 100; ch<=140; ch+=4){ //100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140
            m[ch] = 5.500e9 + 5e6 * (ch-100);
        }
        for (int ch = 149; ch <= 165; ch+=4) { //149, 153, 157, 161, 165
            m[ch] = 5.745e9 + 5e6 * (ch-149);
        }

        //DSRC
        for (int ch = 172; ch <= 184; ch++) { //172, 174, 175, 178, 180, 182, 184
            m[ch] = 5.86e9 + 5e6 * (ch-172);
        }
        return m;
    }
//
//    static std::vector<int> create_channel_vector() {
//        std::vector<int> v;
//
//
//
//    }

};

std::map<int,double> CENTER_FREQUENCIES =  CHANN_FREQ_MAP::create_channel2frequency_map();
