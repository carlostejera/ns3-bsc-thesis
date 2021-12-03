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
    virtual void reconstructLog(ContentShell* cShell) = 0;
    void sendLastEntryTo(int8_t authorId, string type = LOG_ENTRY);
    void sendEntryFromIndexTo(int8_t authorId, int8_t seqFrom);
    bool isMyNeighboursLogUpToDate(LogShell* lShell);
    bool isSubSequentSeqNum(LogShell* lShell);
    int8_t getKeyByValue(Ptr<NetDevice>);
    string getPrevHash();
//    bool isSubsequentContent();

public:
    void printNetworkLog();
    void gossip();



};


#endif //NS_3_30_NETWORKDEVICE_H
