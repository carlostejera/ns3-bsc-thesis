#include "Manager.h"
using namespace ns3;
using namespace std;


/*bool Manager::registerSwitch(Ptr<NetDevice> dev, iShell* shell) {
    cout << dev->GetAddress() << endl;
    cout << "Register a new switch" << endl;
    ostringstream prevEvent;
    prevEvent << "event" << this - seq;
    this->cLog.addToLog(this->seq, prevEvent.str(), this->nodeId, "addToNetwork", to_string(shell->getAuthor()), "Switch was added");
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

void Manager::sendLogEntries(Ptr<NetDevice> dev, uint8_t msg, ns3::Mac48Address sender, int8_t seq) {

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
    oss << "Add the author " << to_string(authorId) << " to the AuthorMacMap.";
    this->authorMacMap.insert(make_pair(authorId, dev));
    this->cLog->addToLog(LogShell(this->currSeq, "prev", this->authorId,
                                  new ContentShell(ASSIGN_MANAGER,to_string(authorId), oss.str())));
    this->currSeq++;
}

void Manager::sendNetworkJoinConfirmation() {
    LogShell lShell = this->cLog->getLastEntry();
    LogShell* p_lShell = &lShell;
    NetShell* nShell;
    Printer stringAssembler;
    string netShellString;

    // Sends a confirmation to the join requester and also all necessary entries to reconstruct the current network
    for (auto iter = this->authorMacMap.begin(); iter != this->authorMacMap.end(); iter++) {
        Ptr<NetDevice> nDev = iter->second;

        auto destinationMac = ns3::Mac48Address::ConvertFrom(nDev->GetAddress());

        nShell = new NetShell(destinationMac, LOG_ENTRY, p_lShell);
        stringAssembler.visit(nShell);
        netShellString = stringAssembler.str();
        stringAssembler.clearOss();
        std::vector <uint8_t> myVector(netShellString.begin(), netShellString.end());
        uint8_t *text = &myVector[0];
        Ptr <Packet> p = Create<Packet>(text, strlen((char *) text));
        cout << p << endl;
        cout << "Sending packets to " << destinationMac << " with Packet size" << p->GetSize() << endl;
        if (!nDev->Send(p, destinationMac, 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }

    }
}

void Manager::recvPkt(
        Ptr<NetDevice> dev,
        Ptr<const Packet> packet,
        uint16_t proto,
        const Address& from,
        const Address& to,
        NetDevice::PacketType pt ) {
    cout << "manager" << endl;
    cout << packet << endl;

    uint32_t pktSize = packet->GetSize();
    uint8_t buf[pktSize];
    memset(buf, 0 , pktSize);
    packet->CopyData(buf, pktSize);
    string netShell (buf, buf + pktSize);
    cout << "----" << netShell << endl;
    NetShell* nShell = SomeFunctions::shell(netShell);
    if (nShell->type == "request") {
        cout << "Request from " << to_string(nShell->shell->authorId) << ": " << nShell->shell->shell->function << endl;
        if (nShell->shell->shell->function == "addToNetwork") {
            // TODO: Change the 48
            this->registerUser(dev, nShell->shell->authorId);
            this->sendNetworkJoinConfirmation();
        }
    }
    delete nShell;
    //------------------------ string is read

//    auto iShell = shell (netShell);
//    cout << (iShell->getFunction() == "addToNetwork") << endl;
//    if (iShell->getFunction() == "addToNetwork") {
//        this->registerSwitch(dev, iShell);
//        if (this->registerSwitch(dev, sender, switchId)) {
//            this->confirmSwitchJoin(dev, sender);
//            this->sendLogEntries(dev, HERE_ARE_THE_LOG_ENTRIES, sender, 0);
//        }
    cout << "lets goooooooooooooo" << endl;
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
//                this->sendLogEntries(dev, HERE_ARE_THE_LOG_ENTRIES, sender, 0);
//            }
//            break;
//            case (USER_JOIN_REQUEST):
//                if (this->registerUser(dev, sender, switchId)) {
//                    this->confirmUserJoin(dev, sender, switchId);
//                }
//        case (LOG_ENTRIES):
//            seq = switchId;
//            cout << "somebody needs entries from seq " << to_string(seq) << endl;
//            this->sendLogEntries(dev, HERE_ARE_THE_LOG_ENTRIES, sender, seq);
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
