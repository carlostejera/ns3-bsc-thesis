#include "User.h"
using namespace ns3;
using namespace std;
void User::recvPkt(Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t proto, const Address& from, const Address& to, NetDevice::PacketType pt) {


}
/*PacketStream stream = this->createBuf(packet);

  stream.readSize();
  auto sender = stream.readMac();
  stream.readMac();

  uint8_t flag;
  uint8_t switchId;
  stream >> flag;
  stream >> switchId;
  switch(flag) {
  case (PACKET_CONFIRM_JOIN):
    cout << "Join confirmed from " << sender << ". Adding switch to list" << endl;
    this->registerSwitch(dev, sender, switchId);
    break;
    default:
      cout << "User " << this->nodeId << " received unknown packet with flag:" << to_string(flag) <<
      " and switchId: " << switchId << endl;
  }*//*

}

PacketStream User::createBuf(Ptr<const Packet> packet) {
  uint8_t buf[512];
  memset(buf, 0 , 512);
  packet->CopyData(buf, 512);
  PacketStream stream(buf);
  return stream;
}




void User::joinSwitch() {
  uint8_t buf[512];
  PacketStream stream(buf);

  stream << PACKET_JOIN << nodeId;

  // Broadcast that the user wants to joing the network
  for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i) {
    Ptr<NetDevice> dev = GetNode()->GetDevice(i);

    Ptr<Packet> p = Create<Packet>(stream.start(), stream.finish(dev->GetAddress(), ns3::Mac48Address("FF:FF:FF:FF:FF:FF")));

    if (!dev->Send(p, ns3::Mac48Address("FF:FF:FF:FF:FF:FF"), 0x800)) {
      std::cout << "Unable to send packet" << std::endl;
    }
  }
  cout << "User " << to_string(this->nodeId) << " sending a JOIN with flag " << to_string(PACKET_JOIN) << endl;
}

void User::registerSwitch(Ptr<NetDevice> dev, ns3::Mac48Address switchMac, uint8_t switchId) {
  this->connectedSwitches.insert(make_pair(switchId, make_pair(switchMac, dev)));
  cout << "Switch " << to_string(switchId) << "added to list;" << endl;

  for (auto it = this->connectedSwitches.begin(); it != this->connectedSwitches.end(); it++) {
    auto idk = it->second.first;
    cout << idk << endl;
  }
}*/
