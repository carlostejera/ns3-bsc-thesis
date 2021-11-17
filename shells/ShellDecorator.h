//
// Created by Carlos Andrés Tejera on 14/11/2021.
//

#ifndef NS_3_30_SHELLDECORATOR_H
#define NS_3_30_SHELLDECORATOR_H
#include "Shell.h"
#include "../Config.h"
class ShellDecorator : public Shell {
protected:
  Shell *shell_;

public:
  ShellDecorator(Shell *shell): shell_ (shell) {
    this->shell_ = shell;
  }

  string assembleString() const override{
    return this->shell_->assembleString();
  }


  Shell* getInnerShell(){
    return this->shell_;
  }
};

#endif //NS_3_30_SHELLDECORATOR_H
