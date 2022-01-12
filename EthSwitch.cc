#include "EthSwitch.h"
#include "shells/LogFunctions.h"
#include "GlobalsValues.h"


using namespace ns3;
using namespace std;
/**
 * To be added by the manager, the switch creates a new log where the manager is addressed. The newest logShell gets
 * extracted and sent to the manager via flooding
 */

void EthSwitch::requestJoiningNetwork() {
    CommunicationLog* log = new CommunicationLog(this->authorId, MANAGER_ALL, this->privateKey);
    ContentShell *cShell = new ContentShell(ADD_SWITCH_TO_NETWORK, this->authorId, this->authorId + " wants to join the network");

    log->appendLogShell(cShell);
    LogShell lShell = log->getLastEntry();
    LogShell* lShell_p = &lShell;

    string commLog = LOGTYPE(this->authorId, MANAGER_ALL);
    NetShell *nShell = new NetShell(MANAGER_ALL, commLog, 0, 0, lShell_p);
    this->logPacket.add(LogPacket(commLog, log, CommunicationType::P2P_COMM));

    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
        Ptr <Packet> p = this->createPacket(nShell);
        Ptr <NetDevice> dev = GetNode()->GetDevice(i);
        if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }
    }
}

/**
 * Assigning the manager in a std::pair
 * @param dev the mac of the manager
 * @param manager the id
 */
void EthSwitch::assignManager(Ptr<NetDevice> dev, std::string manager) {
    this->manager = {manager, dev};
    this->packetOss << "& assigning manager " << endl;
    this->isManagerAssigned = true;
}

/**
 * Function that can be requested by the manager. It adds new arriving switches
 * @param params id of arriving switch
 */
void EthSwitch::addMemberToNetwork(string params) {
    std::string  authorId = params;
    this->familyMembers.push_back(authorId);
}

/**
 * Final print out to
 */
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
 * @param dev The "port" where the device has to answer
 * @param authorId id of the recipient
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


/**
 * Gets a list from the subscriptions and chooses a log randomly. Logs can have been initialised, but being empty.
 * If it is empty, it simulates an entry in order to receive the updated, if there are.
 */
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
                auto packet = this->createPacket(nShell);
                this->sendPacket(entry.second, packet);
        } else {
            LogShell tmp = log->getLastEntry();
            LogShell* p = &tmp;
            nShell = new NetShell(entry.first, randomLogType, 1, 0, p);
                auto packet = this->createPacket(nShell);
                this->sendPacket(entry.second, packet);
        }
    }
    // This will be triggered <gossipInterval> seconds later
    Simulator::Schedule(Seconds(this->gossipInterval), &EthSwitch::gossip, this);
}


/**
 * Handling the receiving packets
 * @param dev device MAC address
 * @param packet the netshell
 * @param proto -
 * @param from -
 * @param to -
 * @param pt -
 */
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
    NetShell* nShell = Parser::shell(netShell);



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
        // Check if it is the manager and if it is not already assigned
        if (nShell->type.find(MANAGER_PREFIX) != string::npos && !this->isManagerAssigned) {
            auto p = Parser::varSplitter(nShell->type, "/");
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

/**
 * Forwards the netshells to the interested neighbours
 * @param dev dev not to send to
 * @param nShell nShell to send
 */

void EthSwitch::forward(Ptr<NetDevice> dev, NetShell *nShell) {
    typedef multimap<string, std::string>::iterator MMAPIterator;
    auto p = Parser::varSplitter(nShell->type, "/");
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
/**
 * Process incoming switch packets
 * @param nShell
 * @param dev
 * @return
 */
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

/**
 * Process packets from user
 * @param nShell
 * @param dev
 */
void EthSwitch::processReceivedUserPacket(NetShell *nShell, Ptr <NetDevice> dev) {
    std::string& shellFunction = nShell->shell->shell->function;
    std::string& shellParam = nShell->shell->shell->params;
//    std::string& shellType = nShell->type;

    // Switches/Users need the content of a user
    if (nShell->flag == 0) {
        if (this->hash(shellFunction) == GET_CONTENT_FROM) {
            auto p = Parser::varSplitter(nShell->type, "/");

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
                this->forward(dev, nShell);
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
/**
 * Floods packets to the neighbours where also new logs are created which are dedicated to them
 * @param dev
 * @param nShell
 */
void EthSwitch::broadcastToNeighbours(Ptr <NetDevice> dev, NetShell *nShell) {
    for (auto entry : this->neighbourMap) {
/*        // Not broadcasting to user
        if (std::find(this->familyMembers.begin(), this->familyMembers.end(), entry.first) == this->familyMembers.end()){
            continue;
        }*/
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
            auto newNShell = new NetShell(neighbourId, logType, 0, 0, lShell_p);


            auto p = this->createPacket(newNShell);
            this->sendPacket(newReceiverDev, p);
        }
    }
}

CommunicationLog *EthSwitch::getLogFrom(string type) {

    return this->logPacket.getLogByWriterReader(type);
}

/**
 * Remove user from interested list. If no user is anymore interested, also the interests of the switches are deleted,
 * since there is no need.
 * @param canceller
 * @param subscription
 * @param nShell
 * @param dev
 */
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

/**
 * Check if an interest exists
 * @param subscription
 * @param subscriber
 * @return
 */
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

/**
 * Forward function for the deletion
 * @param nShell
 * @return
 */
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