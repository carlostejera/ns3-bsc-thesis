#include "Manager.h"
using namespace ns3;
using namespace std;


/*bool Manager::registerSwitch(Ptr<NetDevice> dev, iShell* shell) {
    cout << dev->GetAddress() << endl;
    cout << "Register a new switch" << endl;
    ostringstream prevEvent;
    prevEvent << "event" << this - seq;
    this->networkLog.addToLog(this->seq, prevEvent.str(), this->nodeId, "addToNetwork", to_string(shell->getAuthor()), "Switch was added");
    this->seq++;
    this->newConnectedSwitches.insert(make_pair(shell->getAuthor(), dev->GetAddress()));
    cout << "Switch added to list;" << endl;

    for (auto it = this->connectedSwitches.begin(); it != this->connectedSwitches.end(); it++) {
        auto idk = it->second.first;
        cout << idk << endl;
    }
    return true;
}*/

/*
//-----------------------------------------------------------------------------------------------

void Manager::confirmSwitchJoin(Ptr<NetDevice> dev, ns3::Mac48Address sender) {
    uint8_t buf[512];
    PacketStream stream(buf);

    stream << PACKET_JOIN_CONFIRM_MANAGER << this->nodeId;

    Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), sender));

    if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
        std::cout << "Unable to send packet" << std::endl;
    }
}

//-----------------------------------------------------------------------------------------------

void Manager::confirmUserJoin(Ptr<NetDevice> dev, ns3::Mac48Address userMac, uint8_t userId) {
    uint8_t buf[512];
    PacketStream stream(buf);

    stream << USER_JOIN_REQUEST_APPROVED << userId;

    Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), userMac));
    cout << "Sending confirmation to user " << endl;
    if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
        std::cout << "Unable to send packet" << std::endl;
    }
}

//-----------------------------------------------------------------------------------------------

bool Manager::registerUser(Ptr<NetDevice> dev, ns3::Mac48Address userMac, uint8_t userId) {
    this->connectedUsers.insert(make_pair(userId, make_pair(userMac, dev)));
    cout << "User added to list;" << endl;

    for (auto it = this->connectedUsers.begin(); it != this->connectedUsers.end(); it++) {
        auto idk = it->second.first;
        cout << idk << endl;
    }
    return true;
}

//-----------------------------------------------------------------------------------------------

void Manager::broadcastNetworkChanges(uint8_t flag) {
    cout << "New switch arrived, I (Manager) broadcast the changes into the network" << endl;
    // uint8_t buf[512];
    // PacketStream stream(buf);

    // stream << NETWORK_CHANGES << this->nodeId << this->seq;
    // stream.writeMac(this->tmpSwitchEntry.first);
    cout << to_string(this->seq) << "entry" << endl;
    int tmpCount = 0;
    for (auto it=this->connectedSwitches.begin(); it != this->connectedSwitches.end(); it++) {

        if (tmpCount == 2 && this->testCount == 4) {
            ++tmpCount;
            continue;
        }

        auto addr = it->second.first;
        auto dev = it->second.second;

        this->sendLogEntries(dev, flag, addr, this->seq - 1);
        ++tmpCount;

    }
    ++this->testCount;
}

//-----------------------------------------------------------------------------------------------

void Manager::flood (PacketStream stream, ns3::Address sender, ns3::Address target) {
    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
        Ptr<NetDevice> dev = GetNode()->GetDevice(i);

        Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(sender, target));

        if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }
    }
}

//-----------------------------------------------------------------------------------------------

void Manager::connectToManager() {

}

//-----------------------------------------------------------------------------------------------

void Manager::sendAllLogEntriesTo(Ptr<NetDevice> dev, uint8_t msg, ns3::Mac48Address sender, int8_t seq) {

    uint8_t buf[512];
    PacketStream stream(buf);
    auto endSeq = this->managerLog.back().first - seq;
    stream << msg << this->nodeId << endSeq;


    for (auto it = this->managerLog.begin(); it != this->managerLog.end(); it++) {
        auto managerSeq = it->first;
        auto macAddr = it->second;
        if (seq < managerSeq) {
            stream.writeMac(macAddr);
        }
    }

    Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), sender));
    cout << "Sending entries to switch " << endl;
    if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
        std::cout << "Unable to send packet" << std::endl;
    }
}

bool Manager::switchExists(uint8_t id) {
    return this->connectedSwitches.find(id) != this->connectedSwitches.end();
}

void Manager::removeSwitch(uint8_t id) {
    this->connectedSwitches.erase(id);
    cout << "Switch" << id << " disconnected" << endl;
}
*/

//-----------------------------------------------------------------------------------------------

void Manager::registerUser(Ptr<NetDevice> dev, int8_t authorId) {
    ostringstream oss;
    oss << "New member arrived. Add " << to_string(authorId) << " to member list";
    this->neighbourMap.insert(make_pair(authorId, dev));
    this->familyMembers.push_back(authorId);

    this->networkLog->addToLog(LogShell(this->currSeq, "", this->authorId,
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

//    PacketStream stream(buf);
//
//    stream.readSize();
//
//    auto sender = stream.readMac();
//    stream.readMac();
//
//    uint8_t flag;
//    uint8_t switchId;
//    uint8_t seq;
//
//    stream >> flag; //read packet ID from stream
//    stream >> switchId;
//    cout << "---" << endl;
//    cout << "Manager receives a packet from" << sender << endl;
//    cout << packet << endl;
//    cout << "???" << endl;
//    switch (flag) {
//        case (PACKET_JOIN_MANAGER):
//            if (this->registerSwitch(dev, sender, switchId)) {
//                this->confirmSwitchJoin(dev, sender);
//                this->sendAllLogEntriesTo(dev, HERE_ARE_THE_LOG_ENTRIES, sender, 0);
//            }
//            break;
//            case (USER_JOIN_REQUEST):
//                if (this->registerUser(dev, sender, switchId)) {
//                    this->confirmUserJoin(dev, sender, switchId);
//                }
//        case (LOG_ENTRIES):
//            seq = switchId;
//            cout << "somebody needs entries from seq " << to_string(seq) << endl;
//            this->sendAllLogEntriesTo(dev, HERE_ARE_THE_LOG_ENTRIES, sender, seq);
//            break;
//        case (PACKET_DEJOIN):
//            if(this->switchExists(switchId)) {
//                this->removeSwitch(switchId);
//                this->seq++;
//                this->broadcastNetworkChanges(REMOVE_SWITCH_FROM_LIST);
//            }
//
//            break;
//
//
//        default:
//            break;
//    }
