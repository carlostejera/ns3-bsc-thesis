#ifndef COMMUNICATION_LOG_ENTRY_H
#define COMMUNICATION_LOG_ENTRY_H

#include "Config.h"
#include "shells/LogShell.h"

using namespace ns3;
using namespace std;

/**
 * @brief Communication log entry contains several data types that are important to keep order and to know what
 * kind of function have been executed on other devices
 * 
 */

class CommunicationLog {
    private:
    uint8_t owner;
    vector<LogShell> log;
    uint8_t currentSeqNum = 0;
    

    public:
    CommunicationLog();
    //CommunicationLog(uint8_t owner) : owner(owner) {}
    CommunicationLog(uint8_t owner) {
        this->owner = owner;
    }
    virtual ~CommunicationLog() {}

    void initialiseLog(ns3::Mac48Address authorMac, int8_t id);
    void addToLog(uint8_t seqNum, string prevHash, uint8_t flag, string event, Mac48Address mac, int8_t nodeId);
    void readLog();
    LogShell getLastEntry();
    bool isSuccessorEntry();



    


};

#endif