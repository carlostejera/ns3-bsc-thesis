#include "CommunicationLog.h"
#include "Config.h"
using namespace ns3;
using namespace std;

void CommunicationLog::addToLog(LogShell shell) {
  this->log.push_back(shell);
}

void CommunicationLog::initialiseLog() {
    this->addToLog(LogShell(0, "", this->owner, new ContentShell("", "", "Log initialised")));
}

void CommunicationLog::readLog() {
    cout << "--------------------------------" << endl;
    cout << "--------------------------------" << endl;

}

LogShell CommunicationLog::getLastEntry() {
    return this->log.back();
}