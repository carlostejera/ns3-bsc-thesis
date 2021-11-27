#ifndef USER_H
#define USER_H

#include "CommunicationLog.h"
#include "shells/LogStructures.h"
using namespace ns3;
using namespace std;

class User : public Application {

private:
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedSwitches;
    int8_t nodeId;
    string communicationLog;
    string otherLog;

public:
    void recvPkt(Ptr<NetDevice>, Ptr<const Packet>, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt);

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
