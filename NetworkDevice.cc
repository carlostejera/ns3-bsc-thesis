#include "NetworkDevice.h"

string NetworkDevice::readPacket(Ptr<const Packet> packet) {
    uint32_t pktSize = packet->GetSize();
    uint8_t buf[pktSize];
    memset(buf, 0, pktSize);
    packet->CopyData(buf, pktSize);
    string netShell(buf, buf + pktSize);
    return netShell;
}

void NetworkDevice::gossip() {
    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto authorId = iter->first;
        // If neighbour is family member, send last entry to compare sequence numbers
        if (this->isFamilyMember(authorId)) {
            this->sendLastEntryTo(authorId, GOSSIP);
        }
    }
}

Ptr <Packet> NetworkDevice::createPacket(NetShell *nShell) {
    Printer stringAssembler;

    stringAssembler.visit(nShell);
    string netShellString = stringAssembler.str();
    stringAssembler.clearOss();
    uint8_t* text = (uint8_t*) netShellString.c_str();
    auto p = Create<Packet>(text, strlen((char *) text));
//    MyHeader sourceHeader;
//    sourceHeader.SetData(2);
//    p->AddHeader(sourceHeader);

    p->Print(cout);
    return p;
}

void NetworkDevice::sendPacket(Ptr<NetDevice> nDev, Ptr<Packet> p) {
    auto destinationMac = ns3::Mac48Address::ConvertFrom(nDev->GetAddress());
    if (!nDev->Send(p, destinationMac, 0x800)) {
        std::cout << "Unable to send packet" << std::endl;
    }
}

bool NetworkDevice::isFamilyMember(int8_t authorId) {
    return std::find(this->familyMembers.begin(), this->familyMembers.end(), authorId) != this->familyMembers.end();
}

void NetworkDevice::printNetworkLog() {
    Printer stringAssembler;
    ostringstream oss;
    oss << "-----" << "Device ID: " << to_string(this->authorId) << "-----" << endl;
    for (auto entry : this->logs) {
        oss << entry.first << ":" <<endl;
        oss << entry.second.second->getLogAsString() << endl;
    }
    oss  << "Neighbours: " << endl;
    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto authorId = iter->first;
        auto nDev = iter->second;
        oss << to_string(authorId) << ": " << ns3::Mac48Address::ConvertFrom(nDev->GetAddress()) << endl;
    }
    oss << "Family member:" << endl;
    for (auto member : this->familyMembers) {
        oss << to_string(member) << endl;
    }
    oss << "------" << "LOG END " << to_string(this->authorId) << "------";
    cout << oss.str() << endl;
}

void NetworkDevice::sendLastEntryTo(int8_t authorId, string type) {
    auto receiverNetDevice = this->neighbourMap[authorId];
    auto receiverMac = ns3::Mac48Address::ConvertFrom(receiverNetDevice->GetAddress());

    if (this->networkLog->getLog().empty()) {
        return;
    }

    LogShell lShell = this->networkLog->getLastEntry();
    NetShell* nShell = new NetShell(receiverMac, authorId, type, 0, &lShell);
    Ptr<Packet> p = this->createPacket(nShell);
    this->sendPacket(receiverNetDevice, p);
}

bool NetworkDevice::isSubSequentSeqNum(LogShell *lShell) {
    return lShell->sequenceNum == this->networkLog->getCurrentSeqNum() + 1;
}

bool NetworkDevice::isMyNeighboursLogUpToDate(LogShell *lShell) {
    return lShell->sequenceNum >= this->networkLog->getCurrentSeqNum();
}

void NetworkDevice::sendEntryFromIndexTo(CommunicationLog* log, int8_t receiverId, int8_t seqFrom, string type) {

    seqFrom = seqFrom == -1 ? 0 : seqFrom;

    auto receiverNetDevice = this->neighbourMap[receiverId];
    Printer p;
    LogShell lShell = log->getLastEntry();
    LogShell* logShell_p = &lShell;
    p.visit(logShell_p);
    auto receiverMac = ns3::Mac48Address::ConvertFrom(receiverNetDevice->GetAddress());
    for (int i = seqFrom; i <= log->getCurrentSeqNum(); i++) {
        auto lShell = log->getEntryAt(i);
        NetShell* nShell = new NetShell(receiverMac, receiverId, type, 0, &lShell);
        Ptr<Packet> p = this->createPacket(nShell);
        this->sendPacket(receiverNetDevice, p);
    }
}

int8_t NetworkDevice::getKeyByValue(Ptr<NetDevice> senderDev) {
    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto authorId = iter->first;
        auto nDev = iter->second;
        if (nDev == senderDev) {
            return authorId;
        }
    }
    cout << "something went wrong" << endl;
    return -1;
}

int8_t NetworkDevice::convertStringToId(string id) {
    int authorId;
    stringstream ssAuthorId(id);
    ssAuthorId >> authorId;
    return authorId;
}

bool NetworkDevice::logExists(NetShell* nShell) {
    return this->logs.find(nShell->type) != this->logs.end();
}

void NetworkDevice::addLog(NetShell* nShell) {
    this->logs.insert({nShell->type, {nShell->shell->authorId, new CommunicationLog(nShell->shell->authorId)}});
}

bool NetworkDevice::concatenateEntry(NetShell* nShell) {
    if (!this->logExists(nShell)) {
        this->logs.insert({nShell->type, {nShell->shell->authorId, new CommunicationLog(nShell->shell->authorId)}});
    }

    CommunicationLog* l = this->logs[nShell->type].second;
    if (l->isSubsequentEntry(*(nShell->shell))) {
        this->packetOss << "& concatenating entry " << to_string(nShell->shell->sequenceNum) << " to " << nShell->type << endl;
        l->addToLog(*(nShell->shell));
        return true;
    } else {
        this->packetOss << "& dropping packet, not matching subsequent entry" << endl;
        return false;
    }
}

EnumFunctions NetworkDevice::hash(string input) {
    if (input == "addMemberToNetwork") return ADD_MEMBER_TO_NETWORK;
    if (input == "plugAndPlay") return PLUG_AND_PLAY;
    if (input == "assignManager") return ASSIGN_MAN;
    if (input == "addSwitch") return ADD_SWITCH;
    if (input == "addToNetwork") return ADD_TO_NETWORK;
    if (input == "getContentFrom") return GET_CONTENT_FROM;
    if (input == "pushContent") return UPDATE_CONTENT_FROM;
    return NONE;
}

bool NetworkDevice::isNeighbourToAdd(const int8_t authorId, const uint8_t hops) {
    return this->neighbourMap.find(authorId) == this->neighbourMap.end() && hops == 0;
}

bool NetworkDevice::isNeighbour(const int8_t authorId) {
    return this->neighbourMap.find(authorId) != this->neighbourMap.end();
}


void NetworkDevice::addNeighbour(int8_t authorId, Ptr <NetDevice> dev) {
    this->packetOss << "Adding neighbour: " << to_string(authorId);
    this->neighbourMap.insert(make_pair(authorId, dev));
}

void NetworkDevice::printPacketResult() {
    cout << this->packetOss.str() << endl;
    this->packetOss.str("");

}

