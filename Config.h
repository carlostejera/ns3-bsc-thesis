#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "PacketStream.h"
#include <mutex>


using namespace ns3;
using namespace std;


// Manager variables
const uint8_t PACKET_JOIN = 1;
const uint8_t PACKET_CONFIRM_JOIN = 2;
const uint8_t PACKET_DEJOIN = 15;
//const uint8_t PACKET_PING = 3;
const uint8_t PACKET_PING_RESPONSE = 4;
const uint8_t PACKET_JOIN_MANAGER = 5;
const uint8_t PACKET_JOIN_CONFIRM_MANAGER = 6;
const uint8_t USER_JOIN_REQUEST_APPROVED = 7;
const uint8_t USER_JOIN_REQUEST = 8;
const uint8_t NETWORK_CHANGES = 9;
const uint8_t LOG_ENTRIES = 10;
const uint8_t HERE_ARE_THE_LOG_ENTRIES = 3;
const uint8_t PACKET_PING = 11;
const uint8_t GOSSIPING = 12;
const uint8_t NEIGHBOUR_ASKED_FOR_LOGS = 13;
const uint8_t NEIGHBOUR_GOSSIP_ANSWER = 14;
const uint8_t REMOVE_SWITCH_FROM_LIST = 15;


#endif