#include "CommunicationLog.h"
#include "Config.h"
#include "shells/ContentShell.h"
using namespace ns3;
using namespace std;

CommunicationLog::CommunicationLog() {
}

void CommunicationLog::addToLog(uint8_t seqNum, string prevHash, uint8_t flag, string event, Mac48Address mac, int8_t nodeId) {
  Shell* shell = new ContentShell ("function" + to_string (flag), "params", "msg" + event);
  this->log.push_back(LogShell (shell, seqNum, prevHash, nodeId));
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

LogShell
CommunicationLog::getLastEntry() {
    return this->log.back();
}