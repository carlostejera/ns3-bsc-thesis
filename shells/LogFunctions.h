#pragma once

#include "LogStructures.h"

void Printer::visit(ContentShell *cShell) {
    oss
            << "cShell=(function=" << cShell->function
            << "|params=" << cShell->params
            << "|msg=" << cShell->contMessage
            << ")";

}

void Printer::visit(LogShell *lShell) {
    oss
            << "lShell=(author=" << to_string(lShell->authorId)
            << "|seq=" << to_string(lShell->sequenceNum)
            << "|prevHash=" << lShell->prevEventHash << "|";
    lShell->shell->accept(this);
    oss << ")";

}


void Printer::visit(NetShell *nShell) {
    oss
            << "nShell=(receiver=" << nShell->macReceiver << "/" << to_string(nShell->receiverId)
            << "|type=" << nShell->type << "|";
    nShell->shell->accept(this);
    oss << ")";

}
