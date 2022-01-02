#ifndef USER_H
#define USER_H

#include "CommunicationLog.h"
#include "shells/LogStructures.h"
#include "NetworkDevice.h"

using namespace ns3;
using namespace std;

class User : public Application, public NetworkDevice {

private:
    int count = 0;

public:
    void recvPkt(Ptr<NetDevice>, Ptr<const Packet>, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt);
    void joinNetwork();
    void subscribe(int8_t authorId);
    void plugAndPlay();
    void printNetworkLog() override;
    void pushLogToSwitch();
    bool processReceivedSwitchPacket(NetShell* netShell, Ptr<NetDevice> dev) override;
    void processReceivedUserPacket(NetShell* netShell, Ptr<NetDevice> dev) override;




    User(int32_t id, double errorRate) : Application() {
        this->authorId = id;
        this->myPersonalLog = new CommunicationLog(this->authorId);
        this->name = "user:" + to_string(this->authorId);
    }
    virtual ~User() {}

    virtual void StartApplication(void) {
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
            GetNode()->RegisterProtocolHandler(
                    MakeCallback(&User::recvPkt, this),
                    0x800,
                    GetNode()->GetDevice(i)); //Register Event Handler to all Devices
        }
        this->plugAndPlay();
    }


};
#endif
