#include "Manager.h"
#include "GlobalsValues.h"

using namespace ns3;
using namespace std;

void Manager::registerUser(Ptr<NetDevice> dev, int8_t authorId) {
    ostringstream oss;
    oss << "New member arrived. Add " << to_string(authorId) << " to member list";
    this->neighbourMap.insert(make_pair(authorId, dev));
    this->familyMembers.push_back(authorId);
    this->networkLog->addToLog(LogShell(this->networkLog->getCurrentSeqNum() + 1, this->networkLog->createHash(this->networkLog->getLastEntry()), this->authorId,
                                        new ContentShell(ADD_MEMBER, to_string(authorId), oss.str())));
    this->currSeq++;
}

void Manager::sendNetworkJoinConfirmation(int8_t authorId) {
    // Sends a confirmation to the join requester and also all necessary entries to reconstruct the current network
    Ptr<NetDevice> nDev = this->neighbourMap[authorId];
    auto destinationMac = ns3::Mac48Address::ConvertFrom(nDev->GetAddress());
    ContentShell *cShell = new ContentShell(ASSIGN_MANAGER, to_string(this->authorId), "Request confirmed. Network is joined.");
    LogShell *logShell =  new LogShell(-1, "prev", this->authorId, cShell);

    NetShell* nShell = new NetShell(destinationMac, authorId, DIARY, 0, logShell);

    Ptr <Packet> p = this->createPacket(nShell);
    this->sendPacket(nDev, p);
}

void Manager::sendAllLogEntriesTo(int8_t authorReceiverId) {
    auto receiverNetDevice = this->neighbourMap[authorReceiverId];
    auto receiverMac = ns3::Mac48Address::ConvertFrom(receiverNetDevice->GetAddress());
    NetShell* nShell;
    Ptr<Packet> p;
    cout << "Sending " << to_string(this->networkLog->getLogsSize()) << " entries to " << to_string(authorReceiverId) << endl;
    for (int i = 0; i < this->networkLog->getLogsSize(); i++) {
        auto log = this->networkLog->getEntryAt(i);
        nShell = new NetShell(receiverMac, authorReceiverId, "manager:" + to_string(this->authorId) + "/switch:*", 0, &log);
        p = this->createPacket(nShell);
        this->sendPacket(receiverNetDevice, p);
    }

}

void Manager::broadcastLastNetworkChange(int8_t exceptedReceiver =-1) {
    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto authorId = iter->first;

        // TODO: Drop one packet on purpose
        if (authorId == 3 && exceptedReceiver == 4)
            continue;

        if (authorId != exceptedReceiver) {
            this->sendLastEntryTo(authorId, "manager:" + to_string(this->authorId) + "/switch:*");
        }
    }
}

bool Manager::concatenateEntry(NetShell* netShell) {
    if (!this->logExists(netShell)) {
        // TODO: Maybe change
        this->communicationLogs.push_back({netShell->type, new CommunicationLog(netShell->shell->authorId, this->authorId)});
    }
    CommunicationLog* log = this->getLogFrom(netShell->type);
    string conc = "& concatenating entry " + to_string(netShell->shell->sequenceNum) + " to " + netShell->type + "\n";
    string drop = "& dropping packet, not matching subsequent entry\n";
    bool result = log->addToLog(*(netShell->shell));
    this->packetOss << (result ? conc : drop);
    return result;
}



void Manager::recvPkt(
        Ptr<NetDevice> dev,
        Ptr<const Packet> packet,
        uint16_t proto,
        const Address& from,
        const Address& to,
        NetDevice::PacketType pt ) {

    this->packetOss << "------------------ManagerPacket-------------------" << endl
        << "Manager: " << to_string(this->authorId) << endl;

    string netShell = this->readPacket(packet);
    NetShell* nShell = SomeFunctions::shell(netShell);

    this->packetOss << "Received: " << netShell << endl;
    this->packetOss << "Result: ";

    // Broadcasts for the manager
    if (nShell->type.find("/manager:*") != string::npos && nShell->receiverId == 127) {
        this->concatenateEntry(nShell);

        auto lastEntry = this->getLogFrom(nShell->type)->getLastEntry();
        switch(this->hash(lastEntry.shell->function)) {
            case ADD_TO_NETWORK:
                this->registerUser(dev, nShell->shell->authorId);
                this->packetOss << "Add neighbour " << to_string(nShell->shell->authorId) << " & register Switch ";
//            this->sendNetworkJoinConfirmation(receiverId);
                this->sendAllLogEntriesTo(nShell->shell->authorId);
                this->packetOss << "& sending log " << "manager" + to_string(this->authorId) + "/switch* ";
                this->broadcastLastNetworkChange(nShell->shell->authorId);
                this->packetOss << "& broadcasting changes" << endl;
                break;
            default:
                break;
        }

        // Gossip requests
    } else if (nShell->type.find("manager:" + to_string(this->authorId) + "/switch:*") != string::npos) {
        if (this->myPersonalLog->getCurrentSeqNum() > nShell->shell->sequenceNum) {
            auto seq = nShell->shell->sequenceNum;
            this->sendEntryFromIndexTo(this->networkLog, this->getKeyByValue(dev), seq, "manager" + to_string(this->authorId) + "/switch*");
        }
        this->packetOss << "& nothing needed from me" << endl;
    } else {
        this->packetOss << "Dropping packet, unknown content" << endl;
    }

    this->packetOss << "---------------ManagerPacket_END----------------\n" << endl;
    if (VERBOSE) {
        // cout << this->packetOss.str() << endl;
    }
}

bool Manager::processReceivedSwitchPacket(NetShell *netShell, Ptr <NetDevice> dev) {
    return true;
}

void Manager::processReceivedUserPacket(NetShell *netShell, Ptr <NetDevice> dev) {

}
CommunicationLog *Manager::getLogFrom(string type) {
    for (auto l : this->communicationLogs) {
        if (l.first == type) {
            return l.second;
        }
    }
    return NULL;
}
