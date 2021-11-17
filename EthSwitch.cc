#include "EthSwitch.h"
#include "fetch.h"

using namespace ns3;
using namespace std;


bool EthSwitch::registerUser(Ptr<NetDevice> dev, ns3::Mac48Address userMac, uint8_t deviceId) {
  // lock.lock();
  this->connectedUsers.insert(make_pair(deviceId, make_pair(userMac, dev)));
  cout << "User added to list;" << endl;
  // lock.unlock();


  for (auto it = this->connectedUsers.begin(); it != this->connectedUsers.end();) {
    auto idk = it->second.first;
    cout << idk << endl;
  }
  return true;
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::sendConfirmJoin(Ptr<NetDevice> dev, ns3::Mac48Address sender, uint8_t msg) {
  uint8_t buf[512];
  PacketStream stream(buf);

  stream << msg << this->nodeId;

  Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), sender));

  if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
    std::cout << "Unable to send packet" << std::endl;
  }

}

//-----------------------------------------------------------------------------------------------


void EthSwitch::joinManagersNetwork() {
  uint8_t buf[512];
  PacketStream stream(buf);
  NetShell* nShell = new NetShell(ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), new FunctionShell("addToNetwork", ""));
  string netShells = nShell->assembleString();
  std::vector<uint8_t> myVector(netShells.begin(), netShells.end());
  uint8_t *text = &myVector[0];

  // Broadcast that the user wants to join the network
  for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
    Ptr<Packet> p = Create<Packet>(text, strlen ((char *) text));

    Ptr<NetDevice> dev = GetNode()->GetDevice(i);
    if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
      std::cout << "Unable to send packet" << std::endl;
    }
  }
  cout << "Switch " << to_string(this->nodeId) << " sending a JOIN to the manager." << endl;
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::forwardUserJoinRequest(uint8_t userId, uint8_t msg) {
  uint8_t buf[512];
  PacketStream stream(buf);

  stream << msg << userId;

  Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(manager.second->GetAddress(), manager.first));
  cout << "Forwarding the JOIN of user to the manager" << endl;
  if (!manager.second->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
    std::cout << "Unable to send packet" << std::endl;
  }
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::addToLog(int8_t seq, ns3::Mac48Address mac) {
  this->log.push_back(make_pair(seq, mac));
}

//-----------------------------------------------------------------------------------------------

/*void EthSwitch::printLogEntries() {
  cout << "--------------------------------" << endl;
  cout << "Log entry from Switch " << to_string(this->nodeId) << endl;
  cout << "" << endl;
  for (auto it = this->log.begin(); it != this->log.end(); it++) {
    auto seq = it->first;
    auto mac = it->second;
    cout << "Sequence: " << to_string(seq) << " MacAddress: " << mac << endl;
  }
  cout << "" << endl;

}*/

//-----------------------------------------------------------------------------------------------

