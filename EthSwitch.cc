#include "EthSwitch.h"
#include "shells/LogFunctions.h"
#include "GlobalsValues.h"


using namespace ns3;
using namespace std;
/**
 * Switch tries to join the network by asking for the manager
 */

void EthSwitch::requestJoiningNetwork() {
    CommunicationLog* log = new CommunicationLog(this->authorId, MANAGER_ALL, this->privateKey);

    ContentShell *cShell = new ContentShell(ADD_USER_TO_NETWORK,this->authorId,this->authorId + " wants to join the network");
    log->appendLogShell(cShell);
    LogShell lShell = log->getLastEntry();
    LogShell* lShell_p = &lShell;
    string commLog = LOGTYPE(this->authorId, MANAGER_ALL);
    NetShell *nShell = new NetShell(MANAGER_ALL, commLog, 0, 0, lShell_p);
    this->logPacket.add(LogPacket(commLog, log, CommunicationType::P2P_COMM));
//    this->communicationLogs.insert({commLog, log});
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
    string logName = this->authorId + "/" + authorId;
    CommunicationLog* cLog = new CommunicationLog(this->authorId, authorId, this->privateKey);
    cLog->appendLogShell(new ContentShell("addSwitch",this->authorId,"Add switch to the connected list"));
    this->logPacket.add(LogPacket(logName, cLog, CommunicationType::P2P_COMM));
    LogShell tmp = this->logPacket.getLogByWriterReader(logName)->getLastEntry();

    this->connectedUser.push_back(authorId);

    Ptr<Packet> p = this->createPacket(
        new NetShell(
            authorId,
            logName,
            0,
            0,
            &(tmp)
        )
    );
    this->sendPacket(dev, p);
}

void EthSwitch::gossip() {

    // Choose a random log to gossip about
    LogList subscriptionLogList = this->logPacket.getLogPacketsWithType(CommunicationType::SUBSCRIPTION);
    auto number =  rand() % subscriptionLogList.size();
    auto logPacket = subscriptionLogList.getLogPacketAt(number);
    auto randomLogType = logPacket.getWriterReader();
    auto log = logPacket.getLog();
    NetShell* nShell;

    for (auto entry : this->neighbourMap) {
        if (log->getLog().empty()) { // If the chosen log is empty, tell that your neighbours with a fake log entry to get help
            auto cShell = new ContentShell("f", "p", "I have no content");
            auto lShell = new LogShell(to_string(Simulator::Now().GetSeconds()), -1, "", this->manager.first, "", cShell);
            nShell = new NetShell(entry.first, randomLogType, 1, 0, lShell);
//            if (entry.first != this->manager.first) {
                auto packet = this->createPacket(nShell);
                this->sendPacket(entry.second, packet);
//            }
        } else {
            LogShell tmp = log->getLastEntry();
            LogShell* p = &tmp;
            nShell = new NetShell(entry.first, randomLogType, 1, 0, p);
//            if (entry.first != this->manager.first) {
                auto packet = this->createPacket(nShell);
                this->sendPacket(entry.second, packet);
//            }
        }
    }
    // This will be triggered <gossipInterval> seconds later
    Simulator::Schedule(Seconds(this->gossipInterval), &EthSwitch::gossip, this);
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
    if (true) {
        if (nShell->type.find("/manager") != string::npos) {
            this->packetOss << "Dropping packet" << endl;

        } else if (nShell->type.find("/switch:") != string::npos) {
            // Check if it is the manager and if it is not already assigned
            if (nShell->type.find(MANAGER_PREFIX) != string::npos && !this->isManagerAssigned) {
                auto p = SomeFunctions::varSplitter(nShell->type, "/");
                this->manager = {p.first, dev};
                this->isManagerAssigned = true;
                Simulator::Schedule(Seconds(this->gossipInterval), &EthSwitch::gossip, this);
            }
            if (this->processReceivedSwitchPacket(nShell, dev)) {
                auto lastEntry = this->logPacket.getLogByWriterReader(nShell->type)->getLastEntry();
                //Dropping packet if this switch is not the receiver
//                CommunicationLog *cLog;
                switch (this->hash(lastEntry.shell->function)) {
                    case ADD_MEMBER_TO_NETWORK:this->addMemberToNetwork(lastEntry.shell->params);
                        break;
                    case PLUG_AND_PLAY:this->sendPlugAndPlayConfirmation(dev, nShell->shell->authorId);
                        break;
                    case GET_CONTENT_FROM:
                        // Somebody is interested in a log, so am I
                        this->interestedNeighbours.insert({nShell->shell->shell->params, this->getKeyByValue(dev)});
                        this->broadcastToNeighbours(dev, nShell);
                        this->packetOss << "& and forwarding request";
                        break;
                    case UNSUBSCRIBE_USER:
                        // handle unsubscription
                        this->removeUserFromInl(nShell->shell->authorId, nShell->shell->shell->params, nShell, dev);
                        break;
                    default:break;
                }
            } else {
            }

        } else if (nShell->type.find("/user") != string::npos) {
            this->processReceivedUserPacket(nShell, dev);

        } else {
            this->packetOss << "Dropping unknown packet";
        }
    } else {
        // Gossip answer
/*        if (this->subscriptionExists(nShell->type) && this->isGossipEntryOlder(nShell)) {
            this->sendEntryFromIndexTo(this->getLogFrom(nShell->type), this->getKeyByValue(dev),
                                       nShell->shell->sequenceNum, nShell->type);
            this->packetOss << "Answering to gossip" << endl;
        }*/
    }
    this->packetOss << "----------------SwitchPacket_END----------------\n" << endl;
    if (VERBOSE) {
        if(nShell->flag == 1) {
            this->printBlack(this->packetOss.str());
        } else {
            this->printPacketResult();
        }
    }
    this->packetOss.str("");
}

