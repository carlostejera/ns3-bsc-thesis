//
// Created by Carlos Andrés Tejera on 15/11/2021.
//

#ifndef NS_3_30_SHELLING_H
#define NS_3_30_SHELLING_H

#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace std;
using namespace ns3;

class Shelling {
    typedef map <string, string> MapFunc;

    static constexpr const char* N_SHELL = "net_shell=(";
    static constexpr const char* F_SHELL = "function_shell=(";
    static constexpr const char* L_SHELL = "log_shell=(";
    static constexpr const char* C_SHELL = "content_shell=(";
public:
    static pair <string, string> varSplitter(string s) {
        string del = "=";
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
//        cout << ".-----" << endl;
        while (end != -1) {
//            cout << s.substr(start, end - start) << endl;
            auto p = varSplitter(s.substr(start, end - start));
            v.insert(p);
            start = end + del.size();
            end = s.find(del, start);
        }
//        cout << s.substr(start, end - start) << endl;
        v.insert(varSplitter(s.substr(start, end - start)));
        return v;
    }

    template<class T>

};

#endif //NS_3_30_SHELLING_H
