#ifndef ETHSWITCH_H
#define ETHSWITCH_H

#include "shells/LogStructures.h"
#include "NetworkDevice.h"
using namespace ns3;
using namespace std;



struct EthSwitch : public Application, public NetworkDevice {
    pair<int8_t, Ptr<NetDevice>> manager;
    bool isManagerAssigned = false;
    vector<int8_t> connectedUser;
    multimap<string, int8_t> interestedNeighbours;

    void addMemberToNetwork(string params);
    void assignManager(Ptr<NetDevice>, int8_t);
    void broadcastToNeighbours(Ptr<NetDevice> dev, NetShell* nShell);
    void forward(Ptr<NetDevice>, NetShell*, uint8_t hops);
    CommunicationLog* getLogFrom(string type);
    void gossip();
    void printNetworkLog() override;
    bool processReceivedSwitchPacket(NetShell* nShell, Ptr<NetDevice> dev) override;
    void processReceivedUserPacket(NetShell* nShell, Ptr<NetDevice> dev) override;
    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt );
    void requestJoiningNetwork();
    void sendPlugAndPlayConfirmation(Ptr<NetDevice>, int8_t);

    EthSwitch(int8_t authorId, double errorRate) {
        this->authorId = authorId;
        this->name = "switch:" + to_string(this->authorId);
    }
    virtual ~EthSwitch() {}
    virtual void StartApplication(void) {
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
            GetNode()->RegisterProtocolHandler(
                    MakeCallback(&EthSwitch::recvPkt, this),
                    0x800,
                    GetNode()->GetDevice(i)); //Register Event Handler to all Devices
        }
        this->requestJoiningNetwork();
        Simulator::ScheduleDestroy(&EthSwitch::printNetworkLog, this);
    }

};


#endif