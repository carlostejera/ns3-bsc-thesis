#include "EthSwitch.h"
#include "shells/LogFunctions.h"
#include "GlobalsValues.h"


using namespace ns3;
using namespace std;

void EthSwitch::requestJoiningNetwork() {
    CommunicationLog* log = new CommunicationLog(this->authorId);

    ContentShell *cShell = new ContentShell(
            "addToNetwork",
            to_string(this->authorId),
            to_string(this->authorId) + "wants to join the network"
            );
    LogShell *logShell = new LogShell(
            0,
            "",
            this->authorId,
            cShell
            );

    string commLog = "switch" + to_string(this->authorId) + "/manager*";
    NetShell *nShell = new NetShell(
            ns3::Mac48Address("FF:FF:FF:FF:FF:FF"),
            127,
            commLog,
            0,
            logShell);
    log->addToLog(*logShell);
    this->logs.insert({commLog, {127, log}});
    // Broadcast that the user wants to join the network (simulating via uni-cast)
    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
        Ptr <Packet> p = this->createPacket(nShell);
        Ptr <NetDevice> dev = GetNode()->GetDevice(i);
        if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }
    }
}


void EthSwitch::assignManager(Ptr<NetDevice> sender, int8_t managerId) {
    this->manager = make_pair(managerId, sender);
}

void EthSwitch::addMemberToNetwork(string params) {
    int authorId;
    stringstream ssAuthorId(params);
    ssAuthorId >> authorId;
    this->familyMembers.push_back(authorId);
}

void EthSwitch::printNetworkLog() {
    NetworkDevice::printNetworkLog();
    ostringstream oss;
    Printer stringAssembler;
    oss << "Manager: " << to_string(this->manager.first) << endl;
    oss << "Connected users:";
    for (auto user : this->connectedUser) {
        oss << to_string(user) << " ";
    }
    if (!this->connectedUser.empty())
        oss << "None" << endl;

    oss << "subscriptions:" << endl;
    for (auto entry: this->logsOfUsers) {
        oss << "log for " << to_string(entry.first) << endl;
        for (auto logShell : this->logsOfUsers[entry.first]->getLog()) {
            stringAssembler.visit(&logShell);
            oss << stringAssembler.str() << endl;
            stringAssembler.clearOss();
        }
    }
    oss << endl << "----------------------" << endl;
    cout << oss.str() << endl;
}


void EthSwitch::sendPlugAndPlayConfirmation(Ptr<NetDevice> nDev, int8_t author) {
    LogShell* log = new LogShell(
            0,
            "",
            this->authorId,
            new ContentShell(
                    "addSwitch",
                    to_string(this->authorId),
                    "Add switch to the connected list"
            ));
    string logName = "switch" + to_string(this->authorId) + "/user" + to_string(author);
    CommunicationLog* cLog = new CommunicationLog(this->authorId, log);
    this->logs.insert({logName, {author, cLog}});
    cout << "test" << endl;
    LogShell tmp = this->logs[logName].second->getLastEntry();
    cout << "test" << endl;

    Ptr<Packet> p = this->createPacket(
            new NetShell(
                    ns3::Mac48Address::ConvertFrom(nDev->GetAddress()),
                    author,
                    "switch" + to_string(this->authorId) + "/user" + to_string(author),
                    0,
                    &(tmp)
                    )
    );
    cout << "test" << endl;

    this->sendPacket(nDev, p);
    cout << "test" << endl;

}

void EthSwitch::gossip() {
    string randomLogType;
    int count = 0;
    do {
        if (count == 10) {
            return;
        }
        map<string, pair<int8_t , CommunicationLog*>>::iterator item = this->logs.begin();
        advance(item, rand() % this->logs.size());
        randomLogType = item->first;
        cout << this->authorId << endl;
        cout << (randomLogType.find("switch" + to_string(this->authorId) + "/") == string::npos) << endl;
        count++;
    } while (randomLogType.find("switch" + to_string(this->authorId) + "/") != string::npos);
    auto log = this->logs[randomLogType].second;
    NetShell* nShell;
    for (auto entry : this->neighbourMap) {
        if (log->getLog().empty()) {
            auto cShell = new ContentShell("f", "p", "I have no content");
            auto lShell = new LogShell(-1, "", this->manager.first, cShell);
            nShell = new NetShell(Mac48Address::ConvertFrom(entry.second->GetAddress()), entry.first, randomLogType, 0, lShell);
//            if (entry.first != this->manager.first) {
                auto packet = this->createPacket(nShell);
                this->sendPacket(entry.second, packet);
//            }
        } else {
            LogShell tmp = log->getLastEntry();
            LogShell* p = &tmp;
            nShell = new NetShell(Mac48Address::ConvertFrom(entry.second->GetAddress()), entry.first, randomLogType, 0, p);
//            if (entry.first != this->manager.first) {
                auto packet = this->createPacket(nShell);
                this->sendPacket(entry.second, packet);
//            }
        }
    }
}



