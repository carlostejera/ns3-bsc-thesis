#ifndef MANAGER_H
#define MANAGER_H

#include "shells/LogStructures.h"
#include "NetworkDevice.h"
using namespace ns3;
using namespace std;

class Manager : public Application, public NetworkDevice {
private:
    int8_t currSeq = 0;
public:
    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt);
    void registerUser(Ptr<NetDevice> dev, int8_t authorId);
    void sendNetworkJoinConfirmation(int8_t authorId);
    void sendAllLogEntriesTo(int8_t authorReceiverId);
    void broadcastLastNetworkChange(int8_t exceptedReceiver);
    virtual void reconstructLog(ContentShell* cShell) override;


    Manager(int32_t id) : Application() { this->authorId = id;
    this->networkLog = new CommunicationLog(this->authorId);
    }
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