#ifndef FETCH_H
#define FETCH_H
#include "functional"
#include "CommunicationLog.h"


using namespace ns3;
using namespace std;


typedef std::function<void(CommunicationLog&, uint8_t, string, uint8_t, string, ns3::Mac48Address, int8_t)> fncptr;

fncptr fetch(string s){
    if (s == "addToLog") {
        return &CommunicationLog::addToLog;
    }
    return 0;
}



#endif