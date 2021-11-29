#include "Manager.h"
using namespace ns3;
using namespace std;

void Manager::registerUser(Ptr<NetDevice> dev, int8_t authorId) {
    ostringstream oss;
    oss << "New member arrived. Add " << to_string(authorId) << " to member list";
    this->neighbourMap.insert(make_pair(authorId, dev));
    this->familyMembers.push_back(authorId);

    this->networkLog->addToLog(LogShell(this->networkLog->getCurrentSeqNum() + 1, "", this->authorId,
                                        new ContentShell(ADD_MEMBER, to_string(authorId), oss.str())));
    this->currSeq++;
}

void Manager::sendNetworkJoinConfirmation(int8_t authorId) {
    // Sends a confirmation to the join requester and also all necessary entries to reconstruct the current network
    Ptr<NetDevice> nDev = this->neighbourMap[authorId];
    auto destinationMac = ns3::Mac48Address::ConvertFrom(nDev->GetAddress());
    ContentShell *cShell = new ContentShell(ASSIGN_MANAGER, to_string(this->authorId), "Request confirmed. Network is joined.");
    LogShell *logShell =  new LogShell(-1, "prev", this->authorId, cShell);

    NetShell* nShell = new NetShell(destinationMac, authorId, DIARY, logShell);

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
        nShell = new NetShell(receiverMac, authorReceiverId, LOG_ENTRY, &log);
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
            this->sendLastEntryTo(authorId);
        }
    }
}


void Manager::reconstructLog(ContentShell *cShell) {

}

void Manager::recvPkt(
        Ptr<NetDevice> dev,
        Ptr<const Packet> packet,
        uint16_t proto,
        const Address& from,
        const Address& to,
        NetDevice::PacketType pt ) {

    ostringstream oss;
    oss << "------------------ManagerPacket-------------------" << endl
        << "Manager: " << to_string(this->authorId) << endl
        << packet << endl;

    string netShell = this->readPacket(packet);
    oss << "Received: " << netShell << endl;
    NetShell* nShell = SomeFunctions::shell(netShell);

    if (nShell->type == "request") {
        oss << "Request: "  << nShell->shell->shell->function << " (from "<< to_string(nShell->shell->authorId) << ")" << endl;
        oss << "Result: ";
        if (nShell->shell->shell->function == "addToNetwork") {
            auto receiverId = nShell->shell->authorId;
            this->registerUser(dev, receiverId);
            this->sendNetworkJoinConfirmation(receiverId);
            this->sendAllLogEntriesTo(receiverId);
            this->broadcastLastNetworkChange(receiverId);
            oss << to_string(receiverId) << " added to the network. Sending confirmation and log entries.";
        }
    }
    cout << oss.str() << endl;
    cout << "---------------ManagerPacket_END----------------\n" << endl;

}