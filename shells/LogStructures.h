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

using namespace std;

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
    int8_t sequenceNum = 0;
    string prevEventHash;
    int8_t authorId;
    string timestamp;
    ContentShell* shell;
//    static long globalHash = 100000;
    long myHash; 


    LogShell(int8_t seqNum, string prevHash, int8_t authorId, ContentShell* shell) :
            sequenceNum(seqNum),
            prevEventHash(prevHash),
            authorId(authorId),
            shell(shell)
    {
        //TODO: Add event checker with the global hash
//        this->myHash = globalHash;
//        globalHash++;
    }

    void accept(ExpressionVisitor* visitor) override {
        visitor->visit(this);
    }
};

struct NetShell : public Shell {
    ns3::Mac48Address macReceiver;
    int8_t receiverId;
    string type;
    LogShell* shell;

    NetShell(ns3::Mac48Address mac, int8_t receiverId, string type, LogShell* shell){
        this->macReceiver = mac;
        this->receiverId = receiverId;
        this->type = type;
        this->shell = shell;
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
        m = shellSplit(cShellContent, " ");
        ContentShell *cShellNew = new ContentShell(m["function"], m["params"], "someMsg");

        //Params of log shell
        auto logShellParams = lShellContent.erase(lShellContent.find(C_SHELL));

        m = shellSplit(logShellParams, " ");

        stringstream ssAuthor(m["author"]);
        stringstream ssSeq(m["seq"]);
        int seq;
        int author_id;
        ssAuthor >> author_id;
        ssSeq >> seq;

        LogShell *lShellNew = new LogShell(
                seq,
                m["prevHash"],
                author_id,
                cShellNew
        );
        auto netShellParams = nShellContent.erase(nShellContent.find(L_SHELL));
        m = shellSplit(netShellParams, " ");
        string tmp = m["receiver"];
        auto receiverPair = varSplitter(tmp, "/");
        const char *bruh = receiverPair.first.c_str();

        stringstream ssId(receiverPair.second);
        int receiverId;
        ssId >> receiverId;
        auto resultShell = new NetShell(
                ns3::Mac48Address(bruh),
                receiverId,
                m["type"],
                lShellNew
        );
        return resultShell;
    }

    static NetShell* shell(string shellString) {
        NetShell *shell = helpFunction(shellString);
        return shell;
    }
};

