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

// TODO: Not useful
LogShell CommunicationLog::readFrom(int seq) {
    cout << seq << endl;
    return this->log.back();
}

LogShell CommunicationLog::getEntryAt(int entryNum) {
    return this->log.at(entryNum);
}

int8_t CommunicationLog::getCurrentSeqNum() {
    return this->getLastEntry().sequenceNum;
}

int CommunicationLog::getLogsSize() {
    return this->log.size();
}

vector<LogShell> CommunicationLog::getLog() {
    return this->log;
}

void CommunicationLog::printLastEntry() {
    Printer p;
    cout << "ok" << endl;
    LogShell l = this->getLastEntry();
    p.visit(&l);
    cout << p.str() << endl;
    p.clearOss();
}

string CommunicationLog::getLogAsString() {
    Printer printer;
    ostringstream oss;
    for (auto log: this->log) {
        printer.visit(&log);
        oss << printer.str() << endl;
        printer.clearOss();
    }
    return oss.str();
}