void EthSwitch::forward(Ptr<NetDevice> dev, NetShell* nShell, uint8_t hops) {
    typedef multimap<string, std::string>::iterator MMAPIterator;
    auto p = SomeFunctions::varSplitter(nShell->type, "/");
    auto subscribed = p.first;
    // Check which is the subscription devices are interested in
    pair<MMAPIterator, MMAPIterator> result = this->interestedNeighbours.equal_range(subscribed);
    Ptr<NetDevice> newDev;
    for (MMAPIterator it = result.first; it != result.second; it++) {
        auto check = it->first;
        auto check2 = it->second;
        if (it->first == it->second) continue;
        newDev = this->neighbourMap[it->second];
        if (newDev == dev) continue;
//        nShell->receiverId = it->second;
        auto p = this->createPacket(nShell);
        this->sendPacket(newDev, p);
    }
}

bool EthSwitch::processReceivedSwitchPacket(NetShell *nShell, Ptr <NetDevice> dev) {
    // Check if receiver
    auto shellParam = nShell->shell->shell->params;
    if ((this->isNeighbour(shellParam) && this->hash(nShell->shell->shell->function) == GET_CONTENT_FROM)
    || (this->logPacket.exists(LOGTYPE(shellParam, USER_ALL)))
    ) {
        this->concatenateEntry(nShell);
        this->interestedNeighbours.insert({shellParam, nShell->shell->authorId});
        auto toSubscribe = LOGTYPE(shellParam, USER_ALL);
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
    if (nShell->type.find("manager") != string::npos && nShell->type.find("/" + this->authorId) != string::npos && !this->isManagerAssigned) {
        this->assignManager(dev, nShell->shell->authorId);
        Simulator::Schedule(Seconds(this->gossipInterval), &EthSwitch::gossip, this);
    }
    return this->concatenateEntry(nShell);
}

void EthSwitch::processReceivedUserPacket(NetShell *nShell, Ptr <NetDevice> dev) {
    std::string& shellFunction = nShell->shell->shell->function;
    std::string& shellParam = nShell->shell->shell->params;
//    std::string& shellType = nShell->type;

    // Switches/Users need the content of a user
    if (nShell->flag == 0) {
        if (this->hash(shellFunction) == GET_CONTENT_FROM) {
            auto p = SomeFunctions::varSplitter(nShell->type, "/");

            this->interestedNeighbours.insert({p.second, this->getKeyByValue(dev)});
            // Target is here
            if (this->isNeighbour(shellParam) || this->logPacket.exists(LOGTYPE(shellParam, USER_ALL))) {
                auto toSubscribe = LOGTYPE(shellParam, USER_ALL);
                this->sendEntryFromIndexTo(this->getLogFrom(toSubscribe), this->getKeyByValue(dev), 0, toSubscribe);
            } else {
                // Somebody is interested in a log, so am I
                this->broadcastToNeighbours(dev, nShell);
                this->packetOss << "& and forwarding request";
            }

        } else if (this->hash(shellFunction) == UPDATE_CONTENT_FROM) { // User pushes content to switch
            if (this->concatenateEntry(nShell)) { // If new entry gets concatenated, then push to the subscribers

                // If no interest exist yet, create one for the sender (dev)
                if (!this->interestExists(nShell->shell->authorId, this->getKeyByValue(dev))) {
                    this->interestedNeighbours.insert({nShell->shell->authorId, this->getKeyByValue(dev)});
                }
                this->forward(dev, nShell, ++nShell->hops);
            }
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
        if (newReceiverDev != dev && neighbourId != this->manager.first) {
            if(!this->logPacket.exists(logType)) {
                auto cLog = new CommunicationLog(this->authorId, neighbourId, this->privateKey);
                cLog->initialiseLog();
                this->logPacket.add(LogPacket(logType, cLog, CommunicationType::SWITCH_SWITCH_COMM));
//                this->communicationLogs.insert({logType, cLog});
//                this->communicationLogs[logType]->initialiseLog();
                LogShell tmp = this->logPacket.getLogByWriterReader(logType)->getLastEntry();
                LogShell* lShell_p = &tmp;
                NetShell* initNShell = new NetShell(neighbourId, logType, 0, 0, lShell_p);
                auto p = this->createPacket(initNShell);
                this->sendPacket(newReceiverDev, p);
            }
            this->logPacket.getLogByWriterReader(logType)->appendLogShell(nShell->shell->shell);
            LogShell tmp = this->logPacket.getLogByWriterReader(logType)->getLastEntry();
            LogShell* lShell_p = &tmp;
            nShell = new NetShell(neighbourId, logType, 0, 0, lShell_p);


            auto p = this->createPacket(nShell);
            this->sendPacket(newReceiverDev, p);
        }
    }
}

CommunicationLog *EthSwitch::getLogFrom(string type) {

    return this->logPacket.getLogByWriterReader(type);
}
void EthSwitch::removeUserFromInl(std::string canceller,
                                  std::string subscription,
                                  NetShell *nShell,
                                  Ptr<NetDevice> dev) {
    typedef multimap<std::string, std::string>::iterator iter;
    for (auto i : this->interestedNeighbours) {
    }
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
        auto log = this->logPacket.getLogByWriterReader(logType);
        auto dev = this->neighbourMap[subscriber];
        log->appendLogShell(cShell);
        LogShell lShell = log->getLastEntry();
        LogShell* lShell_p = &lShell;
        nShell = new NetShell(subscriber, logType, 0, 0, lShell_p);
        auto packet = this->createPacket(nShell);
        this->sendPacket(dev, packet);
    }
    return b;
}