#include "EthSwitch.h"
#include "shells/LogStructures.h"
#include "shells/LogFunctions.h"

using namespace ns3;
using namespace std;

void EthSwitch::requestJoiningNetwork() {
    ContentShell *cShell = new ContentShell("addToNetwork", "",
                                            to_string(this->authorId) + "wants to join the network");
    LogShell *logShell = new LogShell(-1, "", this->authorId, cShell);
    NetShell *nShell = new NetShell(ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 127, "request", logShell);

    // Broadcast that the user wants to join the network (simulating via uni-cast)

    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
        Ptr <Packet> p = this->createPacket(nShell);
        Ptr <NetDevice> dev = GetNode()->GetDevice(i);
        auto dest = ns3::Mac48Address::ConvertFrom(dev->GetAddress());
        cout << p << endl;
        cout << "Sending packets to " << dest << " with Packet size " << p->GetSize() << endl;

        if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
            std::cout << "Unable to send packet" << std::endl;
        }
    }
}
void EthSwitch::reconstructNetwork() {
    for (auto &entry : this->networkLog->getLog()) {
      this->reconstructLog(entry.shell);
    }

}

void EthSwitch::reconstructLog(ContentShell* cShell) {
    if (cShell->function == ADD_MEMBER) {
        this->addMemberToNetwork(cShell->params);
    }
}

void EthSwitch::assignManager(ns3::Mac48Address sender, int8_t managerId) {
    this->manager = make_pair(managerId, sender);
}

void EthSwitch::addMemberToNetwork(string params) {
    int authorId;
    stringstream ssAuthorId(params);
    ssAuthorId >> authorId;
    this->familyMembers.push_back(authorId);
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
        << "i am " << to_string(this->authorId) << endl
        << packet << endl;


    // Reads packet size and prepares the transformation of bytes to string
    string netShell = this->readPacket(packet);
    // Transforms the string to a net shell object
    NetShell* nShell = SomeFunctions::shell(netShell);
    oss << "Received: " << netShell << endl;
    oss << "Result: ";



    // Checks the type of of shell (communication or log exchange)

//    if (!this->isFamilyMember(nShell->shell->authorId)) {
//        oss << "Dropping packet. Not family member";
//        cout << oss.str() << endl;
//        return;
//
//    }

    if (this->neighbourMap.find(nShell->shell->authorId) == this->neighbourMap.end()) {
        cout << "Adding neighbour" << endl;
        this->neighbourMap.insert(make_pair(nShell->shell->authorId, dev));
    }

    if (nShell->type == REQUEST) {
        oss << "Dropping packet. Information for the manager";
        cout << oss.str() << endl;
        return;
    }

    // Checks if the device is the receiver
    if (nShell->receiverId == this->authorId) {
        oss << "This packet is for me. ";
        if (nShell->type == DIARY) {
            if (nShell->shell->shell->function == ASSIGN_MANAGER) {
                this->assignManager(ns3::Mac48Address::ConvertFrom(dev->GetAddress()),
                                    nShell->shell->authorId);
                oss << "Manager with ID " << to_string(this->manager.first) << " is assigned.";
            }
        } else if (nShell->type == LOG_ENTRY) {
            if (this->isSubSequentSeqNum(nShell->shell)) {
                this->networkLog->addToLog(*nShell->shell);
                this->reconstructLog(this->networkLog->getLastEntry().shell);
            } else {
                cout << "my seq " << to_string(this->networkLog->getCurrentSeqNum()) << endl;
                cout << "his seq" << to_string(nShell->shell->sequenceNum) << endl;
                oss << "not adding log... not same seq" << endl;
            }
        } else if (nShell->type == GOSSIP) {
            oss << "Gossip ->";
            if (!this->isMyNeighboursLogUpToDate(nShell->shell)) {
                oss << "Different state than my neighbour";
//                this->sendEntryFromIndexTo(this->getKeyByValue(dev), nShell->shell->sequenceNum + 1);
            } else {
                oss << "State up-to-date compared to the neighbour's. No changes.";
            }
        }
    }
    cout << oss.str() << endl;
    cout << "----------------SwitchPacket_END----------------\n" << endl;
}