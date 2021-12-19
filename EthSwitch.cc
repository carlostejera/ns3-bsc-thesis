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
    auto log = this->logs["manager100/switch*"].second;
    for (auto entry : this->neighbourMap) {
        LogShell tmp = log->getLastEntry();
        LogShell* p = &tmp;
        auto nShell = new NetShell(Mac48Address::ConvertFrom(entry.second->GetAddress()), entry.first, "manager100/switch*", 0, p);
        if (entry.second != this->manager.second) {
            auto p = this->createPacket(nShell);
            this->sendPacket(entry.second, p);
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
    ostringstream oss;
    oss << "------------------SwitchPacket-------------------" << endl
        << "i am " << to_string(this->authorId) << endl;


    // Reads packet size and prepares the transformation of bytes to string
    string netShell = this->readPacket(packet);
    // Transforms the string to a net shell object
    NetShell* nShell = SomeFunctions::shell(netShell);
    oss << "Received: " << netShell << endl;

    oss << "Result: ";
    // Records every sender to keep a map of the neighbours
    if (this->neighbourMap.find(this->getKeyByValue(dev)) == this->neighbourMap.end() && nShell->hops == 0) {
        oss << "Adding neighbour: " << to_string(nShell->shell->authorId);
        this->neighbourMap.insert(make_pair(nShell->shell->authorId, dev));
    }

    // Checks if it is the receiver
    cout << "here" << endl;
    if (nShell->type.find("/manager") != string::npos) {
        cout << "dropping packet" <<endl;
        return;
    } else if (nShell->type.find("/switch") != string::npos) {
        cout << "test" << endl;

        if (nShell->receiverId == this->authorId) {
            // Checks what kind of log it is -> If it is not known, a new log will be recorded, since the packet is addressed to this switch
            if (this->logExists(nShell)) {
                this->concatenateEntry(nShell);
                oss << "& concatenating entry " << to_string(nShell->shell->sequenceNum) << " in " << nShell->type << endl;
            } else {
                this->addLog(nShell);
                oss << "& adding new log " << nShell->type << endl;
                if (nShell->type.find("manager") != string::npos) {
                    this->manager = {nShell->shell->authorId, dev};
                    auto tmp = this->logs["switch" + to_string(this->authorId) + "/manager*"];
                    this->logs["switch" + to_string(this->authorId) + "/manager" + to_string(this->manager.first)] = tmp;
                    this->logs.erase("switch" + to_string(this->authorId) + "/manager*");
                    oss << "& assigning manager " << endl;
                }
            }
        } else {
            // For all switches
            this->addLog(nShell);
            oss << "& adding new log " << nShell->type << endl;
        }



    } else if (nShell->type.find("/user") != string::npos) {


        if (this->hash(nShell->shell->shell->function) == GET_CONTENT_FROM) {

            // TODO: Changer hard coded condition
            if (this->isNeighbour(this->convertStringToId(nShell->shell->shell->params)) || this->logs.find("user1/user*") != this->logs.end()) {
                cout << "he is here" << endl;
                this->sendEntryFromIndexTo(this->logs["user" + nShell->shell->shell->params + "/user*"].second, this->getKeyByValue(dev), 0, "user" + nShell->shell->shell->params + "/user*");
            } else {
//                nShell->shell->authorId = this->authorId;
                this->logs.insert({nShell->type, {this->convertStringToId(nShell->shell->shell->params), new CommunicationLog(this->authorId, nShell->shell)}});
                this->forward(dev, nShell, ++nShell->hops);
                oss << "& and forwarding request";
            }


        } else if (this->hash(nShell->shell->shell->function) == UPDATE_CONTENT_FROM) {
            cout << "*******" << endl;
            cout << "should be here" << endl;

            // User pushes to the connected switch
            if (nShell->type == "user" + to_string(nShell->shell->authorId) + "/user*" && this->isNeighbour(nShell->shell->authorId)) {
                if (this->logExists(nShell)) {
                    if (this->concatenateEntry(nShell)) {
                        oss << "& concatenating entry " << to_string(nShell->shell->sequenceNum) << " in "
                            << nShell->type << endl;
                    }
                } else {
                    this->addLog(nShell);
                    oss << "&& adding new log " << nShell->type << endl;
                }
            } else {
                if (this->logExists(nShell)) {
                    if (this->concatenateEntry(nShell)) {
                        oss << "&&& concatenating entry " << to_string(nShell->shell->sequenceNum) << " in " << nShell->type << endl;
                        this->forward(dev, nShell, ++nShell->hops);
                    }

                } else {
                    this->addLog(nShell);
                    oss << "&&&& adding new log " << nShell->type << endl;
                    this->forward(dev, nShell, ++nShell->hops);

                }
                cout << "and heeeeere" << endl;
            }

        }
        cout << oss.str() << endl;
        return;

    } else {
        return;
    }

    /*if (nShell->receiverId == this->authorId) {
        // Checks what kind of log it is -> If it is not known, a new log will be recorded, since the packet is addressed to this switch
        if (this->logExists(nShell)) {
            this->concatenateEntry(nShell);
            oss << "& concatenating entry " << to_string(nShell->shell->sequenceNum) << " in " << nShell->type << endl;
        } else {
            this->addLog(nShell);
            oss << "& adding new log " << nShell->type << endl;
            if (nShell->type.find("manager") != string::npos) {
                this->manager = {nShell->shell->authorId, dev};
                auto tmp = this->logs["switch" + to_string(this->authorId) + "/manager*"];
                this->logs["switch" + to_string(this->authorId) + "/manager" + to_string(this->manager.first)] = tmp;
                this->logs.erase("switch" + to_string(this->authorId) + "/manager*");
                oss << "& assigning manager " << endl;
            }
        }
    } else if (nShell->receiverId == 127 && nShell->type.find("switch*") != string::npos && nShell->type.find("user") != string::npos) {
        this->addLog(nShell);
        oss << "& adding new log " << nShell->type << endl;
    } else if (nShell->type.find("/user") != string::npos) {
        cout << "-------> " << nShell->type << endl;
        cout << "-------> " << "switch" + to_string(this->authorId) + "/user" + nShell->shell->shell->params << endl;
        if (this->hash(nShell->shell->shell->function) == GET_CONTENT_FROM) {
            if (this->isNeighbour(this->convertStringToId(nShell->shell->shell->params))) {
                cout << "he is here" << endl;
                this->sendEntryFromIndexTo(this->logs["user" + nShell->shell->shell->params + "/user*"].second, this->getKeyByValue(dev), 0, "user" + nShell->shell->shell->params + "/user*");
            } else {
                nShell->shell->authorId = this->authorId;
                this->logs.insert({"switch" + to_string(this->authorId) + "/user" + nShell->shell->shell->params, {this->convertStringToId(nShell->shell->shell->params), new CommunicationLog(this->authorId, nShell->shell)}});
                this->forward(dev, nShell->shell, ++nShell->hops);
                oss << "& and forwarding request";
            }
        }

        else if (this->logs.find("switch" + to_string(this->authorId) + "/user" + nShell->shell->shell->params) != this->logs.end()) {
            cout << "do <ou exist" << endl;
            if (this->logExists(nShell)) {
                this->concatenateEntry(nShell);
                oss << "& concatenating entry " << to_string(nShell->shell->sequenceNum) << " in " << nShell->type << endl;
            } else {
                this->addLog(nShell);
                oss << "&&&& adding new log " << nShell->type << endl;
            }
            this->forward(dev, nShell->shell, ++nShell->hops);
        }
        cout << oss.str() << endl;
        return;
    } else {
        return;
    }*/




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
    oss << "----------------SwitchPacket_END----------------\n" << endl;
    if (VERBOSE) {
        cout << oss.str() << endl;
    }

}

bool EthSwitch::isInList(vector<int8_t> v, int8_t authorId) {
    return find(v.begin(), v.end(), authorId) != v.end();
}

void EthSwitch::forward(Ptr<NetDevice> dev, NetShell* nShell, uint8_t hops) {
    for (auto entry : this->neighbourMap) {
        cout << "sending packets" << endl;
        if (entry.second != dev && entry.second != this->manager.second) {
            auto p = this->createPacket(nShell);
            this->sendPacket(entry.second, p);
        }
    }
}