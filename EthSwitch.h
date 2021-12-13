#ifndef ETHSWITCH_H
#define ETHSWITCH_H

#include "shells/LogStructures.h"
#include "NetworkDevice.h"
using namespace ns3;
using namespace std;



struct EthSwitch : public Application, public NetworkDevice {
    pair<int8_t, Ptr<NetDevice>> manager;
    vector<int8_t> connectedUser;
    map<int8_t, Ptr<NetDevice>> pit;
    map<int8_t, CommunicationLog*> logsOfUsers;

    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt );
    void requestJoiningNetwork();
    void assignManager(Ptr<NetDevice>, int8_t);
    void addMemberToNetwork(string params);
    void printNetworkLog() override;
    void sendPlugAndPlayConfirmation(Ptr<NetDevice>, int8_t);
    bool isInList(vector <int8_t> v, int8_t authorId);
    void forward(Ptr<NetDevice>, NetShell*, uint8_t hops);



    EthSwitch(int8_t authorId) {
        this->authorId = authorId;
        this->networkLog = new CommunicationLog(authorId);
    }
    virtual ~EthSwitch() {}
    virtual void StartApplication(void) {
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
            GetNode()->RegisterProtocolHandler(
                    MakeCallback(&EthSwitch::recvPkt, this),
                    0x800,
                    GetNode()->GetDevice(i)); //Register Event Handler to all Devices
        }
    }

};


#endif