#include "EthSwitch.h"
#include "shells/LogFunctions.h"
#include "GlobalsValues.h"


using namespace ns3;
using namespace std;
/**
 * Switch tries to join the network by asking for the manager
 */

void EthSwitch::requestJoiningNetwork() {
    CommunicationLog* log = new CommunicationLog(this->authorId, "127");

    ContentShell *cShell = new ContentShell("addToNetwork",this->authorId,this->authorId + " wants to join the network");
    LogShell *logShell = new LogShell(0,"",this->authorId,cShell);

    string commLog = this->authorId + "/manager:*";
    NetShell *nShell = new NetShell(ns3::Mac48Address("FF:FF:FF:FF:FF:FF"),"127",commLog,0,logShell);
    log->addToLog(*logShell);
    this->logs.insert({commLog, {"127", log}});
    this->communicationLogs.insert({commLog, log});
    // Broadcast that the user wants to join the network (simulating via uni-cast)
    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
        Ptr <Packet> p = this->createPacket(nShell);
        Ptr <NetDevice> dev = GetNode()->GetDevice(i);
        if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }
    }
}


void EthSwitch::assignManager(Ptr<NetDevice> dev, std::string manager) {
    this->manager = {manager, dev};
    this->packetOss << "& assigning manager " << endl;
    this->isManagerAssigned = true;
}

void EthSwitch::addMemberToNetwork(string params) {
    std::string  authorId = params;
    this->familyMembers.push_back(authorId);
}

void EthSwitch::printNetworkLog() {
    NetworkDevice::printNetworkLog();
    ostringstream oss;
    Printer stringAssembler;
    oss << "Manager: " << this->manager.first << endl;
    oss << "Connected users: " << endl;
    for (auto user : this->connectedUser) {
        oss << user << " " << endl;
    }
    if (this->connectedUser.empty())
        oss << "None" << endl;
    oss << "\n";
    oss << "Interested neighbours:" << endl;
    for(auto neighbour : this->interestedNeighbours) {
        oss << neighbour.first << " " << neighbour.second << endl;
    }


    oss << "----------------------" << endl;
    cout << "\033[1;36m" << oss.str() << "\033[0m\n";
}

/**
 * Answering the user about its connection to the switch
 * @param dev
 * @param authorId
 */
