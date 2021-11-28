#ifndef NS_3_30_NETWORKDEVICE_H
#define NS_3_30_NETWORKDEVICE_H

#include "shells/LogStructures.h"
#include "CommunicationLog.h"

using namespace ns3;
using namespace std;


class NetworkDevice {
protected:

    vector<int8_t> familyMembers;
    std::map<int8_t, Ptr<NetDevice>> neighbourMap;
    CommunicationLog* networkLog;
    int8_t authorId;

    string readPacket(Ptr<const Packet> packet);
    Ptr<Packet> createPacket(NetShell* nShell);
    void sendPacket(Ptr<NetDevice> nDev, Ptr<Packet> p);
    bool isFamilyMember(int8_t authorId);
    void gossip();
    virtual void reconstructLog(ContentShell* cShell) = 0;
    void sendLastEntryTo(int8_t authorId);

public:
    void printNetworkLog();



};


#endif //NS_3_30_NETWORKDEVICE_H