void EthSwitch::recvPkt(
        Ptr <NetDevice> dev,
        Ptr<const Packet> packet,
        uint16_t proto,
        const Address &from,
        const Address &to,
        NetDevice::PacketType pt) {


    // Reads packet size and prepares the transformation of bytes to string
    string netShell = this->readPacket(packet);

    // Transforms the string to a net shell object
    NetShell* nShell = SomeFunctions::shell(netShell);


    // Records every sender to keep a map of the neighbours
    if (this->isNeighbourToAdd(this->getKeyByValue(dev), nShell->hops)) {
        this->addNeighbour(nShell->shell->authorId, dev);
    }


    Ptr<Packet> test = Create<Packet>(*packet);
    if (this->rem->IsCorrupt(test)) {
        return;
    }
    this->packetOss << "------------------SwitchPacket-------------------" << endl
        << "i am " << to_string(this->authorId) << endl;


    this->packetOss << "Received: " << netShell << endl;
    this->packetOss << "From: " << ns3::Mac48Address::ConvertFrom(dev->GetAddress()) << endl;
    this->packetOss << "Result: ";


    // Checks if it is the receiver
    if (nShell->type.find("/manager") != string::npos) {
        this->packetOss << "Dropping packet" << endl;

    } else if (nShell->type.find("/switch") != string::npos) {

        if (this->processReceivedSwitchPacket(nShell, dev)) {
            auto lastEntry = this->logs[nShell->type].second->getLastEntry();
            //Dropping packet if this switch is not the receiver
            switch (this->hash(lastEntry.shell->function)) {
                case ADD_MEMBER_TO_NETWORK:
                    this->addMemberToNetwork(lastEntry.shell->params);
                    break;
                case PLUG_AND_PLAY:
                    this->sendPlugAndPlayConfirmation(dev, nShell->shell->authorId);
                    break;
                case GET_CONTENT_FROM:
                    break;
                default:
                    break;
            }
        }

    } else if (nShell->type.find("/user") != string::npos) {
        this->processReceivedUserPacket(nShell, dev);

    } else {
        this->packetOss << "Dropping unknown packet";
    }

    this->packetOss << "----------------SwitchPacket_END----------------\n" << endl;
    if (VERBOSE) {
        this->printPacketResult();
    }
    this->packetOss.clear();
}

bool EthSwitch::isInList(vector<int8_t> v, int8_t authorId) {
    return find(v.begin(), v.end(), authorId) != v.end();
}

void EthSwitch::forward(Ptr<NetDevice> dev, NetShell* nShell, uint8_t hops) {
    typedef multimap<string, int8_t>::iterator MMAPIterator;
    auto p = SomeFunctions::varSplitter(nShell->type, "/");
    pair<MMAPIterator, MMAPIterator> result = this->interestedNeighbours.equal_range(p.first);
    for (MMAPIterator it = result.first; it != result.second; it++) {
        dev = this->neighbourMap[it->second];
        auto p = this->createPacket(nShell);
        this->sendPacket(dev, p);
    }
}

bool EthSwitch::processReceivedSwitchPacket(NetShell *nShell, Ptr <NetDevice> dev) {
    // Check if receiver
    if(this->logExists(nShell)) {
        // Gossip answer
        if (this->logs[nShell->type].second->getCurrentSeqNum() > nShell->shell->sequenceNum) {
            this->sendEntryFromIndexTo(this->logs[nShell->type].second, this->getKeyByValue(dev),
                                       nShell->shell->sequenceNum, nShell->type);
            return false;
        }
    }
    if (nShell->type.find("manager") != string::npos && nShell->receiverId == this->authorId) {
        this->manager = {nShell->shell->authorId, dev};
        this->packetOss << "& assigning manager " << endl;
    }
    return this->concatenateEntry(nShell);
}

void EthSwitch::processReceivedUserPacket(NetShell *nShell, Ptr <NetDevice> dev) {
    if (this->hash(nShell->shell->shell->function) == GET_CONTENT_FROM) {
        auto p = SomeFunctions::varSplitter(nShell->type, "/");

        this->interestedNeighbours.insert({p.second, this->getKeyByValue(dev)});

        // TODO: Changer hard coded condition
        if (this->isNeighbour(this->convertStringToId(nShell->shell->shell->params)) || this->logs.find("user1/user*") != this->logs.end()) {
            cout << "he is here" << endl;
            cout << "user" + nShell->shell->shell->params + "/user*" << endl;
            this->sendEntryFromIndexTo(this->logs["user" + nShell->shell->shell->params + "/user*"].second, this->getKeyByValue(dev), 0, "user" + nShell->shell->shell->params + "/user*");
        } else {
            nShell->type = "switch" + to_string(this->authorId) + "/user" + nShell->shell->shell->params;
            this->logs.insert({nShell->type, {this->convertStringToId(nShell->shell->shell->params), new CommunicationLog(this->authorId, nShell->shell)}});
            this->broadcastToNeighbours(dev, nShell);
            this->packetOss << "& and forwarding request";
        }


    } else if (this->hash(nShell->shell->shell->function) == UPDATE_CONTENT_FROM) {
        cout << "*******" << endl;
        cout << "should be here" << endl;

        // User pushes to the connected switch
        if(this->concatenateEntry(nShell)) {
            this->forward(dev, nShell, ++nShell->hops);
        }
    }
}

void EthSwitch::broadcastToNeighbours(Ptr <NetDevice> dev, NetShell *nShell) {
    for (auto entry : this->neighbourMap) {
        cout << "sending packets" << endl;
        if (entry.second != dev && entry.first != this->manager.first) {
            auto p = this->createPacket(nShell);
            this->sendPacket(entry.second, p);
        }
    }
}