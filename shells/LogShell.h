#ifndef COMMUNICATION_LOG_ENTRY
#define COMMUNICATION_LOG_ENTRY

#include "../Config.h"
#include "functional"
#include <sstream>
#include "ShellDecorator.h"
using namespace ns3;
using namespace std;

/**
 * @brief Communication log entry contains several data types that are important to keep order and to know what
 * kind of function have been executed on other devices
 * 
 */

class LogShell : public ShellDecorator {
    private:
    uint8_t sequenceNum = 0; // Sequence number of entry

    // TODO: Mac and flag should be in the event (rename to content)
    string prevEventHash;

    // TODO: Change nodeId to sourceNodeId or authorId set it together with the seq num
    int8_t nodeId;
    string timestamp;

    public:
      LogShell (Shell* shell, uint8_t seqNum, string prevHash, int8_t nodeId) : ShellDecorator (shell) {
        this->sequenceNum = seqNum;
        this->prevEventHash = prevHash;
        this->nodeId = nodeId;
        this->prevEventHash = "-";
    }
    virtual ~LogShell () {}

    string assembleString() const override {
      return "log_shell=(author=" + to_string(this->nodeId) +
             " seq=" + to_string(this->sequenceNum) +
             " prevHash=" + prevEventHash + " " +
             this->shell_->assembleString()
             + ")";
    }

    void print() {
        cout << this->assembleString() << endl;
        //cout << "Event(SeqNum:" << to_string(this->sequenceNum) << ", prevHash:" << this->prevEventHash << ", flag:" << to_string(this->functionFlag) << ", Message: " << this->event << this->concernedMacAddress <<  ")" << endl;
    }
};

#endif