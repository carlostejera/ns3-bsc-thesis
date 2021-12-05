#include "Config.h"
#include "User.h"
#include "EthSwitch.h"
#include "Manager.h"


using namespace ns3;
using namespace std;

void SwitchScheduleJoin(Ptr <EthSwitch> dev) {
    dev->requestJoiningNetwork();
}

void SwitchScheduleGossip(Ptr <EthSwitch> dev) {
    dev->gossip();
}

void SwitchSchedulePrintNetworkLog(Ptr <EthSwitch> dev) {
    dev->printNetworkLog();
}

void ManagerSchedulePrintNetworkLog(Ptr <Manager> dev) {
    dev->printNetworkLog();
}
template <class T>
void addApplicationToNodes(Ptr<T>* apps, NodeContainer nodes, uint32_t beginFrom) {
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        apps[i] = Create<T>(i + beginFrom);
        nodes.Get(i)->AddApplication(apps[i]);
    }
}

void confNodes(AnimationInterface* anim,
                 Ptr<Node> node,
                 string path,
                 string desc,
                 int x,
                 int y) {
        anim->SetConstantPosition(node, x, y);  //set position of nodes in the animaiton
        anim->UpdateNodeSize(node->GetId(), 10, 10);
        anim->UpdateNodeImage(node->GetId(), anim->AddResource(path));
        anim->UpdateNodeDescription(node->GetId(), desc);
}


int main(int argc, char *argv[]) {

    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    uint32_t userNumbers = 2;
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

    // Connect switches to switches and to the manager
    for (uint32_t i = 0; i < switchNumbers - 1; i++) {
        p2p.Install(switch_nodes.Get(i), switch_nodes.Get(i + 1));
        p2p.Install(switch_nodes.Get(i), manager_nodes.Get(0));
    }
    p2p.Install(switch_nodes.Get(switchNumbers - 1), switch_nodes.Get(0));
    p2p.Install(switch_nodes.Get(switchNumbers - 1), manager_nodes.Get(0));

    //Connect users to switches
    p2p.Install(user_nodes.Get(0), switch_nodes.Get(0));
    p2p.Install(user_nodes.Get(1), switch_nodes.Get(3));

    Ptr <User> user_apps[userNumbers];
    Ptr <EthSwitch> switch_apps[switchNumbers];
    Ptr <Manager> manager_apps[managerNumbers];
    int ethId = 10;
    int managerId = 100;
    addApplicationToNodes<User>(user_apps, user_nodes, 0);
    addApplicationToNodes<EthSwitch>(switch_apps, switch_nodes, ethId);
    addApplicationToNodes<Manager>(manager_apps, manager_nodes, managerId);

    AnimationInterface anim("topology_bcs.xml");
    string pcPath = "/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/ns3-bsc-thesis/img/pc.svg";
    string switchPath = "/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/ns3-bsc-thesis/img/eth.svg";
    string managerPath = "/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/ns3-bsc-thesis/img/manager.svg";

    confNodes(&anim, user_nodes.Get(0), pcPath, "User" + to_string(0), 0, 0);
    confNodes(&anim, user_nodes.Get(1), pcPath, "User" + to_string(1), 100, 100);

    int x = 23;
    int y = 23;
    uint32_t i = 0;
    for (; i < switchNumbers / 2; i++) {
        confNodes(&anim, switch_nodes.Get(i), switchPath, "Switch" + to_string(i + ethId), x, y);
        x += 27;
    }

    y = 77;
    for (; i < switchNumbers; i++) {
        x -= 27;
        confNodes(&anim, switch_nodes.Get(i), switchPath, "Switch" + to_string(i + ethId), x, y);
    }

    confNodes(&anim, manager_nodes.Get(0), managerPath, "Manager" + to_string(managerId), 50, 50);

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


    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation

    return 0;
}
