#include "EthSwitch.h"
#include "shells/LogFunctions.h"
#include "GlobalsValues.h"


using namespace ns3;
using namespace std;
/**
 * Switch tries to join the network by asking for the manager
 */

void EthSwitch::requestJoiningNetwork() {
    CommunicationLog* log = new CommunicationLog(this->authorId, 127);

    ContentShell *cShell = new ContentShell("addToNetwork",to_string(this->authorId),to_string(this->authorId) + " wants to join the network");
    LogShell *logShell = new LogShell(0,"",this->authorId,cShell);

    string commLog = "switch:" + to_string(this->authorId) + "/manager:*";
    NetShell *nShell = new NetShell(ns3::Mac48Address("FF:FF:FF:FF:FF:FF"),127,commLog,0,logShell);
    log->addToLog(*logShell);
    this->logs.insert({commLog, {127, log}});
    this->communicationLogs.push_back({commLog, log});
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
    this->manager = {managerId, sender};
    this->packetOss << "& assigning manager " << endl;
    this->isManagerAssigned = true;
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
    oss << "Connected users: " << endl;
    for (auto user : this->connectedUser) {
        oss << to_string(user) << " " << endl;
    }
    if (this->connectedUser.empty())
        oss << "None" << endl;
    oss << "----------------------" << endl;
    cout << "\033[1;36m" << oss.str() << "\033[0m\n";
}

/**
 * Answering the user about its connection to the switch
 * @param nDev
 * @param connectingUser
 */
void EthSwitch::sendPlugAndPlayConfirmation(Ptr<NetDevice> nDev, int8_t connectingUser) {
    LogShell* log = new LogShell(
            0,
            "",
            this->authorId,
            new ContentShell(
                    "addSwitch",
                    to_string(this->authorId),
                    "Add switch to the connected list"
            ));

    string logName = "switch:" + to_string(this->authorId) + "/user:" + to_string(connectingUser);
    CommunicationLog* cLog = new CommunicationLog(this->authorId, connectingUser);
    cLog->addToLog(*log);

    this->logs.insert({logName, {connectingUser, cLog}});
    LogShell tmp = this->logs[logName].second->getLastEntry();

    this->connectedUser.push_back(connectingUser);

    Ptr<Packet> p = this->createPacket(
            new NetShell(
                ns3::Mac48Address::ConvertFrom(nDev->GetAddress()),
                connectingUser,
                logName,
                0,
                &(tmp)
                    )
    );
    this->sendPacket(nDev, p);
}

void EthSwitch::gossip() {

    // Choose a random log to gossip about
/*    do {
        if (count == 10) {
            return;
        }
        map<string, pair<int8_t , CommunicationLog*>>::iterator item = this->logs.begin();
        advance(item, rand() % this->logs.size());
        randomLogType = item->first;
        count++;
    } while (randomLogType.find("switch:" + to_string(this->authorId) + "/") != string::npos);

    auto log = this->logs[randomLogType].second;*/
    auto number =  rand() % this->subscriptions.size();
    auto randomLogType = this->subscriptions[number].first;
    auto log = this->subscriptions[number].second;
    NetShell* nShell;

    for (auto entry : this->neighbourMap) {
        if (log->getLog().empty()) { // If the chosen log is empty, tell that your neighbours with a fake log entry to get help
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
    Simulator::Schedule(Seconds(5), &EthSwitch::gossip, this);
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

    this->packetOss << "------------------SwitchPacket-------------------" << endl
        << "i am " << to_string(this->authorId) << endl;


    this->packetOss << "Received: " << netShell << endl;
    this->packetOss << "From: " << ns3::Mac48Address::ConvertFrom(dev->GetAddress()) << endl;
    this->packetOss << "Result: ";


    // Checks if it is the receiver
    if (nShell->type.find("/manager") != string::npos) {
        this->packetOss << "Dropping packet" << endl;

    } else if (nShell->type.find("/switch:") != string::npos) {
        cout << netShell << endl;
        cout << netShell << endl;
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
        if (this->isGossipEntryOlder(nShell)) {
            this->sendEntryFromIndexTo(this->getLogFrom(nShell->type), this->getKeyByValue(dev),
                                       nShell->shell->sequenceNum, nShell->type);
            return false;
        }
    }
    if (nShell->type.find("manager") != string::npos && nShell->receiverId == this->authorId && !this->isManagerAssigned) {
        this->assignManager(dev, nShell->shell->authorId);
        Simulator::Schedule(Seconds(5), &EthSwitch::gossip, this);
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
            auto toSubscribe = "user:" + nShell->shell->shell->params + "/user:*";
            this->sendEntryFromIndexTo(this->getLogFrom(toSubscribe), this->getKeyByValue(dev), 0, toSubscribe);
        } else {
            // Somebody is interested in a log, so am I
            nShell->type = "switch:" + to_string(this->authorId) + "/user:" + nShell->shell->shell->params;
            auto cLog = new CommunicationLog(this->authorId, this->convertStringToId(nShell->shell->shell->params));
            cLog->addToLog(*nShell->shell);
            this->logs.insert({nShell->type, {this->convertStringToId(nShell->shell->shell->params), cLog}});
            this->broadcastToNeighbours(dev, nShell);
            this->packetOss << "& and forwarding request";
        }


    } else if (this->hash(nShell->shell->shell->function) == UPDATE_CONTENT_FROM) { // User pushes content to switch
        if(this->concatenateEntry(nShell)) { // If new entry gets concatenated, then push to the subscribers
            this->forward(dev, nShell, ++nShell->hops);
        }
    } else {
        if(this->logExists(nShell)) {
            // Gossip answer
            if (this->isGossipEntryOlder(nShell)) {
                this->sendEntryFromIndexTo(this->getLogFrom(nShell->type), this->getKeyByValue(dev),
                                           nShell->shell->sequenceNum, nShell->type);
            }
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

CommunicationLog *EthSwitch::getLogFrom(string type) {
    for (auto l : this->subscriptions) {
        if (l.first == type) {
            return l.second;
        }
    }
    return NULL;
}
