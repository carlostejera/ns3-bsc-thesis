#include "NetworkDevice.h"

/**
 * Deserializes the packet to a string
 * @param packet packet to read
 * @return
 */
string NetworkDevice::readPacket(Ptr<const Packet> packet) {
    uint32_t pktSize = packet->GetSize();
    uint8_t buf[pktSize];
    memset(buf, 0, pktSize);
    packet->CopyData(buf, pktSize);
    string netShell(buf, buf + pktSize);
    return netShell;
}


/**
 * Converts the given net shell to a string and then it gets converted into a char sequence to finally create the
 * NS3 packet
 * @param nShell net shell to send
 * @return
 */
Ptr <Packet> NetworkDevice::createPacket(NetShell *nShell) {
    Printer stringAssembler;

    stringAssembler.visit(nShell);
    string netShellString = stringAssembler.str();
    stringAssembler.clearOss();
    uint8_t* text = (uint8_t*) netShellString.c_str();
    auto p = Create<Packet>(text, strlen((char *) text));

    return p;
}

/**
 * Sends packet to the given receiver (Net Device)
 * @param nDev receiver
 * @param p packet to send
 */
void NetworkDevice::sendPacket(Ptr<NetDevice> nDev, Ptr<Packet> p) {
    auto destinationMac = ns3::Mac48Address::ConvertFrom(nDev->GetAddress());
    if (!nDev->Send(p, destinationMac, 0x800)) {
        std::cout << "Unable to send packet" << std::endl;
    }
}

/**
 * Checks if the given author ID is part of the network
 * @param authorId
 * @return
 */
bool NetworkDevice::isFamilyMember(std::string authorId) {
    return std::find(this->familyMembers.begin(), this->familyMembers.end(), authorId) != this->familyMembers.end();
}

void NetworkDevice::printNetworkLog() {
    Printer stringAssembler;
    ostringstream oss;
    oss << "-----" << "Device ID: " << this->authorId << "-----" << endl;
    oss << "Communication logs:" << endl;
    for (auto log : this->communicationLogs) {
        oss << log.first << endl;
        oss << log.second->getLogAsString() << endl;
    }
    oss << "Subscriptions:" << endl;
    for (auto log : this->subscriptions) {
        oss << log.first << endl;
        oss << log.second->getLogAsString() << endl;
    }
    oss << endl << "Neighbours: " << endl;
    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto authorId = iter->first;
        auto nDev = iter->second;
        oss << authorId << ": " << ns3::Mac48Address::ConvertFrom(nDev->GetAddress()) << endl;
    }
    oss << "Family member:" << endl;
    for (auto member : this->familyMembers) {
        oss << member << endl;
    }
    oss << "------" << "LOG END " << this->authorId << "------";
    cout << "\033[1;" << "33" << "m" << oss.str() << "\033[0m\n";
}

void NetworkDevice::sendEntryFromIndexTo(CommunicationLog* log, std::string receiverId, int8_t seqFrom, string type) {
    if (receiverId == "-1") {
        this->packetOss << "Not known neighbour. dropped" << endl;
        return;
    }

    if (log == NULL) {
        cout << "no log yet" << endl;
        return;
    }
    seqFrom = seqFrom == -1 ? 0 : seqFrom;
    auto receiverNetDevice = this->neighbourMap[receiverId];
    Printer p;
    cout << "hihi" << endl;
    LogShell lShell = log->getLastEntry();
    LogShell* logShell_p = &lShell;
    p.visit(logShell_p);
    cout << "check" << endl;
    cout << receiverId << endl;
    auto receiverMac = ns3::Mac48Address::ConvertFrom(receiverNetDevice->GetAddress());
    cout << "mate" << endl;
    for (int i = seqFrom; i <= log->getCurrentSeqNum(); i++) {
        auto lShell = log->getEntryAt(i);
        NetShell* nShell = new NetShell(receiverMac, receiverId, type, 0, &lShell);
        Ptr<Packet> p = this->createPacket(nShell);
        this->sendPacket(receiverNetDevice, p);
    }
}

std::string NetworkDevice::getKeyByValue(Ptr<NetDevice> senderDev) {
    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto authorId = iter->first;
        auto nDev = iter->second;
        if (nDev == senderDev) {
            return authorId;
        }
    }
    cout << "something went wrong" << endl;
    return "-1";
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

bool NetworkDevice::concatenateEntry(NetShell* nShell) {
    if (!this->logExists(nShell)) {
        // TODO: Maybe change
        this->logs.insert({nShell->type, {nShell->shell->authorId, new CommunicationLog(nShell->shell->authorId, "108")}});
        if (nShell->type.find(SWITCH_PREFIX) != string::npos && nShell->type.find(USER_PREFIX) != string::npos) {
            this->communicationLogs.push_back({nShell->type,this->logs[nShell->type].second});
        } else {
            this->subscriptions.push_back({nShell->type, this->logs[nShell->type].second});
        }
        cout << "xdddddddddddddddddddd" << endl;
    }

    return this->isEntryConcatenated(nShell);
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

bool NetworkDevice::isNeighbourToAdd(const std::string authorId, const uint8_t hops) {
    return this->neighbourMap.find(authorId) == this->neighbourMap.end() && hops == 0;
}

bool NetworkDevice::isNeighbour(const std::string authorId) {
    return this->neighbourMap.find(authorId) != this->neighbourMap.end();
}


void NetworkDevice::addNeighbour(std::string authorId, Ptr<NetDevice> dev) {
    this->packetOss << "Adding neighbour: " << authorId;
    this->neighbourMap.insert(make_pair(authorId, dev));
}

void NetworkDevice::printPacketResult() {
    cout << this->packetOss.str() << endl;
    this->packetOss.str("");

}
bool NetworkDevice::isGossipEntryOlder(NetShell *nShell) {
    return this->logs[nShell->type].second->getCurrentSeqNum() > nShell->shell->sequenceNum;
}
bool NetworkDevice::isEntryConcatenated(NetShell* netShell) {
    CommunicationLog* l = this->logs[netShell->type].second;
    string conc = "& concatenating entry " + to_string(netShell->shell->sequenceNum) + " to " + netShell->type + "\n";
    string drop = "& dropping packet, not matching subsequent entry\n";
    bool result = l->addToLog(*(netShell->shell));
    this->packetOss << (result ? conc : drop);
    return result;
}

