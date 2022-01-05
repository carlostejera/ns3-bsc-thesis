#ifndef ETHSWITCH_H
#define ETHSWITCH_H

#include "shells/LogStructures.h"
#include "NetworkDevice.h"
using namespace ns3;
using namespace std;



struct EthSwitch : public Application, public NetworkDevice {
    pair<std::string, Ptr<NetDevice>> manager;
    bool isManagerAssigned = false;
    vector<std::string> connectedUser;
    multimap<std::string, std::string> interestedNeighbours;

    void addMemberToNetwork(string params);
    void assignManager(Ptr<NetDevice> dev, std::string manager);
    void broadcastToNeighbours(Ptr<NetDevice> dev, NetShell* nShell);
    void forward(Ptr<NetDevice>, NetShell*, uint8_t hops);
    CommunicationLog* getLogFrom(string type);
    void gossip();
    void printNetworkLog() override;
    bool processReceivedSwitchPacket(NetShell* nShell, Ptr<NetDevice> dev) override;
    void processReceivedUserPacket(NetShell* nShell, Ptr<NetDevice> dev) override;
    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt );
    void requestJoiningNetwork();
    void sendPlugAndPlayConfirmation(Ptr<NetDevice> dev, std::string authorId);
    void removeUserFromInl(std::string canceller,
                           std::string subscription,
                           NetShell *nShell,
                           Ptr<NetDevice> dev);
    bool interestExists(std::string subscription, std::string subscriber);
    bool forwardDeletion(NetShell *nShell);
    double gossipInterval;

    EthSwitch(std::string authorId, double gossipInterval) {
        this->authorId = SWITCH_PREFIX + authorId;
        this->gossipInterval = gossipInterval;
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