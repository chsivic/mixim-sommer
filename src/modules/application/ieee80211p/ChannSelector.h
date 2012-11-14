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

#ifndef CHANNSELECTOR_H_
#define CHANNSELECTOR_H_

#include <iostream>
#include <fstream>
#include <algorithm>
#include <distrib.h>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "Coord.h"

using namespace std;

// The record structure containing input for reinforcement learning
struct channelRecordEntryStruct{
    int channelNo;
    int channelVisits;
    double channelValue;

    channelRecordEntryStruct(int channelNo = 0, int channelVisits = 0,
            double channelValue = 1) :
            channelNo(channelNo), channelVisits(channelVisits), channelValue(
                    channelValue) { };

    std::string getRecordAsString(void){
        std::stringstream out;
        out << "channelNo:" << channelNo << ", channelVisits:"
                << channelVisits << ",   \tchannelValue:" << channelValue
                << endl;
        return out.str();
    };

    friend bool operator < (channelRecordEntryStruct &i1, channelRecordEntryStruct &i2){
        return i1.channelValue < i2.channelValue;
    };

    bool operator < (channelRecordEntryStruct &i2){
        return channelValue < i2.channelValue;
    };
};
typedef std::map<int, channelRecordEntryStruct> channelRecordMap;
bool channelRecordCompare(channelRecordMap::value_type &i1, channelRecordMap::value_type &i2);



class ChannSelector {
protected:

    bool debug;

    std::string channelSelectionMode;// whether use RL-based channel selection

    channelRecordMap channelRecords;
    std::vector<int> channelVector;
    int totalChannelVisits;
    double explorationRate_epsilon;
    double learningRate_alpha;
    int bestChannelNo;
    double bestChannelValue;
//    int nextChannelToUse;
    bool exploration; //explore or exploit at this step

    int lastExploitationSCH;

    double channelValueDB[40][10];
    std::string fileName;
    int currentTripNo;

public:
    ChannSelector();
    virtual ~ChannSelector();

    enum MEASURE_TYPE {
        TOTAL_PKTS,
        CHANNEL_BUSY_RATIO,
        TOTAL_BYTES
    };

    void initialize(int lowestSCH, int highestSCH, string channelSelectionMode,
            float epsilon, float alpha, string dbFileName, bool debug);

    void updateChannelValueDB(Coord old, Coord cur);

    /* called at the beginning of every CCH interval*/
    int getNextSCH(int currentSCH, double measure, MEASURE_TYPE type);




    void printChannelRecords(void);

    void finish(void);

protected:
    int getNextSCHViaLearning(int currentSCH, double instantReward);

    double calculateInstantReward(double measure, MEASURE_TYPE type);
};

#endif /* CHANNSELECTOR_H_ */
