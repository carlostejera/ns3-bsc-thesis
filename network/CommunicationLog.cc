#include "CommunicationLog.h"
#include "Config.h"
using namespace ns3;
using namespace std;

CommunicationLog::CommunicationLog() {
}

void CommunicationLog::addToLog(uint8_t seqNum, string prevHash, uint8_t flag, string event, Mac48Address mac, int8_t nodeId) {
    this->log.push_back(CommunicationLogEntry(seqNum, prevHash, flag, event, mac, nodeId));
}

void CommunicationLog::initialiseLog(ns3::Mac48Address authorMac, int8_t id) {
    this->addToLog(0, "", 0, to_string(this->owner) + "'s initialised ", authorMac, id);
}

void CommunicationLog::readLog() {
    cout << "--------------------------------" << endl;
    for (auto i = this->log.begin(); i != this->log.end(); i++) {
        i->print();
    }
    cout << "--------------------------------" << endl;

}

CommunicationLogEntry CommunicationLog::getLastEntry() {
    return this->log.back();
}