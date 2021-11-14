//
// Created by Carlos Andrés Tejera on 14/11/2021.
//

#ifndef NS_3_30_FUNCTIONSHELL_H
#define NS_3_30_FUNCTIONSHELL_H

#include "ShellDecorator.h"

class FunctionShell : public Shell {
private:
  string function;
  string params;

public:
  FunctionShell (string function, string params) {
    this->function = function;
    this->params = params;
  }
  virtual ~FunctionShell () {}

  string assembleString() const override{
    return "function_shell=(function=" + this->function +
           " params=(" + this->params + ")" +
           + ")";
  }
};

#endif //NS_3_30_FUNCTIONSHELL_H
