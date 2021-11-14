#ifndef ETHSWITCH_H
#define ETHSWITCH_H

#include "Config.h"
#include "CommunicationLog.h"
#include "UserLog.h"
#include <functional>

using namespace ns3;
using namespace std;



class EthSwitch : public Application {
private:
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedSwitches;
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> connectedUsers;
    map<int32_t, pair<ns3::Mac48Address, Ptr<NetDevice>>> tmpConnectedUsers;

    vector<pair<int8_t, ns3::Mac48Address>> log;

    vector<CommunicationLog> neighbourLogs;
    vector<UserLog> userLogs;

    CommunicationLog cLog; //
    int32_t seq = 0;

    pair<ns3::Mac48Address, Ptr<NetDevice>> manager;
    //TODO: Author id
    int8_t nodeId;
    mutex lock;

public:
    void recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt );
    bool registerUser(Ptr<NetDevice> dev, ns3::Mac48Address userMac, uint8_t deviceId);
    void sendConfirmJoin(Ptr<NetDevice> dev, ns3::Mac48Address sender, uint8_t msg);
    void joinManagersNetwork();
    void forwardUserJoinRequest(uint8_t userId, uint8_t msg);
    void addToLog(int8_t seq, ns3::Mac48Address mac);
    bool needEntries(int8_t managerSeq);
    void askForLogEntries();
    void gossip();
    void giveNeighbourLogs(Ptr<NetDevice> dev, ns3::Mac48Address sender, int8_t missingSeq);
    void sendLogEntries(Ptr<NetDevice> dev, ns3::Mac48Address sender, int8_t seq);
    void printLogEntries();
    void dejoin();
    void removeFromLog(int8_t seq, ns3::Mac48Address);



    //map<string, fncptr> functionMap;

    EthSwitch(int32_t id) : Application() { 
        this->nodeId = id; 
        this->cLog = CommunicationLog(0);
        this->cLog.initialiseLog(ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), this->nodeId);
        ++this->seq;
        this->printLogEntries();
        //this->functionMap.emplace("addSwitchToList", addSwitchToList);
        //this->functionMap.emplace("removeSwitchFromList", removeSwitchFromList);
        //this->functionMap.emplace("addUserToList", addUserToList)
        //this->functionMap.emplace("removeUserFromList", removeUserFromList)
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