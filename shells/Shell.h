//
// Created by Carlos Andrés Tejera on 14/11/2021.
//

#ifndef NS_3_30_SHELL_H
#define NS_3_30_SHELL_H
#include "../Config.h"
using namespace std;


class Shell {
public:
  virtual string assembleString() const = 0;
  virtual ~Shell() {}


  static pair<string, string> varSplitter (string s) {
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
    return make_pair (first, second);
  }

  static map<string, string> shellSplit (string s, string del) {
    map<string, string> v;

    int start = 0;
    int end = s.find(del);
    cout << ".-----" << endl;
    while (end != -1) {
      cout << s.substr(start, end - start) << endl;
      auto p = varSplitter (s.substr (start, end - start));
      v.insert(p);
      start = end + del.size();
      end = s.find(del, start);
    }
    cout << s.substr(start, end - start) << endl;
    v.insert(varSplitter (s.substr (start, end - start)));
    return v;
  }

  static void shell(string shellString)
  {
    cout << shellString << endl;
    auto nShell = "net_shell=(";
    auto fShell = "function_shell=(";
    auto lShell = "log_shell=(";
    //    auto cShell = "content_shell=(";

    auto start = shellString.find (nShell) + strlen (nShell);
    auto end = shellString.find_last_of (")") - start;
    auto netShellContent = shellString.substr (start, end);
    cout << netShellContent << endl;
    if (netShellContent.find (fShell) != string::npos)
      {
        auto fShellStart = netShellContent.find (fShell) + strlen (fShell);
        auto fShellEnd = netShellContent.find_last_of (")") - fShellStart;
        auto fShellContent = netShellContent.substr (fShellStart, fShellEnd);
        cout << fShellContent << endl;
        auto m = shellSplit (fShellContent, " ");
        cout << "luli" << endl;
        for (auto iter = m.begin (); iter != m.end (); iter++)
          {
            auto first = iter->first;
            auto sec = iter->second;
            cout << first << " " << sec << endl;
          }
      }
    else if (netShellContent.find (lShell))
      {
      }
  }
};

class ConcreteShell : Shell {
public:
  string assembleString() const override {
    return "default";
  }
};

#endif //NS_3_30_SHELL_H
