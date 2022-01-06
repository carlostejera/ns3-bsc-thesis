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
    if (this->isSubsequentEntry(shell)){
        this->log.push_back(shell);
        return true;
    }
    return false;
}
/**
 * Initialises a log with a first entry to avoid correctness problems
 */
void CommunicationLog::initialiseLog() {
    this->appendLogShell(new ContentShell("", "", "Log initialised"));
//    this->addToLog(LogShell(0, "", this->owner, new ContentShell("", "", "Log initialised")));
}

/**
 * @return last LogShell of the log
 */
LogShell CommunicationLog::getLastEntry() {
    return this->log.back();
}
/**
 * Get log with given sequence number
 * @param entryNum sequence number
 * @return LogShell at given position
 */
LogShell CommunicationLog::getEntryAt(int entryNum) {
    return this->log.at(entryNum);
}

/**
 * Get current sequence number. If the log is yet empty, it returns -1.
 * @return sequence number
 */
int16_t CommunicationLog::getCurrentSeqNum() {
    return this->log.empty() ? -1 : this->getLastEntry().sequenceNum;
}
/**
 * @return Number of LogShells
 */
int CommunicationLog::getLogsSize() {
    return this->log.size();
}

/**
 * @return log
 */
vector<LogShell> CommunicationLog::getLog() {
    return this->log;
}

/**
 * @return Log as a long string for printing reasons
 */
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

/**
 * Create a hash value of the last LogShell of the log ensure that the next LogShell is not altered in any way.
 * @param entry last LogShell
 * @return hash value
 */
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
    return oss.str();
}
/**
 * Checks:  Does the new LogShell have the next sequence number?
 *          Is the new LogShell's hash value the same one, if the last LogShell is converted to a hash value?
 *          Is the author also the owner?
 *
 * If the log is still empty, it checks only if the sequence number is 0. For LogShell 0 is no hash value.
 *
 * @param lShell the new LogShell to append
 * @return result of the checks
 */
bool CommunicationLog::isSubsequentEntry(LogShell lShell) {
    return ((this->log.empty() && lShell.sequenceNum == 0)
    ||
    (this->getCurrentSeqNum() + 1 == lShell.sequenceNum && this->createHash(this->getLastEntry()) == lShell.prevEventHash && this->owner == lShell.authorId)
    );
}
/**
 *
 * @return dedicated
 */
const std::string & CommunicationLog::getDedicated() const {
    return this->dedicated;
}

/**
 *
 * @return owner
 */
const std::string & CommunicationLog::getOwner() const {
    return this->owner;
}

/**
 * CommunicationLog's function handles the correct way to append a new ContentShell to the log as LogShell only if
 * the device is the author himself.
 * @param contentShell new content to append as LogShell
 */
void CommunicationLog::appendLogShell(ContentShell* contentShell) {
    auto hash = this->log.empty() ? "" : this->createHash(this->getLastEntry());
    this->log.push_back(LogShell(to_string(Simulator::Now().GetSeconds()), this->getCurrentSeqNum() + 1, hash, this->owner, "", contentShell));
}
bool CommunicationLog::empty() {
    return this->log.empty();
}