bool EthSwitch::needEntries(int8_t managerSeq) {
  if (this->seq == 0) {
    return true;
  }

  cout << to_string(managerSeq) << " and " << to_string(this->seq) << " and " << to_string(managerSeq - this->seq) << " and " << ((managerSeq - this->seq) == 0) << endl;
  return ((managerSeq - this->seq) >= 0) ? false : true;
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::askForLogEntries() {
  uint8_t buf[512];
  PacketStream stream(buf);

  stream << LOG_ENTRIES << this->seq;

  Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(manager.second->GetAddress(), manager.first));
  if (!manager.second->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
    std::cout << "Unable to send packet" << std::endl;
  }
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::gossip() {

  uint8_t buf[512];
  PacketStream stream(buf);

  stream << GOSSIPING << this->nodeId << this->seq;
  cout << "Switch" << to_string(this->nodeId) << " telling his neighbours about its sequence number " << to_string(this->seq) << endl;

  // Broadcast that the user wants to join the network
  for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
    Ptr<NetDevice> dev = GetNode()->GetDevice(i);
    if(dev->GetAddress() == this->manager.second->GetAddress()) {
      continue;
    }
    Ptr<Packet> p = Create<Packet>(
        stream.start(),
        stream.finish(dev->GetAddress(), ns3::Mac48Address("FF:FF:FF:FF:FF:FF")));

    if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
      std::cout << "Unable to send packet" << std::endl;
    }
  }
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::giveNeighbourLogs(Ptr<NetDevice> dev, ns3::Mac48Address sender, int8_t missingSeq) {
  //this->sendLogEntries(dev, sender, missingSeq);

  uint8_t buf[512];
  PacketStream stream(buf);
  auto endSeq = (this->log.back().first + 1) - missingSeq;
  cout << this->seq << endl;
  stream << NEIGHBOUR_GOSSIP_ANSWER << this->nodeId << (this->seq - 1) << endSeq ;


  for (auto it = this->log.begin(); it != this->log.end(); it++) {
    auto managerSeq = it->first;
    auto macAddr = it->second;
    cout << "managerseq " << to_string(managerSeq) << "and missing seq " << to_string(missingSeq) << endl;
    if (missingSeq <= managerSeq) {
      stream.writeMac(macAddr);
      cout << "writiiiing" << endl;
    }
  }

  Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), sender));
  cout << "Sending " << to_string(endSeq) << " entries to switch: " << dev->GetAddress() << endl;
  if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
    std::cout << "Unable to send packet" << std::endl;
  }
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::printLogEntries() {
  this->cLog.readLog();
}

//-----------------------------------------------------------------------------------------------


void EthSwitch::sendLogEntries(Ptr<NetDevice> dev, ns3::Mac48Address sender, int8_t seq) {

  uint8_t buf[512];
  PacketStream stream(buf);
  auto endSeq = this->log.back().first - seq;
  stream << HERE_ARE_THE_LOG_ENTRIES << this->nodeId << endSeq;


  for (auto it = this->log.begin(); it != this->log.end(); it++) {
    auto managerSeq = it->first;
    auto macAddr = it->second;
    if (seq < managerSeq) {
      stream.writeMac(macAddr);
    }
  }

  Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), sender));
  cout << "Sending entries to switch " << endl;
  if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
    std::cout << "Unable to send packet" << std::endl;
  }
}

//-----------------------------------------------------------------------------------------------

void EthSwitch::dejoin() {

  uint8_t buf[512];
  PacketStream stream(buf);

  stream << PACKET_DEJOIN << this->nodeId;
  cout << "Dejoining the network" << endl;

   for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
    Ptr<NetDevice> dev = GetNode()->GetDevice(i);
    if(dev->GetAddress() == this->manager.second->GetAddress()) {
      Ptr<Packet> p = Create<Packet>(
          stream.start(),
          stream.finish(dev->GetAddress(), this->manager.second->GetAddress()));

      if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
        std::cout << "Unable to send packet" << std::endl;
        }
    }
  }
}

//-----------------------------------------------------------------------------------------------


