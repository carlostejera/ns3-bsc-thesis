#include "Config.h"
#include "User.h"
#include "EthSwitch.h"
#include "Manager.h"
#include "GlobalsValues.h"

using namespace ns3;
using namespace std;

void SwitchScheduleJoin(Ptr <EthSwitch> dev) {
    dev->requestJoiningNetwork();
}

void SwitchScheduleGossip(Ptr <EthSwitch> dev) {
    dev->gossip();
}

void UserScheduleJoin(Ptr<User> dev) {
    dev->plugAndPlay();
}

void UserScheduleSubscribe(Ptr<User> dev, uint8_t authorId) {
    dev->subscribe(authorId);
}

void UserSchedulePushLog(Ptr<User> dev) {
    dev->pushLogToSwitch();
}

template <class T>
void SchedulePrintNetworkLog(Ptr <T> dev) {
    dev->printNetworkLog();
}

template <class T>
void addApplicationToNodes(Ptr<T>* apps, NodeContainer nodes, uint32_t beginFrom, double errorRate) {
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        apps[i] = Create<T>(i + beginFrom, errorRate);
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

void otherTopology(const uint32_t userNumbers, const uint32_t switchNumbers, const uint32_t managerNumbers) {

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
    p2p.Install(user_nodes.Get(2), switch_nodes.Get(5));

    Ptr <User> user_apps[userNumbers];
    Ptr <EthSwitch> switch_apps[switchNumbers];
    Ptr <Manager> manager_apps[managerNumbers];
    int ethId = 10;
    int managerId = 100;

    addApplicationToNodes<User>(user_apps, user_nodes, 0, 0);
    addApplicationToNodes<EthSwitch>(switch_apps, switch_nodes, ethId, 0.001);
    addApplicationToNodes<Manager>(manager_apps, manager_nodes, managerId, 0);



    p2p.EnablePcap("test.pcap", user_nodes.Get(1)->GetDevice(0), true, true);

    AnimationInterface anim("topology_bcs.xml");
    anim.EnablePacketMetadata(true);
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



    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------

    uint64_t time = 2000;
    // Switches joining network
    for (uint8_t i = 0; i < switchNumbers; i++) {
        Simulator::Schedule(MilliSeconds(time), &SwitchScheduleJoin, switch_apps[i]);
    }

    // Gossiping
    for (int j = 0 ; j < 10; j ++ ) {
        for (uint8_t i = 0; i < switchNumbers; i++) {
            Simulator::Schedule(Seconds(time), &SwitchScheduleGossip, switch_apps[i]);
            time += 300;
        }
    }
    cout << VERBOSE << endl;

    for (uint8_t i = 0; i < userNumbers; i++) {
        Simulator::Schedule(Seconds(time), &UserScheduleJoin, user_apps[i]);
        time += 100;
    }

    for (int i = 0; i < 3; i++) {
        Simulator::Schedule(Seconds(time), &UserSchedulePushLog, user_apps[1]);
        time += 100;
    }

    Simulator::Schedule(Seconds(time), &UserScheduleSubscribe, user_apps[0], 1);
    time += 1000;
    Simulator::Schedule(Seconds(time), &UserScheduleSubscribe, user_apps[2], 1);

    time += 100;

    for (int i = 0; i < 3; i++) {
        Simulator::Schedule(Seconds(time), &UserSchedulePushLog, user_apps[1]);
        time += 0.1;
    }
    time += 100;


    for (int j = 0 ; j < 100; j ++ ) {
        for (uint8_t i = 0; i < switchNumbers; i++) {
            Simulator::Schedule(Seconds(time), &SwitchScheduleGossip, switch_apps[i]);
            time += 300;
        }
    }

    for (uint8_t i = 0; i < switchNumbers; i++) {
        Simulator::Schedule(Seconds(time), &SchedulePrintNetworkLog<EthSwitch>, switch_apps[i]);
    }
    Simulator::Schedule(Seconds(time), &SchedulePrintNetworkLog<Manager>, manager_apps[0]);

    for (uint8_t i = 0; i < userNumbers; i++) {
        Simulator::Schedule(Seconds(time), &SchedulePrintNetworkLog<User>, user_apps[i]);
    }

    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation


}

void lineTopology(const uint32_t userNumbers, const uint32_t switchNumbers, const uint32_t managerNumbers, const double errorRate) {
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
    p2p.Install(switch_nodes.Get(switchNumbers - 1), manager_nodes.Get(0));

    //Connect users to switches
    p2p.Install(user_nodes.Get(0), switch_nodes.Get(0));
    p2p.Install(user_nodes.Get(1), switch_nodes.Get(switchNumbers - 1));

    Ptr <User> user_apps[userNumbers];
    Ptr <EthSwitch> switch_apps[switchNumbers];
    Ptr <Manager> manager_apps[managerNumbers];
    int ethId = 10;
    int managerId = 100;

    addApplicationToNodes<User>(user_apps, user_nodes, 0, 0);
    addApplicationToNodes<EthSwitch>(switch_apps, switch_nodes, ethId, errorRate);
    addApplicationToNodes<Manager>(manager_apps, manager_nodes, managerId, 0);

    p2p.EnablePcap("test.pcap", user_nodes.Get(1)->GetDevice(0), true, true);

    string pcPath = "/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/ns3-bsc-thesis/img/pc.svg";
    string switchPath = "/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/ns3-bsc-thesis/img/eth.svg";
    string managerPath = "/mnt/c/Users/carlosandrestejera/Documents/university/bachelorthesis/source/ns-3.30/scratch/ns3-bsc-thesis/img/manager.svg";

    p2p.EnablePcapAll ("myNetworkPcaps");

    // CONSTELLATION
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    AnimationInterface anim("topology_bcs.xml");
    anim.EnablePacketMetadata(true);

    confNodes(&anim, user_nodes.Get(0), pcPath, "User" + to_string(0), 0, 0);
    confNodes(&anim, user_nodes.Get(1), pcPath, "User" + to_string(1), 100, 0);

    int distance = 20;
    int start = - ((distance * switchNumbers) / 2);

    confNodes(&anim, user_nodes.Get(0), pcPath, "User" + to_string(0), start, 0);
    confNodes(&anim, user_nodes.Get(1), pcPath, "User" + to_string(1), -start, 0);


    int x = start;
    int y = 50;
    uint32_t i = 0;
    for (; i < switchNumbers; i++) {
        confNodes(&anim, switch_nodes.Get(i), switchPath, "Switch" + to_string(i + ethId), x, y);
        x += distance;
    }

    confNodes(&anim, manager_nodes.Get(0), managerPath, "Manager" + to_string(managerId), 0, 0);

    // SCHEDULE
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    uint32_t time = 2000;
    // Switches joining network
    for (uint8_t i = 0; i < switchNumbers; i++) {
        Simulator::Schedule(MilliSeconds(time), &SwitchScheduleJoin, switch_apps[i]);
        time += 100;
    }
    for (int j = 0; j < 100; j++) {
        for (uint8_t i = 0; i < switchNumbers; i++) {
            Simulator::Schedule(Seconds(time), &SwitchScheduleGossip, switch_apps[i]);
        }
        time += 2000;
    }



    //PRINTER
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    for (uint8_t i = 0; i < switchNumbers; i++) {
        Simulator::Schedule(Seconds(time), &SchedulePrintNetworkLog<EthSwitch>, switch_apps[i]);
    }
    Simulator::Schedule(Seconds(time), &SchedulePrintNetworkLog<Manager>, manager_apps[0]);

    for (uint8_t i = 0; i < userNumbers; i++) {
        Simulator::Schedule(Seconds(time), &SchedulePrintNetworkLog<User>, user_apps[i]);
    }

    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation

}


int main(int argc, char *argv[]) {

    CommandLine cmd;
    cmd.Parse(argc, argv);
    cout << VERBOSE << endl;
    Time::SetResolution(Time::NS);
    Packet::EnablePrinting();
    Packet::EnableChecking();
//    otherTopology(3, 6, 1);
    lineTopology(2, 10, 1, 0.009);
    return 0;
}
