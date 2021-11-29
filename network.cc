#include "Config.h"
#include "User.h"
#include "EthSwitch.h"
#include "Manager.h"


using namespace ns3;
using namespace std;

void SwitchScheduleJoin(Ptr<EthSwitch> dev) {
  dev->requestJoiningNetwork();
}

void SwitchScheduleGossip(Ptr<EthSwitch> dev) {
    dev->gossip();
}

void SwitchSchedulePrintNetworkLog(Ptr<EthSwitch> dev) {
    dev->printNetworkLog();
}

void ManagerSchedulePrintNetworkLog(Ptr<Manager> dev) {
    dev->printNetworkLog();
}



int main(int argc, char* argv[]) {

  CommandLine cmd;
  cmd.Parse(argc, argv);

  Time::SetResolution(Time::NS);

  uint32_t userNumbers = 3;
  uint32_t switchNumbers = 6;
  uint32_t managerNumbers = 1;

  NodeContainer user_nodes;
  user_nodes.Create(userNumbers);

  NodeContainer switch_nodes;
  switch_nodes.Create(switchNumbers);

  NodeContainer manager_nodes;
  manager_nodes.Create(managerNumbers);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  for (uint32_t i = 0; i < switchNumbers - 2; i++) {
    p2p.Install(switch_nodes.Get(i), switch_nodes.Get(i + 1));
    p2p.Install(switch_nodes.Get(i), manager_nodes.Get(0));

  }
  p2p.Install(switch_nodes.Get(switchNumbers - 2), manager_nodes.Get(0));

  p2p.Install(switch_nodes.Get(switchNumbers - 1), manager_nodes.Get(0));
  p2p.Install(switch_nodes.Get(switchNumbers - 1), switch_nodes.Get(0));
  p2p.Install(switch_nodes.Get(switchNumbers - 1), switch_nodes.Get(1));

  Ptr<User> user_apps[userNumbers];
  Ptr<EthSwitch> switch_apps[switchNumbers];
  Ptr<Manager> manager_apps[managerNumbers];

  for (uint32_t i = 0; i < userNumbers; i++) {
    user_apps[i] = Create<User>(i);
    user_nodes.Get(i)->AddApplication(user_apps[i]);
  }

  for (uint32_t i = 0; i < switchNumbers; i++) {
    switch_apps[i] = Create<EthSwitch>(i);
    switch_nodes.Get(i)->AddApplication(switch_apps[i]);
  }

  for (uint32_t i = 0; i < managerNumbers; i++) {
    manager_apps[i] = Create<Manager>(100 + i);
    manager_nodes.Get(i)->AddApplication(manager_apps[i]);
  }


  AnimationInterface anim("topology_bcs.xml");
  int x = 0;
  int y = 75;
  for (uint32_t i = 0; i < userNumbers; i++) {
    anim.SetConstantPosition(user_nodes.Get(i), x, y);  //set position of nodes in the animaiton
    anim.UpdateNodeSize(i, 10, 10);
    anim.UpdateNodeImage(i, anim.AddResource("/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/computer.svg"));
    anim.UpdateNodeDescription(i, "PC" + to_string(i));

    x += 25;
  }
  x = 0;
  y = 50;
  for (uint32_t i = 0; i < switchNumbers; i++) {
    anim.SetConstantPosition(switch_nodes.Get(i), x, y);
    anim.UpdateNodeSize(userNumbers + i, 5, 5);
    anim.UpdateNodeImage(userNumbers + i, anim.AddResource("/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/switch.svg"));
    anim.UpdateNodeDescription(userNumbers + i, "Switch" + to_string(i));
    x += 20;
  }
  anim.SetConstantPosition(switch_nodes.Get(switchNumbers - 1), 10, 75);

  anim.SetConstantPosition(manager_nodes.Get(0), 50, 0);
  anim.UpdateNodeSize(userNumbers + switchNumbers + 0, 5, 5);
  anim.UpdateNodeImage(userNumbers + switchNumbers+ 0, anim.AddResource("/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/controller.svg"));
  anim.UpdateNodeDescription(userNumbers + switchNumbers+ 0, "Manager" + to_string(0));

  int time = 1;
  for (uint8_t i = 0; i < switchNumbers; i++) {
    Simulator::Schedule(Seconds(time), &SwitchScheduleJoin, switch_apps[i]);
    ++time;
  }

    for (uint8_t i = 0; i < switchNumbers; i++) {
        Simulator::Schedule(Seconds(time), &SwitchScheduleGossip, switch_apps[i]);
        ++time;
    }

    for (uint8_t i = 0; i < switchNumbers; i++) {
        Simulator::Schedule(Seconds(time), &SwitchSchedulePrintNetworkLog, switch_apps[i]);
        ++time;
    }

    Simulator::Schedule(Seconds(time), &ManagerSchedulePrintNetworkLog, manager_apps[0]);


    /*for (uint8_t i = 0; i < switchNumbers; i++) {
      Simulator::Schedule(Seconds(time), &SwitchScheduleGossip, switch_apps[i]);
      ++time;
    }

    Simulator::Schedule(Seconds(time), &SwitchScheduleDejoin, switch_apps[0]);
    ++time;

    for (uint8_t i = 0; i < switchNumbers; i++) {
      Simulator::Schedule(Seconds(time), &SwitchSchedulePrint, switch_apps[i]);
    }*/

  Simulator::Run(); //run simulation
  Simulator::Destroy(); //end simulation

  return 0;
}
