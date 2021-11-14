#ifndef MANAGER_H
#define MANAGER_H

#include "scratch/network/Config.h"
using namespace ns3;
using namespace std;

class Manager : public Application {
private:
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedSwitches;
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedUsers;
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedManagers;
    int8_t nodeId;
    vector<pair<int8_t, ns3::Mac48Address>> managerLog;
    pair<ns3::Mac48Address, Ptr<NetDevice>> tmpSwitchEntry;
    int8_t seq = 0;
    int testCount = 0;

public:
    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt);
    bool registerSwitch(Ptr<NetDevice> dev, ns3::Mac48Address switchMac, uint8_t switchId);
    void confirmSwitchJoin(Ptr<NetDevice> dev, ns3::Mac48Address sender);
    void confirmUserJoin(Ptr<NetDevice> dev, ns3::Mac48Address userMac, uint8_t userId);
    bool registerUser(Ptr<NetDevice> dev, ns3::Mac48Address userMac, uint8_t userId);
    void broadcastNetworkChanges(uint8_t flag);
    void flood(PacketStream stream, ns3::Address sender, ns3::Address target);
    void connectToManager();
    void sendLogEntries(Ptr<NetDevice> dev,uint8_t msg, ns3::Mac48Address sender, int8_t seq);
    bool switchExists(uint8_t id);
    void removeSwitch(uint8_t id);


    Manager(int32_t id) : Application() { this->nodeId = id; }
    virtual ~Manager() {}

    virtual void StartApplication(void) {
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
            GetNode()->RegisterProtocolHandler(
                    MakeCallback(&Manager::recvPkt, this),
                    0x800,
                    GetNode()->GetDevice(i)); //Register Event Handler to all Devices
        }
    }



};


#endif