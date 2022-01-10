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

MyHeader::MyHeader() {
    // we must provide a public default constructor,
    // implicit or explicit, but never private.
}

MyHeader::~MyHeader() {
}

TypeId
MyHeader::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::MyHeader")
            .SetParent<Header>()
            .AddConstructor<MyHeader>();
    return tid;
}

TypeId
MyHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
MyHeader::Print(std::ostream &os) const {
    // This method is invoked by the packet printing
    // routines to print the content of my header.
    //os << "data=" << m_data << std::endl;
    os << "data=" << m_data;
}

uint32_t
MyHeader::GetSerializedSize(void) const {
    // we reserve 2 bytes for our header.
    return 2;
}

void
MyHeader::Serialize(Buffer::Iterator start) const {
    // we can serialize two bytes at the start of the buffer.
    // we write them in network byte order.
    start.WriteHtonU16(m_data);
}

uint32_t
MyHeader::Deserialize(Buffer::Iterator start) {
    // we can deserialize two bytes from the start of the buffer.
    // we read them in network byte order and store them
    // in host byte order.
    m_data = start.ReadNtohU16();

    // we return the number of bytes effectively read.
    return 2;
}

void
MyHeader::SetData(uint16_t data) {
    m_data = data;
}

uint16_t
MyHeader::GetData(void) const {
    return m_data;
}

