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
            << "lShell=(author=" << lShell->authorId
            << "|seq=" << to_string(lShell->sequenceNum)
            << "|prevHash=" << lShell->prevEventHash
            << "|timestamp=" << lShell->timestamp
            << "|signature=" << lShell->signature << "|";
    lShell->shell->accept(this);
    oss << ")";

}


void Printer::visit(NetShell *nShell) {
    oss
    << "nShell=(send_timestamp=" << nShell->timestamp
    << "|log_type=" << nShell->type
    << "|flag=" << to_string(nShell->flag)
    << "|hops=" <<  to_string(nShell->hops) << "|";
    nShell->shell->accept(this);
    oss << ")";

}

