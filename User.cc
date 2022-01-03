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


    if (nShell->receiverId == this->authorId) {
        switch (this->hash(nShell->shell->shell->function)) {
            case ADD_SWITCH:
                this->neighbourMap.insert(make_pair(nShell->shell->authorId, dev));
                break;
            case UPDATE_CONTENT_FROM:
                this->concatenateEntry(nShell);
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
                cout << senderId << endl;
                this->sendEntryFromIndexTo(this->myPersonalLog, senderId, nShell->shell->sequenceNum,
                                           LOGTYPE(authorId, USER_ALL));
                break;
        }
    }



    cout << oss.str() << endl;
}

void User::joinNetwork() {}

void User::subscribe(std::string authorId) {
//    this->subscriptions.push_back({authorId + "/user:*", new CommunicationLog(authorId, "user:*")});
    for (auto item : this->neighbourMap) {
        auto neighbourId = item.first;
        auto mac = ns3::Mac48Address::ConvertFrom(item.second->GetAddress());

        auto cShell = new ContentShell("getContentFrom", authorId,"Subscribe the author " + authorId);
        auto type = this->authorId + "/switch:*";
        this->communicationLogs[type]->appendLogShell(cShell);
        auto lShell = this->communicationLogs[type]->getLastEntry();
        auto nShell = new NetShell(mac, authorId, type, 0, 0,&lShell);
        auto p = this->createPacket(nShell);
        this->sendPacket(item.second, p);
    }
    cout << "I am subscribing" << endl;
}

void User::plugAndPlay() {
    ContentShell *cShell = new ContentShell("plugAndPlay",this->authorId,this->authorId + " plug and play");
    LogShell *logShell = new LogShell(0, "", this->authorId, cShell);
    string logName = this->authorId + "/switch:*";
    NetShell *nShell = new NetShell(ns3::Mac48Address("FF:FF:FF:FF:FF:FF"),"127",logName,0, 0, logShell);
//    this->logs.insert({logName, {"127", new CommunicationLog(this->authorId)}});
    this->communicationLogs.insert({logName, new CommunicationLog(this->authorId, SWITCH_ALL)});
    this->communicationLogs[logName]->addToLog(*logShell);
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
    for (auto item: this->communicationLogs) {
        oss << "Log of " << item.first << endl;
        for (auto lShell: item.second->getLog()) {
            stringAssembler.visit(&lShell);
            oss << stringAssembler.str() << endl;
            stringAssembler.clearOss();
        }
    }
    oss << "Logs:" << endl;
    for (auto item: this->subscriptions) {
        oss << "Log of " << item.first << endl;
        for (auto lShell: item.second->getLog()) {
            stringAssembler.visit(&lShell);
            oss << stringAssembler.str() << endl;
            stringAssembler.clearOss();
        }
    }
    oss << "-----------------------" << endl;
    cout << oss.str() << endl;
}


void User::pushLogToSwitch() {
    this->myPersonalLog->addToLog(LogShell(this->count, this->myPersonalLog->getLog().empty() ? "" : this->myPersonalLog->createHash(this->myPersonalLog->getLastEntry()), this->authorId, new ContentShell("pushContent", "", "this is my log " +
            to_string(this->count))));
    this->count += 1;
    LogShell lShell = this->myPersonalLog->getLastEntry();
    LogShell *shell_p = &lShell;

    for (auto entry: this->neighbourMap) {
        NetShell *netShell = new NetShell(ns3::Mac48Address::ConvertFrom(entry.second->GetAddress()), entry.first,
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
        for (auto i : this->communicationLogs) {
        }
        auto log = this->communicationLogs[LOGTYPE(this->authorId, SWITCH_ALL)];
        auto type = LOGTYPE(log->getOwner(), log->getDedicated());
        auto mac = ns3::Mac48Address::ConvertFrom(dev->GetAddress());

        auto cShell = new ContentShell(UNSUBSCRIBE, authorId,"Unsubscribe the neighbourAuthor " + authorId);
        log->appendLogShell(cShell);
        auto lShell = log->getLastEntry();
        auto nShell = new NetShell(mac, neighbourAuthor, type, 0, 0, &lShell);
        auto p = this->createPacket(nShell);
        this->sendPacket(dev, p);
    }
    cout << "I am unsubscribing" << endl;
    this->removeSubscription(authorId);

}
