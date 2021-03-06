#include "Manager.h"
#include "GlobalsValues.h"

using namespace ns3;
using namespace std;

void Manager::registerUser(Ptr<NetDevice> dev, std::string authorId) {
    ostringstream oss;
    oss << "New member arrived. Add " << authorId << " to member list";
    this->neighbourMap.insert(make_pair(authorId, dev));
    this->familyMembers.push_back(authorId);
    this->myPersonalLog->appendLogShell(new ContentShell(ADD_MEMBER, authorId, oss.str()));
    this->currSeq++;
}

void Manager::sendNetworkJoinConfirmation(std::string authorId) {
    // Sends a confirmation to the join requester and also all necessary entries to reconstruct the current network
/*    Ptr<NetDevice> nDev = this->neighbourMap[authorId];
    auto destinationMac = ns3::Mac48Address::ConvertFrom(nDev->GetAddress());
    ContentShell *cShell = new ContentShell(ASSIGN_MANAGER, this->authorId, "Request confirmed. Network is joined.");
    LogShell *logShell =  new LogShell(-1, "prev", this->authorId, cShell);

    NetShell* nShell = new NetShell(destinationMac, authorId, DIARY, 0, 0, logShell);

    Ptr <Packet> p = this->createPacket(nShell);
    this->sendPacket(nDev, p);*/
}

void Manager::broadcastLastNetworkChange(std::string exceptedReceiver ="-1") {
    auto seq = this->myPersonalLog->getLastEntry().sequenceNum;
    auto type = this->authorId + "/switch:*";

    for (auto iter = this->neighbourMap.begin(); iter != this->neighbourMap.end(); iter++) {
        auto receiverAuthor = iter->first;

        // TODO: Drop one packet on purpose
        if (receiverAuthor == "3" && exceptedReceiver == "4")
            continue;

        if (receiverAuthor != exceptedReceiver) {
            this->sendEntryFromIndexTo(this->myPersonalLog, receiverAuthor, seq, type);
        }
    }
}

bool Manager::concatenateEntry(NetShell* netShell) {
    if (!this->logExists(netShell)) {
        // TODO: Maybe change
        this->logPacket.add(LogPacket(netShell->type, new CommunicationLog(netShell->shell->authorId,
                                                                           this->authorId,
                                                                           -1), CommunicationType::P2P_COMM));
    }
    CommunicationLog* log = this->logPacket.getLogByWriterReader(netShell->type);
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
        << "Manager: " << this->authorId << endl;

    string netShell = this->readPacket(packet);
    NetShell* nShell = Parser::shell(netShell);

    this->packetOss << "Received: " << netShell << endl;
    this->packetOss << "Result: ";

    // Broadcasts for the manager
    if (nShell->type.find(MANAGER_ALL) != string::npos) {

        this->concatenateEntry(nShell);

        auto lastEntry = this->logPacket.getLogByWriterReader(nShell->type)->getLastEntry();
        switch(this->hash(lastEntry.shell->function)) {
            case ADD_TO_NETWORK:
                this->registerUser(dev, nShell->shell->authorId);
                this->packetOss << "Add neighbour " << nShell->shell->authorId << " & register Switch ";
                this->sendEntryFromIndexTo(this->myPersonalLog, nShell->shell->authorId, 0, this->myType);
                this->packetOss << "& sending log " << "manager" + this->authorId + "/switch* ";
                this->broadcastLastNetworkChange(nShell->shell->authorId);
                this->packetOss << "& broadcasting changes" << endl;
                break;
            default:
                break;
        }

        // Gossip requests
    } else if (nShell->type.find(this->authorId + "/switch:*") != string::npos) {
        if (this->myPersonalLog->getCurrentSeqNum() > nShell->shell->sequenceNum) {
            auto seq = nShell->shell->sequenceNum;
            this->sendEntryFromIndexTo(this->myPersonalLog,
                                       this->getKeyByValue(dev), seq, "manager" + this->authorId + "/switch*");
        }
        this->packetOss << "& nothing needed from me" << endl;
    } else {
        this->packetOss << "Dropping packet, unknown content" << endl;
    }

    this->packetOss << "---------------ManagerPacket_END----------------\n" << endl;
    if (VERBOSE) {
         cout << this->packetOss.str() << endl;
    }
}

bool Manager::processReceivedSwitchPacket(NetShell *netShell, Ptr <NetDevice> dev) {
    return true;
}

void Manager::processReceivedUserPacket(NetShell *netShell, Ptr <NetDevice> dev) {

}
