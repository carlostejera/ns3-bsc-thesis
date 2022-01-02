#include "User.h"

using namespace ns3;
using namespace std;

void
User::recvPkt(Ptr <NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address &from, const Address &to,
              NetDevice::PacketType pt) {
    ostringstream oss;
    oss << "------------------UserPacket-------------------" << endl
        << "i am " << to_string(this->authorId) << endl;

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
                this->connectedSwitches.insert(make_pair(nShell->shell->authorId, dev));
                break;
            default:
                cout << "choking" << endl;
                int8_t senderId;

                if(nShell->type != "user" + to_string(this->authorId) + "/user*") {
                    return;
                }

                for (auto entry : this->connectedSwitches) {
                    if (entry.second == dev) {
                        senderId = entry.first;
                    }
                }
                cout << to_string(senderId) << endl;
                this->sendEntryFromIndexTo(this->userLog, senderId, nShell->shell->sequenceNum, "user" +
                        to_string(this->authorId) + "/user*");
                break;
        }
    }



    cout << oss.str() << endl;
}

void User::joinNetwork() {}

void User::subscribe(int8_t authorId) {
    for (auto item : this->connectedSwitches) {
        auto mac = ns3::Mac48Address::ConvertFrom(item.second->GetAddress());

        auto cShell = new ContentShell("getContentFrom", to_string(authorId),
                                       "Subscribe the author " + to_string(authorId));
        auto lShell = new LogShell(0, "", this->authorId, cShell);
        this->logs.insert({"user" + to_string(this->authorId) + "/user" + to_string(authorId),{authorId, new CommunicationLog(this->authorId, authorId)}});
        auto nShell = new NetShell(mac, 1, "user:" + to_string(this->authorId) + "/user:" + to_string(authorId), 0, lShell);
        auto p = this->createPacket(nShell);
        this->sendPacket(item.second, p);
    }
    this->interested = authorId;
    cout << "I am subscribing" << endl;
}

void User::plugAndPlay() {
    ContentShell *cShell = new ContentShell("plugAndPlay",
                                            to_string(this->authorId),
                                            to_string(this->authorId) + " plug and play"
    );
    LogShell *logShell = new LogShell(0, "", this->authorId, cShell);
    string logName = "user:" + to_string(this->authorId) + "/switch:*";
    NetShell *nShell = new NetShell(
            ns3::Mac48Address("FF:FF:FF:FF:FF:FF"),
            127,
            logName,
            0,
            logShell);
    this->logs.insert({logName, {127, new CommunicationLog(this->authorId)}});
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

    oss << "-------User " << to_string(this->authorId) << "-------" << endl;
    oss << "My Log: " << endl;
    for (auto lShell : this->userLog->getLog()) {
        stringAssembler.visit(&lShell);
        oss << stringAssembler.str() << endl;
        stringAssembler.clearOss();
    }
    oss << "Connected Switches: " << endl;
    for (auto item : this->connectedSwitches) {
        oss << to_string(item.first) << endl;
    }
    oss << "Logs:" << endl;
    for (auto item: this->subscriptions) {
        oss << "Log of " << to_string(item.first) << endl;
        for (auto lShell: this->subscriptions[item.first]->getLog()) {
            stringAssembler.visit(&lShell);
            oss << stringAssembler.str() << endl;
            stringAssembler.clearOss();
        }
    }
    oss << "-----------------------" << endl;
    cout << oss.str() << endl;
}


void User::pushLogToSwitch() {
    this->userLog->addToLog(LogShell(this->count, this->userLog->getLog().empty() ? "" : this->userLog->createHash(this->userLog->getLastEntry()), this->authorId, new ContentShell("pushContent", "", "This is my Computer log entry" +
            to_string(this->count))));
    this->count += 1;
    LogShell lShell = this->userLog->getLastEntry();
    LogShell *shell_p = &lShell;

    for (auto entry: this->connectedSwitches) {
        NetShell *netShell = new NetShell(ns3::Mac48Address::ConvertFrom(entry.second->GetAddress()), entry.first,
                                          "user:" + to_string(this->authorId) + "/user:*", 0, shell_p);
        auto p = this->createPacket(netShell);
        this->sendPacket(entry.second, p);
    }

}

bool User::processReceivedSwitchPacket(NetShell *netShell, Ptr <NetDevice> dev) {
    return true;

}

void User::processReceivedUserPacket(NetShell *netShell, Ptr <NetDevice> dev) {

}
