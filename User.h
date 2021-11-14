#ifndef USER_H
#define USER_H

#include "Config.h"
using namespace ns3;
using namespace std;

class User : public Application {

private:
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedSwitches;
    int8_t nodeId;
    string communicationLog;
    string otherLog;
    mutex lock;

public:
    void recvPkt(Ptr<NetDevice>, Ptr<const Packet>, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt);
    void joinSwitch();
    void registerSwitch(Ptr<NetDevice> dev, ns3::Mac48Address switchMac, uint8_t switchId);
    PacketStream createBuf(Ptr<const Packet> packet);

    User(int32_t id) : Application() { this->nodeId = id; }
    virtual ~User() {}

    virtual void StartApplication(void) {
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
            GetNode()->RegisterProtocolHandler(
                    MakeCallback(&User::recvPkt, this),
                    0x800,
                    GetNode()->GetDevice(i)); //Register Event Handler to all Devices
        }
    }


};
#endif
