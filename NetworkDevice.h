#ifndef NS_3_30_NETWORKDEVICE_H
#define NS_3_30_NETWORKDEVICE_H

#include "shells/LogStructures.h"
#include "CommunicationLog.h"

using namespace ns3;
using namespace std;


class NetworkDevice {
protected:
    ostringstream packetOss;
    vector<int8_t> familyMembers;
    std::map<int8_t, Ptr<NetDevice>> neighbourMap;
    CommunicationLog* networkLog;
    int8_t authorId;
    map<string, pair<int8_t , CommunicationLog*>> logs;
    Ptr<RateErrorModel> rem;

    string readPacket(Ptr<const Packet> packet);
    Ptr<Packet> createPacket(NetShell* nShell);
    void sendPacket(Ptr<NetDevice> nDev, Ptr<Packet> p);
    bool isFamilyMember(int8_t authorId);
    void sendLastEntryTo(int8_t authorId, string type = LOG_ENTRY);
    void sendEntryFromIndexTo(CommunicationLog* log, int8_t receiverId, int8_t seqFrom, string type);
    bool isMyNeighboursLogUpToDate(LogShell* lShell);
    bool isSubSequentSeqNum(LogShell* lShell);
    int8_t getKeyByValue(Ptr<NetDevice>);
    int8_t convertStringToId(string);
    bool logExists(NetShell* nShell);
    void addLog(NetShell* nShell);
    bool concatenateEntry(NetShell* nShell);
    EnumFunctions hash(string input);
    bool isNeighbourToAdd(const int8_t authorId, const uint8_t hops);
    bool isNeighbour(const int8_t authorId);
    void addNeighbour(int8_t, Ptr<NetDevice>);
    virtual bool processReceivedSwitchPacket(NetShell* netShell, Ptr<NetDevice> dev) = 0;
    virtual void processReceivedUserPacket(NetShell* netShell, Ptr<NetDevice> dev) = 0;
    void printPacketResult();

//    bool isSubsequentContent();

public:
    virtual void printNetworkLog();

    virtual void gossip();



};


#endif //NS_3_30_NETWORKDEVICE_H
