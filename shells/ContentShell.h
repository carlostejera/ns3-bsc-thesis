//
// Created by Carlos Andrés Tejera on 14/11/2021.
//

#ifndef NS_3_30_CONTENTSHELL_H
#define NS_3_30_CONTENTSHELL_H

#include "Shell.h"
#include "../Config.h"

class ContentShell : public Shell {
private:
  string function;
  string params;
  string contentAsMsg;

public:
  ContentShell (string function, string params, string msg) {
  this->function = function;
  this->params = params;
  this->contentAsMsg = msg;
  }
  virtual ~ContentShell () {}

  string assembleString() const override{
    return "content_shell=(function=" + this->function +
        " params=(" + this->params + ")" +
        " message:" + this->contentAsMsg
        + ")";
  }
};

#endif //NS_3_30_CONTENTSHELL_H
