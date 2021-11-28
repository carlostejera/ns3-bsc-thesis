#ifndef ETHSWITCH_H
#define ETHSWITCH_H

#include "shells/LogStructures.h"
#include "NetworkDevice.h"
using namespace ns3;
using namespace std;



struct EthSwitch : public Application, public NetworkDevice {
    CommunicationLog* diary;
    pair<int8_t, ns3::Mac48Address> manager;

    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt );
    void requestJoiningNetwork();
    void assignManager(ns3::Mac48Address, int8_t);
    void addMemberToNetwork(string params);
    void reconstructLog(ContentShell* cShell) override;
    void reconstructNetwork();



    EthSwitch(int8_t authorId) {
        this->authorId = authorId;
        this->networkLog = new CommunicationLog(authorId);
        this->diary = new CommunicationLog(authorId);
//        this->networkLog->initialiseLog();
//        this->diary->initialiseLog();
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