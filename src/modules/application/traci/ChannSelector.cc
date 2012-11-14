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

#include "ChannSelector.h"
#include "Consts80211.h"
#include <assert.h>

bool channelRecordCompare(channelRecordMap::value_type &i1,
        channelRecordMap::value_type &i2)
{
    return i1.second.channelValue < i2.second.channelValue;
}

ChannSelector::ChannSelector() {
    // TODO Auto-generated constructor stub

}

void ChannSelector::initialize(int lowestSCH, int highestSCH,
        string channelSelectionMode, float epsilon, float alpha, string dbFileName, bool debug) {

    this->debug = debug;
    this->channelSelectionMode = channelSelectionMode;

    /** Initialize channelValue database from file*/
    if (channelSelectionMode == "learning") {
        std::ifstream dbFile;
        fileName = dbFileName;
        fileName.erase(fileName.begin());
        fileName.erase(fileName.find('"'));
        currentTripNo = atoi(
                fileName.substr(this->fileName.find("trip") + 4).c_str());

        std::string inFileName = fileName + ".txt"; //channelValueDB_trip0.txt
        fileName = fileName.substr(0, fileName.find("trip") + 4); //channelValueDB_trip
        dbFile.open(inFileName.c_str());
        if (dbFile) {
            std::cout << "Load channel value from file " << inFileName
                    << " (TripNo:" << currentTripNo << ")" << endl;
            for (int loc = 0; loc < 40; ++loc) {
                for (int ch = 0; ch < 10; ++ch) {
//                    dbFile >> channelValueDB[loc][ch];
                    channelValueDB[loc][ch]=1;
                }
            }
        } else {
            std::cerr << "Cannot open file " << inFileName
                    << " to read channel values" << endl;
            exit(1);
        }
        dbFile.close();
    } else if (channelSelectionMode == "random") {

    } else {
        ASSERT2 (channelSelectionMode=="none", channelSelectionMode.c_str());
    }

    /** Initialize channel value records */
    for (int ch = lowestSCH; ch <= highestSCH; ch++) {
        if ( CENTER_FREQUENCIES.find(ch) != CENTER_FREQUENCIES.end() ){
            channelRecords.insert( std::make_pair(ch,channelRecordEntryStruct(ch)) );
            channelVector.push_back(ch);

        }
    }

    if (debug) {
        std::cout<<"[debug] Available channels are: ";
        for (std::vector<int>::iterator it=channelVector.begin(); it!=channelVector.end(); it++)
            std::cout<<*it<<"-";
        std::cout<<endl;
    }



    totalChannelVisits=0;
    explorationRate_epsilon = epsilon;
    learningRate_alpha=alpha;
    bestChannelNo = *channelVector.begin(); // set to the first available channel
    lastExploitationSCH = bestChannelNo;
    bestChannelValue = channelRecords[bestChannelNo].channelValue;

}

/** Update channelvalueDB (long term memory) with channelRecords (short term memory) */
void ChannSelector::updateChannelValueDB(Coord old, Coord cur){
    for (unsigned int ch = 0; ch < this->channelVector.size(); ++ch) {

        this->channelValueDB[int(old.x / 50)][ch] =
                channelRecords[channelVector[ch]].channelValue; //save channel value data in last rounded position

        channelRecords[channelVector[ch]].channelValue =
                this->channelValueDB[int(cur.x / 50)][ch];//load channel value for current position from database

        //TODO: consider channel visit times.
    }



}

/** Convert channel measurement (such as channel busy time, total packets, total bytes)
 * to normalized value as reinforcement signal. 1 is best, 0 is worst
 * TODO: Implement non-linear measure-value functions here*/
