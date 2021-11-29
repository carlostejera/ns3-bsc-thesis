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

    return Create<Packet>(text, strlen((char *) text));
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
    for (LogShell lShell : this->networkLog->getLog()) {
        stringAssembler.visit(&lShell);
        oss << stringAssembler.str() << endl;
        stringAssembler.clearOss();
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
    LogShell lShell = this->networkLog->getLastEntry();
    NetShell* nShell = new NetShell(receiverMac, authorId, type, &lShell);
    Ptr<Packet> p = this->createPacket(nShell);
    this->sendPacket(receiverNetDevice, p);
}

bool NetworkDevice::isSubSequentSeqNum(LogShell *lShell) {
    return lShell->sequenceNum == this->networkLog->getCurrentSeqNum() + 1;
}

bool NetworkDevice::isMyNeighboursLogUpToDate(LogShell *lShell) {
    return lShell->sequenceNum >= this->networkLog->getCurrentSeqNum();
}

void NetworkDevice::sendEntryFromIndexTo(int8_t authorId, int8_t seqFrom) {
    auto receiverNetDevice = this->neighbourMap[authorId];
    cout << receiverNetDevice->GetAddress() << endl;

    auto receiverMac = ns3::Mac48Address::ConvertFrom(receiverNetDevice->GetAddress());
    cout << receiverMac << endl;
    for (int i = seqFrom; i <= this->networkLog->getCurrentSeqNum(); i++) {
        auto lShell = this->networkLog->getEntryAt(i);
        NetShell* nShell = new NetShell(receiverMac, authorId, LOG_ENTRY, &lShell);
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
