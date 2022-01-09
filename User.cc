#include "User.h"

using namespace ns3;
using namespace std;

void
User::recvPkt(Ptr <NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address &from, const Address &to,
              NetDevice::PacketType pt) {
    ostringstream oss;
    oss << "------------------UserPacket-------------------" << endl
        << "i am " << this->authorId << endl;

    string netShell = this->readPacket(packet);
    NetShell *nShell = SomeFunctions::shell(netShell);
    oss << "Received: " << netShell << endl;
    oss << "Result: ";
    cout << "Received: " << netShell << endl;

    if (this->isNeighbourToAdd(this->getKeyByValue(dev), nShell->hops)) {
        this->addNeighbour(nShell->shell->authorId, dev);
    }


    if (nShell->type.find("/user:") != string::npos) {
        if (nShell->shell->authorId != this->authorId) {
            switch (this->hash(nShell->shell->shell->function)) {
                case ADD_SWITCH:
                    this->neighbourMap.insert(make_pair(nShell->shell->authorId, dev));
                    break;
                case UPDATE_CONTENT_FROM:
                    this->concatenateEntry(nShell);
                    /*if (this->logPacket.getLogByWriterReader(LOGTYPE("user:1", USER_ALL))->getCurrentSeqNum() == 1) {
                        cout << "////////////////////////////////////////////////" << endl;
                        cout << "////////////////////////////////////////////////" << endl;
                        cout << "///// MY START TIME IS " << to_string(this->timer) << "///////////////" << endl;
                        cout << "////" << "SIMULATOR TIME" << to_string(Simulator::Now().GetSeconds() - this->timer) << "     " << "////" << endl;
                        cout << "////////////////////////////////////////////////" << endl;
                        cout << "////////////////////////////////////////////////" << endl;
                        Simulator::Stop();

                    }*/
                default:
                    cout << "choking" << endl;
                    std::string senderId;
                    // gossip answer
                    if(nShell->type != this->authorId + "/user:*") {
                        return;
                    }

                    for (auto entry : this->neighbourMap) {
                        if (entry.second == dev) {
                            senderId = entry.first;
                        }
                    }
                    this->sendEntryFromIndexTo(this->myPersonalLog, senderId, nShell->shell->sequenceNum,
                                               LOGTYPE(authorId, USER_ALL));
                    break;
            }
        }

    }



    cout << oss.str() << endl;
}

void User::joinNetwork() {}

void User::subscribe(std::string authorId) {
//    this->subscriptions.push_back({authorId + "/user:*", new CommunicationLog(authorId, "user:*")});
    for (auto item : this->neighbourMap) {
        auto neighbourId = item.first;
//        auto mac = ns3::Mac48Address::ConvertFrom(item.second->GetAddress());

        auto cShell = new ContentShell("getContentFrom", authorId,"Subscribe the author " + authorId);
        auto type = this->authorId + "/switch:*";
        this->logPacket.getLogByWriterReader(type)->appendLogShell(cShell);
        auto lShell = this->logPacket.getLogByWriterReader(type)->getLastEntry();
        auto nShell = new NetShell(authorId, type, 0, 0, &lShell);
        auto p = this->createPacket(nShell);
        this->sendPacket(item.second, p);
    }
    this->timer = Simulator::Now().GetSeconds();
    cout << "I am subscribing" << endl;
}

void User::plugAndPlay() {
    auto comm = new CommunicationLog(this->authorId, SWITCH_ALL);

    ContentShell *cShell = new ContentShell("plugAndPlay",this->authorId,this->authorId + " plug and play");
    comm->appendLogShell(cShell);

    string logName = this->authorId + "/switch:*";
    this->logPacket.add(LogPacket(logName, comm, CommunicationType::P2P_COMM));

    auto logShell = this->logPacket.getLogByWriterReader(logName)->getLastEntry();
    LogShell* lShell_p = &logShell;
    NetShell *nShell = new NetShell(SWITCH_ALL, logName, 0, 0, lShell_p);

    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
        Ptr <Packet> p = this->createPacket(nShell);
        Ptr <NetDevice> dev = GetNode()->GetDevice(i);

        if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }
    }
}

void User::printNetworkLog() {
    ostringstream oss;
    Printer stringAssembler;
    NetworkDevice::printNetworkLog();
    oss << "-------User " << this->authorId << "-------" << endl;
    oss << "My Log: " << endl;
    for (auto lShell : this->myPersonalLog->getLog()) {
        stringAssembler.visit(&lShell);
        oss << stringAssembler.str() << endl;
        stringAssembler.clearOss();
    }
    oss << "Connected Switches: " << endl;
    for (auto item : this->neighbourMap) {
        oss << item.first << endl;
    }
    oss << "-----------------------" << endl;
    cout << oss.str() << endl;
}


void User::pushLogToSwitch() {
    this->myPersonalLog->appendLogShell(new ContentShell("pushContent", "", "this is my log " + to_string(this->count)));
    this->count += 1;
    LogShell lShell = this->myPersonalLog->getLastEntry();
    LogShell *shell_p = &lShell;

    for (auto entry: this->neighbourMap) {
        NetShell *netShell = new NetShell(entry.first,
                                          LOGTYPE(this->authorId, USER_ALL), 0, 0, shell_p);
        auto p = this->createPacket(netShell);
        this->sendPacket(entry.second, p);
    }

}

bool User::processReceivedSwitchPacket(NetShell *netShell, Ptr <NetDevice> dev) {
    return true;

}

void User::processReceivedUserPacket(NetShell *netShell, Ptr <NetDevice> dev) {

}
void User::unsubscribe(std::string authorId) {
    for (auto item : this->neighbourMap) {
        auto neighbourAuthor = item.first;
        auto dev = item.second;

        auto log = this->logPacket.getLogByWriterReader(LOGTYPE(this->authorId, SWITCH_ALL));
        auto type = LOGTYPE(log->getOwner(), log->getDedicated());
//        auto mac = ns3::Mac48Address::ConvertFrom(dev->GetAddress());

        auto cShell = new ContentShell(UNSUBSCRIBE, authorId,"Unsubscribe the neighbourAuthor " + authorId);
        log->appendLogShell(cShell);
        auto lShell = log->getLastEntry();
        auto nShell = new NetShell(neighbourAuthor, type, 0, 0, &lShell);
        auto p = this->createPacket(nShell);
        this->sendPacket(dev, p);
    }
    cout << "I am unsubscribing" << endl;
    this->removeSubscription(authorId);

}
