#ifndef MANAGER_H
#define MANAGER_H

#include "shells/LogStructures.h"
#include "NetworkDevice.h"
using namespace ns3;
using namespace std;

class Manager : public Application, public NetworkDevice {
private:
    int8_t currSeq = 0;
    string myType;
public:
    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt);
    void registerUser(Ptr<NetDevice> dev, std::string authorId);
    void sendNetworkJoinConfirmation(std::string authorId);
    void broadcastLastNetworkChange(std::string exceptedReceiver);
    bool processReceivedSwitchPacket(NetShell* netShell, Ptr<NetDevice> dev) override;
    void processReceivedUserPacket(NetShell* netShell, Ptr<NetDevice> dev) override;
    bool concatenateEntry(NetShell* netShell);

    Manager(std::pair<int, int> pq, double gossipInterval) : Application() {
        RsaSignature signature(pq.first, pq.second);
        auto pubKey = signature.generatePublicKey();
        auto privKey = signature.generatePrivateKey();

        this->authorId = MANAGER_PREFIX + to_string((int) pubKey);
        this->myType = this->authorId + "/switch:*";
        this->privateKey = privKey;
        this->myPersonalLog = new CommunicationLog(this->authorId, SWITCH_ALL, this->privateKey);
        this->myPersonalLog->initialiseLog();
        this->publicKey = pubKey;
    }
    virtual ~Manager() {}

    virtual void StartApplication(void) {
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
            GetNode()->RegisterProtocolHandler(
                    MakeCallback(&Manager::recvPkt, this),
                    0x800,
                    GetNode()->GetDevice(i)); //Register Event Handler to all Devices
        }
        Simulator::ScheduleDestroy(&Manager::printNetworkLog, this);
    }



};


#endif