void EthSwitch::sendPlugAndPlayConfirmation(Ptr<NetDevice> dev, std::string authorId) {
    LogShell* log = new LogShell(
            0,
            "",
            this->authorId,
            new ContentShell(
                    "addSwitch",
                    this->authorId,
                    "Add switch to the connected list"
            ));

    string logName = this->authorId + "/" + authorId;
    CommunicationLog* cLog = new CommunicationLog(this->authorId, authorId);
    cLog->addToLog(*log);

    this->logs.insert({logName, {authorId, cLog}});
    LogShell tmp = this->logs[logName].second->getLastEntry();

    this->connectedUser.push_back(authorId);

    Ptr<Packet> p = this->createPacket(
            new NetShell(
                ns3::Mac48Address::ConvertFrom(dev->GetAddress()),
                authorId,
                logName,
                0,
                &(tmp)
                    )
    );
    this->sendPacket(dev, p);
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
        << "i am " << this->authorId << endl;


    this->packetOss << "Received: " << netShell << endl;
    this->packetOss << "From: " << ns3::Mac48Address::ConvertFrom(dev->GetAddress()) << endl;
    this->packetOss << "Result: ";

    // Checks if it is the receiver
    if (nShell->type.find("/manager") != string::npos) {
        this->packetOss << "Dropping packet" << endl;

    } else if (nShell->type.find("/switch:") != string::npos) {
        if (this->processReceivedSwitchPacket(nShell, dev)) {
            auto lastEntry = this->logs[nShell->type].second->getLastEntry();
            //Dropping packet if this switch is not the receiver
            CommunicationLog* cLog;
            pair<std::string, std::string> receiver;
            switch (this->hash(lastEntry.shell->function)) {
                case ADD_MEMBER_TO_NETWORK:
                    this->addMemberToNetwork(lastEntry.shell->params);
                    break;
                case PLUG_AND_PLAY:this->sendPlugAndPlayConfirmation(dev, nShell->shell->authorId);
                    break;
                case GET_CONTENT_FROM:
                    // Somebody is interested in a log, so am I
                    receiver = SomeFunctions::varSplitter(nShell->receiverId, "/");
                    this->interestedNeighbours.insert({nShell->shell->shell->params, this->getKeyByValue(dev)});
                    nShell->type = this->authorId + "/" + nShell->shell->shell->params;
                    cLog = new CommunicationLog(this->authorId, nShell->shell->shell->params);
                    cLog->addToLog(*nShell->shell);
                    this->logs.insert({nShell->type, {nShell->shell->shell->params, cLog}});
                    this->broadcastToNeighbours(dev, nShell);
                    this->packetOss << "& and forwarding request";
                    break;
                case UNSUBSCRIBE_USER:
                    // handle unsubscription
                    this->removeUserFromInl(nShell->shell->authorId, nShell->shell->shell->params, nShell, dev);
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

void EthSwitch::forward(Ptr<NetDevice> dev, NetShell* nShell, uint8_t hops) {
    typedef multimap<string, std::string>::iterator MMAPIterator;
    auto p = SomeFunctions::varSplitter(nShell->type, "/");
    pair<MMAPIterator, MMAPIterator> result = this->interestedNeighbours.equal_range(p.first);
    for (MMAPIterator it = result.first; it != result.second; it++) {
        dev = this->neighbourMap[it->second];
        nShell->receiverId = it->second;
        auto p = this->createPacket(nShell);
        this->sendPacket(dev, p);
    }
}

bool EthSwitch::processReceivedSwitchPacket(NetShell *nShell, Ptr <NetDevice> dev) {
    // Check if receiver
    auto shellParam = nShell->shell->shell->params;
    if (this->isNeighbour(shellParam) && this->hash(nShell->shell->shell->function) == GET_CONTENT_FROM) {
        this->concatenateEntry(nShell);
        this->interestedNeighbours.insert({shellParam, nShell->shell->authorId});
        cout << "he is here" << endl;
        auto toSubscribe = LOGTYPE(shellParam, USER_ALL);
        cout << shellParam << endl;
        cout << "i am " << this->authorId << endl;
        cout << toSubscribe << endl;
        this->sendEntryFromIndexTo(this->getLogFrom(toSubscribe), this->getKeyByValue(dev), 0, toSubscribe);
        return false;
    }
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
    std::string& shellFunction = nShell->shell->shell->function;
    std::string& shellParam = nShell->shell->shell->params;
//    std::string& shellType = nShell->type;

    // Switches/Users need the content of a user
    if (this->hash(shellFunction) == GET_CONTENT_FROM) {
        auto p = SomeFunctions::varSplitter(nShell->type, "/");

        this->interestedNeighbours.insert({p.second, this->getKeyByValue(dev)});
        // Target is here
        if (this->isNeighbour(shellParam) || this->logs.find(LOGTYPE(shellParam, USER_ALL)) != this->logs.end()) {
            cout << "he is here" << endl;
            auto toSubscribe = LOGTYPE(shellParam, USER_ALL);
            cout << shellParam << endl;
            cout << "i am " << this->authorId << endl;
            cout << toSubscribe << endl;
            this->sendEntryFromIndexTo(this->getLogFrom(toSubscribe), this->getKeyByValue(dev), 0, toSubscribe);
        } else {
            // Somebody is interested in a log, so am I
            this->broadcastToNeighbours(dev, nShell);
            this->packetOss << "& and forwarding request";
        }


    } else if (this->hash(shellFunction) == UPDATE_CONTENT_FROM) { // User pushes content to switch
        if(this->concatenateEntry(nShell)) { // If new entry gets concatenated, then push to the subscribers
            cout << this->getKeyByValue(dev) << endl;
            if (!this->interestExists(nShell->shell->authorId, this->getKeyByValue(dev))) {
                this->interestedNeighbours.insert({nShell->shell->authorId, this->getKeyByValue(dev)});
            }
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
        auto neighbourId = entry.first;
        auto newReceiverDev = entry.second;
        auto logType = LOGTYPE(this->authorId, neighbourId);

        if (entry.second != dev && neighbourId != this->manager.first) {
            if(!this->isInCommunicationLog(logType)) {
                this->communicationLogs.insert({logType, new CommunicationLog(this->authorId, neighbourId)});
                this->communicationLogs[logType]->initialiseLog();
                LogShell tmp = this->communicationLogs[logType]->getLastEntry();
                LogShell* lShell_p = &tmp;
                NetShell* initNShell = new NetShell(ns3::Mac48Address::ConvertFrom(newReceiverDev->GetAddress()), neighbourId, logType, 0, lShell_p);
                auto p = this->createPacket(initNShell);
                this->sendPacket(newReceiverDev, p);
            }
            this->communicationLogs[logType]->appendLogShell(nShell->shell->shell);
            LogShell tmp = this->communicationLogs[logType]->getLastEntry();
            LogShell* lShell_p = &tmp;
            nShell = new NetShell(ns3::Mac48Address::ConvertFrom(newReceiverDev->GetAddress()), neighbourId, logType, 0, lShell_p);

            cout << "sending packets" << endl;

            auto p = this->createPacket(nShell);
            this->sendPacket(newReceiverDev, p);
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
void EthSwitch::removeUserFromInl(std::string canceller,
                                  std::string subscription,
                                  NetShell *nShell,
                                  Ptr<NetDevice> dev) {
    typedef multimap<std::string, std::string>::iterator iter;
    for (auto i : this->interestedNeighbours) {
    }
    cout << subscription << endl;
    std::pair<iter, iter> iterpair = this->interestedNeighbours.equal_range(subscription);
    iter it = iterpair.first;
    for (; it != iterpair.second; ++it) {
        if (it->second == canceller) {
            this->interestedNeighbours.erase(it);
            break;
        }
    }

    it = iterpair.first;
    for (auto neighbour : this->interestedNeighbours) {
        auto mmSubscription = neighbour.first;
        auto subscriber = neighbour.second;
        if (subscriber.find(USER_PREFIX) == std::string::npos) {
            this->removeSubscription(subscription);
            nShell->type = LOGTYPE(nShell->shell->shell->params, USER_ALL);

            if(this->forwardDeletion(nShell)) {
                this->interestedNeighbours.clear();
            }
            break;
        }
    }
}
bool EthSwitch::interestExists(std::string subscription, std::string subscriber) {
    typedef multimap<std::string, std::string>::iterator iter;
    std::pair<iter, iter> iterpair = this->interestedNeighbours.equal_range(subscription);
    iter it = iterpair.first;
    for (; it != iterpair.second; ++it) {
        auto sub = it->first;
        auto subscr = it->second;
        if (sub == subscription && subscr == subscriber) {
            return true;
        }
    }

    return false;
}
bool EthSwitch::forwardDeletion(NetShell *nShell) {

    auto cShell = nShell->shell->shell;
    bool b = true;
    for (auto p : this->interestedNeighbours) {
        auto subscriber = p.second;
        auto idk = p.first;
        if (idk == subscriber) {
            b = false;
            continue;
        }
        auto logType = LOGTYPE(this->authorId, subscriber);
        auto log = this->communicationLogs[logType];
        auto dev = this->neighbourMap[subscriber];
        log->appendLogShell(cShell);
        LogShell lShell = log->getLastEntry();
        LogShell* lShell_p = &lShell;
        nShell = new NetShell(ns3::Mac48Address::ConvertFrom(dev->GetAddress()), subscriber, logType, 0, lShell_p);
        auto packet = this->createPacket(nShell);
        this->sendPacket(dev, packet);
    }
    return b;
}