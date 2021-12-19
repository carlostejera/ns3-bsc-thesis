#include "CommunicationLog.h"
#include "Config.h"
using namespace ns3;
using namespace std;

bool CommunicationLog::addToLog(LogShell shell) {

    if (this->log.empty()) {
        this->log.push_back(shell);
        return true;
    }
    if (shell.sequenceNum == this->getCurrentSeqNum() + 1 && shell.prevEventHash == this->createHash(this->getLastEntry())) {
        this->log.push_back(shell);
        return true;
    }
    cout << "i dont need that" << endl;
    return false;
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
    return this->log.empty() ? -1 : this->getLastEntry().sequenceNum;
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

string CommunicationLog::createHash(LogShell entry) {
    Printer p;
    p.visit(entry.shell);
    string content = p.str();
    const char* contentAsChar = content.c_str();
    p.clearOss();

    int sum = 0;
    int i = 0;
    while (contentAsChar[i] != '\0') {
        sum += contentAsChar[i];
        i++;
    }
    return to_string(sum);}
