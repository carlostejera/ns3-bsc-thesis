#include "Config.h"
#include "User.h"
#include "EthSwitch.h"
#include "Manager.h"

using namespace ns3;
using namespace std;

string userPath = "/home/carlos/Documents/bake/source/ns-3.32/scratch/ns3-bsc-thesis/img/pc.svg";
string switchPath = "/home/carlos/Documents/bake/source/ns-3.32/scratch/ns3-bsc-thesis/img/eth.svg";
string managerPath = "/home/carlos/Documents/bake/source/ns-3.32/scratch/ns3-bsc-thesis/img/manager.svg";

void errorSettings(const double errorRate) {
    Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (errorRate));
    Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

    Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (1));
    Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));
}

template <class T>
void addApplicationToNodes(Ptr<T>* apps, NodeContainer nodes, uint32_t beginFrom, double errorRate) {
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        apps[i] = Create<T>(i + beginFrom, errorRate);
        nodes.Get(i)->AddApplication(apps[i]);
    }
}

void confNodes(AnimationInterface& anim,
                 NodeContainer& nodeContainer,
                 string path,
                 string desc,
                 int x,
                 int y,
                 int xDistance = 0,
                 int yDistance = 0) {
    Ptr<Node> node;
    for (uint32_t i = 0; i < nodeContainer.GetN(); i++) {
        node = nodeContainer.Get(i);
        anim.SetConstantPosition(node, x, y);  //set position of nodes in the animaiton
        anim.UpdateNodeSize(node->GetId(), 10, 10);
        anim.UpdateNodeImage(node->GetId(), anim.AddResource(path));
        anim.UpdateNodeDescription(node->GetId(), desc + ":" + to_string(i));
        x += xDistance;
        y += yDistance;
    }
}

void lineTopology(const uint32_t userNumbers, const uint32_t switchNumbers, const uint32_t managerNumbers, const double errorRate) {

    errorSettings(errorRate);

    NodeContainer user_nodes;
    NodeContainer switch_nodes;
    NodeContainer manager_nodes;

    NetDeviceContainer switchToSwitchContainer;
    NetDeviceContainer mangerToSwitchContainer;
    NetDeviceContainer userToSwitchContainer;

    manager_nodes.Create(managerNumbers);
    user_nodes.Create(userNumbers);
    switch_nodes.Create(switchNumbers);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    
    // Connect switches to switches and to the manager
    for (uint32_t i = 0; i < switchNumbers - 1; i++) {
        switchToSwitchContainer.Add(p2p.Install(switch_nodes.Get(i), switch_nodes.Get(i + 1)));
        mangerToSwitchContainer.Add(p2p.Install(switch_nodes.Get(i), manager_nodes.Get(0)));
    }
    mangerToSwitchContainer.Add(p2p.Install(switch_nodes.Get(switchNumbers - 1), manager_nodes.Get(0)));

    //Connect users to switches
    userToSwitchContainer.Add(p2p.Install(user_nodes.Get(0), switch_nodes.Get(0)));
    userToSwitchContainer.Add(p2p.Install(user_nodes.Get(1), switch_nodes.Get(switchNumbers - 1)));

    Ptr <User> user_apps[userNumbers];
    Ptr <EthSwitch> switch_apps[switchNumbers];
    Ptr <Manager> manager_apps[managerNumbers];
    int ethId = 10;
    int managerId = 100;

    addApplicationToNodes<User>(user_apps, user_nodes, 0, 0);
    addApplicationToNodes<EthSwitch>(switch_apps, switch_nodes, ethId, 0);
    addApplicationToNodes<Manager>(manager_apps, manager_nodes, managerId, 0);

    // Add error model (packet loss)
    ObjectFactory factory;
    factory.SetTypeId("ns3::RateErrorModel");
    Ptr<ErrorModel> em = factory.Create<ErrorModel>();

    for (uint32_t i = 0; i < switchToSwitchContainer.GetN(); i++) {
        switchToSwitchContainer.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }
    for (uint32_t i = 0; i < mangerToSwitchContainer.GetN(); i++) {
        mangerToSwitchContainer.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }
    for (uint32_t i = 0; i < userToSwitchContainer.GetN(); i++) {
        userToSwitchContainer.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }

    p2p.EnablePcapAll ("myNetworkPcaps");

    // CONSTELLATION
    //---------------------------------------------------------------------------------------------------------------------------------------
    AnimationInterface anim("topology_bcs.xml");
    anim.EnablePacketMetadata(true);

    int distance = 20;

    confNodes(anim, user_nodes, userPath, "User" + to_string(0), 0, 0, 100);
    confNodes(anim, switch_nodes, switchPath, "Switch" + to_string(ethId), 10, 50, distance);
    confNodes(anim, manager_nodes, managerPath, "Manager" + to_string(managerId), 50, 0);

    // SCHEDULE
    //---------------------------------------------------------------------------------------------------------------------------------------
    auto stopTime = Seconds(20);

    for (uint32_t i = 0; i < switch_nodes.GetN(); i++) {
        switch_apps[i]->SetStartTime(Seconds(1));
    }
    for (uint32_t i = 0; i < user_nodes.GetN(); i++) {
        user_apps[i]->SetStartTime(Seconds(2));
    }
    Simulator::Schedule(Seconds(3), &User::subscribe, user_apps[0], 1);

    for (int i = 0; i < 10; i++) {
        Simulator::Schedule(Seconds(3), &User::pushLogToSwitch, user_apps[1]);
    }


    Simulator::Stop(Seconds(10));
    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation

}

void p2pTopology(const double errorRate) {
    errorSettings(errorRate);

    uint32_t switchNumbers = 1;
    uint32_t managerNumbers = 1;

    NodeContainer switch_nodes;
    switch_nodes.Create(switchNumbers);
    NodeContainer manager_nodes;
    manager_nodes.Create(managerNumbers);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer device_container = p2p.Install(switch_nodes.Get(switchNumbers - 1), manager_nodes.Get(0));

    Ptr <EthSwitch> switch_apps[switchNumbers];
    Ptr <Manager> manager_apps[managerNumbers];
    int ethId = 10;
    int managerId = 100;

    addApplicationToNodes<EthSwitch>(switch_apps, switch_nodes, ethId, 0);
    addApplicationToNodes<Manager>(manager_apps, manager_nodes, managerId, 0);

    ObjectFactory factory;
    factory.SetTypeId("ns3::RateErrorModel");
    Ptr<ErrorModel> em = factory.Create<ErrorModel>();
    device_container.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    p2p.EnablePcapAll ("p2pTopologyPCAPS");

    // CONSTELLATION
    //---------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------
    AnimationInterface anim("p2pTopology.xml");
    anim.EnablePacketMetadata(true);

    int distance = 20;
    int x =  - ((distance * switchNumbers) / 2);
    int y = 50;
    confNodes(anim, switch_nodes, switchPath, "Switch" + to_string(ethId), x, y, distance);
    confNodes(anim, manager_nodes, managerPath, "Manager" + to_string(managerId), 0, 0);


    switch_apps[0]->SetStartTime(Seconds(1));
    switch_apps[0]->SetStopTime(Seconds(20));

    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation

}


int main(int argc, char *argv[]) {

    CommandLine cmd;
    string topology = "";
    cmd.AddValue("topo", "Test topology", topology);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    Packet::EnablePrinting();
    Packet::EnableChecking();

    if (topology == "line") {
        lineTopology(2, 5, 1, 0.001);
    } else if (topology == "p2p") {
        p2pTopology(0);
    }
    return 0;
}
