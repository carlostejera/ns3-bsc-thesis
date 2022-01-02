#include "CommunicationLog.h"
#include "Config.h"
using namespace ns3;
using namespace std;

/**
 * Adds a log entry to the log, only if the log is empty of if the sequence number and previous hash are correct
 * @param shell
 * @return
 */

bool CommunicationLog::addToLog(LogShell shell) {
    if (this->log.empty() && shell.sequenceNum == 0) {
        this->log.push_back(shell);
        return true;
    }
    // Check sequence and previous hash
    if (shell.sequenceNum == this->getCurrentSeqNum() + 1 && shell.prevEventHash == this->createHash(this->getLastEntry())) {
        this->log.push_back(shell);
        return true;
    }
    return false;
}

void CommunicationLog::initialiseLog() {
    this->addToLog(LogShell(0, "", this->owner, new ContentShell("", "", "Log initialised")));
}

LogShell CommunicationLog::getLastEntry() {
    cout << to_string(this->owner) << " " << to_string(this->dedicated) << endl;
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

    SHA_CTX shactx;
    uint8_t digest[SHA_DIGEST_LENGTH];

    SHA1_Init(&shactx);
    SHA1_Update(&shactx, content.c_str(), content.size());
    SHA1_Final(digest, &shactx);
    ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        oss << hex << setw(2) << setfill('0') << (int)digest[i];
    }
    cout << oss.str() << endl;

    return oss.str();
}

bool CommunicationLog::isSubsequentEntry(LogShell lShell) {
    return ((this->log.empty() && lShell.sequenceNum == 0)
    ||
    (this->getCurrentSeqNum() + 1 == lShell.sequenceNum && this->createHash(this->getLastEntry()) == lShell.prevEventHash && this->owner == lShell.authorId)
    );
}
const int8_t& CommunicationLog::getDedicated() const {
    return this->dedicated;
}
const int8_t &CommunicationLog::getOwner() const {
    return this->owner;
}
