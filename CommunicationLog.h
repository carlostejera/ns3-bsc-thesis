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
    std::string owner;
    vector<LogShell> log;
    std::string dedicated;
  public:

    CommunicationLog(std::string owner) {
        this->owner = owner;
        this->dedicated = owner;
    }

    CommunicationLog(std::string owner, std::string dedicated) {
        this->owner = owner;
        this->dedicated = dedicated;
    }
    virtual ~CommunicationLog() {}

    void initialiseLog();
    bool addToLog( LogShell shell);
    LogShell getLastEntry();
    LogShell getEntryAt(int entryNum);
    int16_t getCurrentSeqNum();
    int getLogsSize();
    vector<LogShell> getLog();
    string getLogAsString();
    string createHash(LogShell shell);
    bool isSubsequentEntry(LogShell lShell);
    const std::string & getDedicated() const;
    const std::string & getOwner() const;
    void appendLogShell(ContentShell* contentShell);
    bool empty();
};

#endif