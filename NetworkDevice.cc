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
    oss << "------" << "LOG END " << to_string(this->authorId) << "------";
    cout << oss.str() << endl;
}

void NetworkDevice::sendLastEntryTo(int8_t authorId) {
    auto receiverNetDevice = this->neighbourMap[authorId];
    auto receiverMac = ns3::Mac48Address::ConvertFrom(receiverNetDevice->GetAddress());
    LogShell lShell = this->networkLog->getLastEntry();
    NetShell* nShell = new NetShell(receiverMac, authorId, LOG_ENTRY, &lShell);
    Ptr<Packet> p = this->createPacket(nShell);
    this->sendPacket(receiverNetDevice, p);
}
