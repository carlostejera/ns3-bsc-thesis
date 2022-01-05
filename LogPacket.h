#pragma once
#include "CommunicationLog.h"

enum CommunicationType {
    SWITCH_SWITCH_COMM,
    SUBSCRIPTION,
    P2P_COMM,
    NO_TYPE
};

struct LogPacket {
  private:
    CommunicationLog* log;
    CommunicationType logType;
    std::string writerReader;
  public:
    LogPacket(std::string writerReader, CommunicationLog* log, CommunicationType type)
    : log(log), logType(type), writerReader(writerReader)
    {}

    CommunicationLog *getLog() const {
        return log;
    }
    CommunicationType getLogType() const {
        return logType;
    }
    const std::string &getWriterReader() const {
        return writerReader;
    }

    const bool isTypeOf(CommunicationType type ) const {
        return this->logType == type;
    }

    const std::string enumsToString(CommunicationType type) {
        if (type == CommunicationType::SWITCH_SWITCH_COMM) return "CommunicationType::SWITCH_SWITCH_COMM";
        if (type == CommunicationType::SUBSCRIPTION) return "CommunicationType::SUBSCRIPTION";
        if (type == CommunicationType::P2P_COMM) return "CommunicationType::P2P_COMM";
        if (type == CommunicationType::NO_TYPE) return "CommunicationType::NO_TYPE";
        return "";
    }

    const std::string toString() {
        ostringstream oss;
        oss << "////////////////////////////////////////" << endl;
        oss << "\033[1;" << "36" << "m";
        oss << "LOG PACKET (" << enumsToString(this->logType) << ", LOG: " << this->writerReader << ")";
        oss << "\033[0m\n";

        oss << "\033[1;" << "34" << "m";
        oss << log->getLogAsString();
        oss << "\033[0m";
        oss << "////////////////////////////////////////" << endl;
        return oss.str();
    }

};

struct LogList {
    vector<LogPacket> list;

    LogList(std::vector<LogPacket> list) : list(list) {}
    LogList() {}

    void add(LogPacket packet) { list.push_back(packet); }

    void remove(std::string writerReader) {
        for (auto it = this->list.begin(); it != this->list.end();  ++it) {
            if (it->getWriterReader() == writerReader) {
                this->list.erase(it);
                return;
            }
        }
    }
    std::string toString() {
        ostringstream oss;
        for (auto log : this->list) {
            oss <<  log.toString() << endl;
        }
        return oss.str();

    }

    CommunicationLog* getLogByWriterReader(std::string writerReader) {
        for (auto logPacket : this->list) {
            if (logPacket.getWriterReader() == writerReader) {
                return logPacket.getLog();
            }
        }
        return nullptr;
    }

    bool exists(std::string writerReader) {
        for (auto logPacket : this->list) {
            if (logPacket.getWriterReader() == writerReader) {
                return true;
            }
        }
        return false;
    }

    const int size() const {
        return this->list.size();
    }

    LogPacket getLogPacketAt(int index) {
        return this->list[index];
    }

    LogList getLogPacketsWithType(CommunicationType type) {
        std::vector<LogPacket> tmpList;
        for (auto logPacket : this->list) {
            if (logPacket.getLogType() == type) {
                tmpList.push_back(logPacket);
            }
        }
        return LogList(tmpList);
    }
};