double ChannSelector::calculateInstantReward(double measure, MEASURE_TYPE type){
    double instantReward;
    switch (type){
    case TOTAL_PKTS:
        instantReward = max<double>(0.0, 300 - measure)/300.0;
        break;
    case CHANNEL_BUSY_RATIO:
        instantReward = measure > (double)1 ? (double)0 : (double)1 - measure;
        break;
    case TOTAL_BYTES:
        instantReward = max<double>(0.0, 100e3 - measure)/double(100e3);
        break;
    default:
        ASSERT2(false,"unknown measurement");
        break;
    }
    return instantReward;
}

int ChannSelector::getNextSCH(int currentSCH, double measure, MEASURE_TYPE type){
    double instantReward = calculateInstantReward(measure, type);
    int myNextSCH;

    if (channelSelectionMode == "learning") {
        // execute learning
        myNextSCH = getNextSCHViaLearning(currentSCH, instantReward);
    } else if (channelSelectionMode == "random"){
        //pick a random channel
        if (instantReward < 0.8) {//TODO: change this into a parameter when initialization.
            do
                myNextSCH = channelVector[rand() % channelVector.size()];
            while (myNextSCH == currentSCH);
        } else
        {
            myNextSCH = currentSCH;
        }
    } else {
        ASSERT2 (channelSelectionMode=="none", channelSelectionMode.c_str());
        myNextSCH = currentSCH;
    }

    if (debug) {
        cout << "[debug] channelSelectionMode-" << channelSelectionMode
                << ", measure = " << measure << ", instantReward = "<<instantReward<<", " << currentSCH << " -> "
                << myNextSCH << endl;
        if (channelSelectionMode == "learning")
            this->printChannelRecords();
    }

    return myNextSCH;
}

int ChannSelector::getNextSCHViaLearning(int currentSCH, double instantReward)
{
    // update channel record
    this->totalChannelVisits ++;
    channelRecords[currentSCH].channelVisits ++;

    this->channelRecords[currentSCH].channelValue +=
            learningRate_alpha * (instantReward - channelRecords[currentSCH].channelValue);



    exploration = uniform(0,1) < explorationRate_epsilon;
    if (exploration)
    {
        return channelVector[rand() % channelVector.size()];
    } else {
        //find the element in channelRecords (a map) with max channel value;
        bestChannelNo = max_element(channelRecords.begin(),
                channelRecords.end(), channelRecordCompare)->first;

        // prevent jitter between two equally good channels
        if (channelRecords[bestChannelNo].channelValue
                > channelRecords[lastExploitationSCH].channelValue * 1.02) {
            lastExploitationSCH = bestChannelNo;
            return bestChannelNo;
        } else {
            return lastExploitationSCH;
        }
    }
}

void ChannSelector::printChannelRecords(){

    EV << "Print channel records after " << totalChannelVisits << " visits:"<<endl;
    EV << "CH\tNo. visits \t Normalized quality"<< endl;
    EV << "------------------------------" << endl;
    for (channelRecordMap::iterator iter = channelRecords.begin();
            iter != channelRecords.end(); iter++) {
        cout <<"[debug] "<< (*iter).second.channelNo << "\t" << (*iter).second.channelVisits
                  << "\t" << (*iter).second.channelValue << endl;
    }
    EV << "------------------------------" << endl;
}

void ChannSelector::finish(void){
    if (channelSelectionMode == "learning"){
        std::ofstream dbFile;

        char buffer[10];

        sprintf(buffer, "%d", currentTripNo + 1);
        std::string outFileName = fileName + buffer + ".txt";
        dbFile.open(outFileName.c_str());
        if (!dbFile) { // file couldn't be opened
            std::cerr << "Error: file could not be opened" << endl;
            exit(1);
        } else {
            std::cout << "Saved channel value to file "<<outFileName<<endl;
        }
        for (int loc = 0; loc < 40; ++loc) {
            for (int ch = 0; ch < 10; ++ch)
                dbFile << channelValueDB[loc][ch] << " ";
            dbFile << endl;
        }
        dbFile.close();
    }
}

ChannSelector::~ChannSelector() {
    // TODO Auto-generated destructor stub
}