void EthSwitch::recvPkt(
    Ptr<NetDevice> dev,
    Ptr<const Packet> packet,
    uint16_t proto,
    const Address& from,
    const Address& to,
    NetDevice::PacketType pt ) {

  uint32_t pktSize = packet->GetSize();
  uint8_t buf[pktSize];
  memset(buf, 0 , pktSize);
  packet->CopyData(buf, pktSize);
  string netShell = string ((char *)buf);
  cout << netShell  <<  endl;


  PacketStream stream(buf);
  stream.readSize();

  auto sender = stream.readMac();
  stream.readMac();

  uint8_t flag;
  uint8_t deviceId;
  uint8_t seqN;
  uint8_t neighbourSeq;

  uint8_t tmp;



  stream >> flag;
  stream >> deviceId;
  stream >> seqN;
  pair<ns3::Mac48Address, Ptr<NetDevice>> p;
  auto newSwitch = stream.readMac();
  cout << "---" << endl;
  fncptr f;
  //cout << "Switch" << to_string(this->nodeId) << " receives a packet from " << sender << "with flag " << to_string(flag) <<endl;
  switch(flag) {
  case (PACKET_JOIN):
    cout << "Join received from " << to_string(deviceId) << ". Requesting acceptance of the manager." << endl;
    tmpConnectedUsers.insert(make_pair(deviceId, make_pair(sender, dev)));
    this->forwardUserJoinRequest(deviceId, USER_JOIN_REQUEST);
    break;
    case (PACKET_JOIN_CONFIRM_MANAGER):
      cout << "Accepted by the manager. Adding him to my variable" << endl;
      this->manager = make_pair(sender, dev);
      break;
      case (USER_JOIN_REQUEST_APPROVED):
        if (tmpConnectedUsers.find(deviceId) != tmpConnectedUsers.end()) {
          auto m = tmpConnectedUsers[deviceId];
          tmpConnectedUsers.erase(deviceId);
          cout << "Manager approved the connection with the network. Registering now User" << endl;
          if (this->registerUser(m.second, m.first, deviceId)) {
            this->sendConfirmJoin(m.second, m.first, PACKET_CONFIRM_JOIN);
          }
        }
        break;
      case (NETWORK_CHANGES):
        cout << "Receiving network changes. Pair " << newSwitch << " with seq " << to_string(seqN) << endl; //to_string(p.first) << endl;
        // this is on reverse bcs something does not work well
        if (needEntries(seqN)) {
          cout << "I need " << to_string(seqN - this->seq) << " log entries" << endl;
          this->askForLogEntries();

        } else {
          cout << "I (Switch" << to_string(this->nodeId) << ") am already up-to-date." << " My seq is " << to_string(this->seq) << endl;
        }
        break;

        case (HERE_ARE_THE_LOG_ENTRIES):
          cout << "Writing " << to_string(seqN) << "entries into my log" << endl;

          for (int8_t i = 0; i < seqN; i++) {
            auto entry = stream.readMac();
            this->log.push_back(make_pair(this->seq, entry));
            this->cLog.addToLog(this->seq, "", HERE_ARE_THE_LOG_ENTRIES, "Adding MAC to my list. ", entry, deviceId);
            ++this->seq;
          }
          break;
        case (NEIGHBOUR_GOSSIP_ANSWER):
            this->lock.lock();
            stream >> neighbourSeq;
            cout << "deviceid" << to_string(deviceId)<< " endseq" << to_string(seqN) << "neighbour seq" << to_string(neighbourSeq) << endl;

            if (seqN < this->seq) {
              cout << "Somebody updated me, thanks though." << endl;
              this->lock.unlock();
              return;
            }
            tmp = seqN - this->seq + 1;
            for (int8_t i = 0; i < tmp; i++) {
              cout << " I (Switch" << to_string(this->nodeId) << ") received " << to_string(seqN) << " events bcs my seq num is" << to_string(this->seq) << endl;
              auto entry = stream.readMac();
              this->log.push_back(make_pair(this->seq++, entry));
            }
            this->lock.unlock();
            cout << "My new seq num is " << to_string(this->seq) << endl;
            break;
          case (GOSSIPING):
              neighbourSeq = seqN;
              cout << sender << " is gossiping. It has sequence number " << to_string(neighbourSeq) << endl;
              if (needEntries(neighbourSeq)) {
                this->giveNeighbourLogs(dev, sender, neighbourSeq);
              } else {
                cout << "My neighbour is good, I (Switch" << to_string(this->nodeId) << " have the same sequence number" << endl;
              }
              break;

          case (NEIGHBOUR_ASKED_FOR_LOGS):
            this->sendLogEntries(dev, sender, seqN);
            break;
          case (REMOVE_SWITCH_FROM_LIST):
            f = fetch("addToLog");
            f(this->cLog, this->seq, "", HERE_ARE_THE_LOG_ENTRIES, "Removing MAC from my list. ", ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), deviceId);
            //this->cLog.addToLog(this->seq, "", HERE_ARE_THE_LOG_ENTRIES, "Removing MAC from my list. ", ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), deviceId);
            this->seq++;
            break;
          default:
            cout << "Switch" << to_string(this->nodeId) << " received unknown packet with flag:" << to_string(flag) <<
            " and deviceId " << to_string(deviceId) << endl;


  }

}