//
// Created by Carlos Andrés Tejera on 14/11/2021.
//

#ifndef NS_3_30_NETSHELL_H
#define NS_3_30_NETSHELL_H

#include "ShellDecorator.h"

using namespace ns3;

class NetShell : public ShellDecorator {
private:
  ns3::Mac48Address receiver;
  string type;
  string communicationFunction;

public:
  NetShell(Shell* shell, Mac48Address receiver) : ShellDecorator (shell) {
    this->receiver = receiver;
  }

  string  assembleString() const override {
    ostringstream s1;
    s1 << this->receiver;
    return "net_shell=(receiver=" + s1.str() + " " +
        this->shell_->assembleString() + ")";

  }
};

#endif //NS_3_30_NETSHELL_H
