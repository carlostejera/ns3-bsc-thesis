#ifndef NS_3_30_NETWORKDEVICE_H
#define NS_3_30_NETWORKDEVICE_H

#include "shells/LogStructures.h"
#include "CommunicationLog.h"
#include "../../build/ns3/ptr.h"

using namespace ns3;
using namespace std;


class NetworkDevice {
protected:
    // Attributes
    ostringstream packetOss;
    vector<std::string> familyMembers;
    std::map<std::string, Ptr<NetDevice>> neighbourMap;
    std::string authorId;
    map<string, pair<std::string , CommunicationLog*>> logs;
    map<string, CommunicationLog*> communicationLogs;
    vector<pair<string, CommunicationLog*>> subscriptions;
    CommunicationLog* myPersonalLog;
    string name;


    string readPacket(Ptr<const Packet> packet);
    Ptr<Packet> createPacket(NetShell* nShell);
    void sendPacket(Ptr<NetDevice> nDev, Ptr<Packet> p);
    bool isFamilyMember(std::string authorId);
    void sendEntryFromIndexTo(CommunicationLog* log, std::string receiverId, int8_t seqFrom, string type);
    std::string getKeyByValue(Ptr <NetDevice> senderDev);
    int8_t convertStringToId(string);
    bool logExists(NetShell* nShell);
    virtual bool concatenateEntry(NetShell* nShell);
    EnumFunctions hash(string input);
    bool isNeighbourToAdd(const std::string authorId, const uint8_t hops);
    bool isNeighbour(const std::string authorId);
    void addNeighbour(std::string authorId, Ptr<NetDevice> dev);
    virtual bool processReceivedSwitchPacket(NetShell* netShell, Ptr<NetDevice> dev) = 0;
    virtual void processReceivedUserPacket(NetShell* netShell, Ptr<NetDevice> dev) = 0;
    void printPacketResult();
    bool isGossipEntryOlder(NetShell* nShell);
    bool isEntryConcatenated(NetShell* netShell);
    const std::string LOGTYPE(std::string writer, std::string reader) const;
//    std::string type(std::string)

//    bool isSubsequentContent();

public:
    virtual void printNetworkLog();




};


#endif //NS_3_30_NETWORKDEVICE_H
