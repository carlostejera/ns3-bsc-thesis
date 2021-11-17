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



};

class ConcreteShell : Shell {
public:
  string assembleString() const override {
    return "default";
  }
};

#endif //NS_3_30_SHELL_H
