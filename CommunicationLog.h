#ifndef COMMUNICATION_LOG_ENTRY_H
#define COMMUNICATION_LOG_ENTRY_H

#include "shells/LogStructures.h"
using namespace ns3;
using namespace std;

/**
 * @brief Communication log entry contains several data types that are important to keep order and to know what
 * kind of function have been executed on other devices
 * 
 */

class CommunicationLog {
    private:
    int8_t owner;
    vector<LogShell> log;

    public:
    CommunicationLog(int8_t owner) {
        this->owner = owner;
    }
    virtual ~CommunicationLog() {}

    void initialiseLog();
    void addToLog(LogShell shell);
    void readLog();
    string readFrom(int seq);
    LogShell getLastEntry();
    LogShell getEntryAt(int entryNum);
    int8_t getCurrentSeqNum();
    int getLogsSize();
    vector<LogShell> getLog();

    


};

#endif