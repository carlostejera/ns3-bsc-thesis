#pragma once
#include <iostream>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include <map>
#include "../Enums.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <iostream>
#include "ns3/type-id.h"
#include "../../../build/ns3/simulator.h"

using namespace std;
using namespace ns3;

struct ContentShell;
struct LogShell;
struct NetShell;
struct Printer;

typedef map <string, string> MapFunc;

static constexpr const char* N_SHELL = "nShell=(";
static constexpr const char* L_SHELL = "lShell=(";
static constexpr const char* C_SHELL = "cShell=(";

static constexpr const char* REQUEST = "request";
static constexpr const char* DIARY = "diary";
static constexpr const char* LOG_ENTRY = "logEntry";
static constexpr const char* ADD_MEMBER = "addMemberToNetwork";

static constexpr const char* ASSIGN_MANAGER = "assignManager";
static constexpr const char* GOSSIP = "gossip";


static constexpr const char* UNSUBSCRIBE = "removeUserFromInl";

static constexpr const char* SWITCH_PREFIX = "switch:";
static constexpr const char* USER_PREFIX = "user:";
static constexpr const char* MANAGER_PREFIX = "manager:";
static constexpr const char* ALL_SUFFIX = "*";
static constexpr const char* MANAGER_RECEIVER = *("/") + MANAGER_PREFIX;
static constexpr const char* SWITCH_RECEIVER = *("/") + SWITCH_PREFIX;
static constexpr const char* USER_RECEIVER = *("/") + USER_PREFIX;
static constexpr const char* USER_ALL = "user:*";
static constexpr const char* SWITCH_ALL = "switch:*";
static constexpr const char* MANAGER_ALL = "manager:*";
static constexpr const char* ADD_USER_TO_NETWORK = "addToNetwork";
//static std::string prefixKey = "-----BEGIN PUBLIC KEY-----\n";
//static std::string suffixKey = "-----END RSA PRIVATE KEY-----";



struct ExpressionVisitor {
    ostringstream oss;
    string str() const {return oss.str();}
    void clearOss() {oss.str("");}

    virtual void visit(ContentShell* cShell) = 0;
    virtual void visit(LogShell* lShell) = 0;
    virtual void visit(NetShell* nShell) = 0;
};

struct Printer : ExpressionVisitor {
    void  visit(ContentShell* cShell) override;
    void  visit(LogShell* lShell) override;
    void  visit(NetShell* nShell) override;
};

struct Shell {
    virtual ~Shell() = default;
    // virtual string assembleString() const = 0;
    virtual void accept(ExpressionVisitor* visitor) = 0;

};

struct ContentShell : public Shell {
    string function;
    string params;
    string contMessage;

    ContentShell(string f, string p, string msg) : function(f), params(p), contMessage(msg) {}

    void accept(ExpressionVisitor* visitor) override {
        visitor->visit(this);
    }
};

struct LogShell : public Shell {
    int16_t sequenceNum = 0;
    string prevEventHash;
    std::string authorId;
    ContentShell* shell;
    std::string timestamp;
    std::string signature;
    long myHash;


    LogShell(std::string timestamp, int16_t seqNum, string prevHash, std::string authorId, std::string signature, ContentShell* shell) :
            sequenceNum(seqNum),
            prevEventHash(prevHash),
            authorId(authorId),
            shell(shell),
            timestamp(timestamp),
            signature(signature)
    {
    }

    void accept(ExpressionVisitor* visitor) override {
        visitor->visit(this);
    }
};

struct NetShell : public Shell {
    string type;
    LogShell* shell;
    uint8_t hops;
    uint8_t flag;
    std::string timestamp;

    NetShell(std::string receiverId,
             string type,
             uint8_t flag,
             uint8_t hops,
             LogShell *shell,
             std::string timestamp = to_string(Simulator::Now().GetSeconds())) {
        this->timestamp = timestamp;
        this->type = type;
        this->shell = shell;
        this->hops = hops;
        this->flag = flag;
    }

    void accept(ExpressionVisitor* visitor) override {
        visitor->visit(this);
    }
};


struct SomeFunctions {
    static pair <string, string> varSplitter(string s, string del = "=") {
        int start = 0;
        int end = s.find(del);
        string first;
        string second;
        while (end != -1) {
            first = s.substr(start, end - start);
            start = end + del.size();
            end = s.find(del, start);
        }
        second = s.substr(start, end - start);
        return make_pair(first, second);
    }

    static map <string, string> shellSplit(string s, string del) {
        map <string, string> v;

        int start = 0;
        int end = s.find(del);
        while (end != -1) {
            auto p = varSplitter(s.substr(start, end - start));
            v.insert(p);
            start = end + del.size();
            end = s.find(del, start);
        }
        v.insert(varSplitter(s.substr(start, end - start)));
        return v;
    }

    static NetShell* helpFunction(string shellString) {

        MapFunc m;
        int startIndex = shellString.find(N_SHELL) + strlen(N_SHELL);
        int end = shellString.find_last_of(")") - startIndex;

        string nShellContent = shellString.substr(startIndex, end);
        // Inner of log shell
        // TODO: Change that 7
        startIndex = nShellContent.find(L_SHELL) + strlen(L_SHELL);
        end = nShellContent.find_last_of(")") - startIndex;
        string lShellContent = nShellContent.substr(startIndex, end);
        // inner of content shell
        startIndex = lShellContent.find(C_SHELL) + strlen(C_SHELL);
        end = lShellContent.find_last_of(")") - startIndex;


        string cShellContent = lShellContent.substr(startIndex, end);
        // content logShellParams
        m = shellSplit(cShellContent, "|");
        ContentShell *cShellNew = new ContentShell(m["function"], m["params"], m["msg"]);

        //Params of log shell
        auto logShellParams = lShellContent.erase(lShellContent.find(C_SHELL));

        m = shellSplit(logShellParams, "|");

        stringstream ssSeq(m["seq"]);
        int seq;
        ssSeq >> seq;

        LogShell *lShellNew = new LogShell(
                m["timestamp"],
                seq,
                m["prevHash"],
                m["author"],
                m["signature"],
                cShellNew
        );
        auto netShellParams = nShellContent.erase(nShellContent.find(L_SHELL));
        m = shellSplit(netShellParams, "|");
        stringstream ssHops(m["hops"]);
        int hops;
        ssHops >> hops;
        string tmp = m["receiver"];
        auto receiverPair = varSplitter(tmp, "/");

        stringstream ssFlag(m["flag"]);
        int flag;
        ssFlag >> flag;
        auto resultShell = new NetShell(
            receiverPair.second,
            m["log_type"],
            flag,
            hops,
            lShellNew,
            m["send_timestamp"]
        );
        return resultShell;
    }

    static NetShell* shell(string shellString) {
        NetShell *shell = helpFunction(shellString);
        return shell;
    }
};

