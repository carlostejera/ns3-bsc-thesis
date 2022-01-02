#ifndef COMMUNICATION_LOG_ENTRY_H
#define COMMUNICATION_LOG_ENTRY_H

#include "shells/LogStructures.h"
#include <openssl/sha.h>
#include <iomanip>
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
  public:
    const int8_t &getOwner() const;
  private:
    vector<LogShell> log;
    int8_t dedicated;
  public:

  public:

    CommunicationLog(int8_t owner) {
        this->owner = owner;
        this->dedicated = owner;
    }

    CommunicationLog(int8_t owner, int8_t dedicated) {
        this->owner = owner;
        this->dedicated = dedicated;
    }

    CommunicationLog(int8_t owner, LogShell* logShell) {
        this->owner = owner;
        this->addToLog(*logShell);
    }
    virtual ~CommunicationLog() {}

    void initialiseLog();
    bool addToLog( LogShell shell);
    LogShell getLastEntry();
    LogShell getEntryAt(int entryNum);
    int8_t getCurrentSeqNum();
    int getLogsSize();
    vector<LogShell> getLog();
    string getLogAsString();
    string createHash(LogShell shell);
    bool isSubsequentEntry(LogShell lShell);
    const int8_t& getDedicated() const;

};

#endif