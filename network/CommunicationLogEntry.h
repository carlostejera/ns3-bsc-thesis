#ifndef COMMUNICATION_LOG_ENTRY
#define COMMUNICATION_LOG_ENTRY

#include "Config.h"
#include "functional"
#include <sstream>

using namespace ns3;
using namespace std;

/**
 * @brief Communication log entry contains several data types that are important to keep order and to know what
 * kind of function have been executed on other devices
 * 
 */

class CommunicationLogEntry {
    private:
    uint8_t sequenceNum = 0; // Sequence number of entry

    // TODO: Mac and flag should be in the event (rename to content)
    string event;   // Event as string to know what actually happens
    string prevEventHash;
    
    uint8_t functionFlag;   // Function flag to know what kind of function has to be executed (only add or remove a MAC  of the list)
    
    ns3::Mac48Address concernedMacAddress; 

    // TODO: Change nodeId to sourceNodeId or authorId set it together with the seq num
    int8_t nodeId;
    string timestamp;   

    public:
    CommunicationLogEntry(uint8_t seqNum, string prevHash, uint8_t flag, string event, Mac48Address mac, int8_t nodeId) {
        this->sequenceNum = seqNum;
        this->prevEventHash = prevHash;
        this->functionFlag = flag;
        this->event = event;
        this->concernedMacAddress = mac;
        this->nodeId = nodeId;

        this->prevEventHash = "-";
    }
    virtual ~CommunicationLogEntry() {}

    string createEvent(){
        ostringstream s1;
        s1 << this->concernedMacAddress;
        return "id:" + to_string(this->nodeId) + 
        " seq:" + to_string(this->sequenceNum) +
        " prevHash:" + prevEventHash +
        " content(function:" + to_string(this->functionFlag) + 
        " mac:" + s1.str() +
        " msg:" + this->event
        + ")";
    }

    void print() {
        cout << this->createEvent() << endl;
        //cout << "Event(SeqNum:" << to_string(this->sequenceNum) << ", prevHash:" << this->prevEventHash << ", flag:" << to_string(this->functionFlag) << ", Message: " << this->event << this->concernedMacAddress <<  ")" << endl;
    }
};
// EthernetFrameNs3(dest_mac_address, logEntry(id, seq, prevHash, content(function, params, comment), signature))
#endif