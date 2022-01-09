#include "Config.h"
#include "User.h"
#include "EthSwitch.h"
#include "Manager.h"
#include "../../build/ns3/node-container.h"
#include "../../build/ns3/point-to-point-helper.h"

using namespace ns3;
using namespace std;

string userPath = "/home/carlos/Documents/bake/source/ns-3.32/scratch/ns3-bsc-thesis/img/pc.svg";
string switchPath = "/home/carlos/Documents/bake/source/ns-3.32/scratch/ns3-bsc-thesis/img/eth.svg";
string managerPath = "/home/carlos/Documents/bake/source/ns-3.32/scratch/ns3-bsc-thesis/img/manager.svg";

int ethId = 10;
int managerId = 100;
int c = 0;
std::vector<std::string> files {
    "./scratch/ns3-bsc-thesis/keys/1",
    "./scratch/ns3-bsc-thesis/keys/2",
    "./scratch/ns3-bsc-thesis/keys/3",
    "./scratch/ns3-bsc-thesis/keys/4",
    "./scratch/ns3-bsc-thesis/keys/5",
    };

std::string readFile(std::string path) {
    string line;
    string output = "";
    ifstream myfile (path);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            cout << line << endl;
            output += line;
        }
        myfile.close();
    }
    return output;
}
void errorSettings(const double errorRate) {
    Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (errorRate));
    Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

//    Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=4]"));
}

template <class T>
void addApplicationToNodes(Ptr<T>* apps, NodeContainer nodes, uint32_t beginFrom, double gossipInterval) {


    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        auto privKey = readFile(files.at(c) + "/priv.txt");
        auto pubKey = readFile(files.at(c) + "/pub.txt");
        apps[i] = Create<T>(pubKey, gossipInterval, privKey);
        nodes.Get(i)->AddApplication(apps[i]);
        c++;
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

void enablePacketLoss(Ptr<ErrorModel> em) {
    em->Enable();
}

void disablePacketLoss(Ptr<ErrorModel> em) {
    em->Disable();
}

void lineTopologyInterface(Ptr<User> *user_apps,
                           Ptr<EthSwitch> *switch_apps,
                           Ptr<Manager> *manager_apps,
                           NodeContainer &user_nodes,
                           NodeContainer &switch_nodes,
                           NodeContainer &manager_nodes,
                           PointToPointHelper &p2p,
                           Ptr<ErrorModel> em)
                            {
    p2p.EnablePcapAll ("myNetworkPcaps");
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    double SWITCH_JOINING = 1;
    double USER_JOINING = 2;
    double ENABLE_ERROR_RATE = 2.5;
    double USER0_SUBSCRIBING_USER1 = 5;
    double USER1_PUSHING_START = 5;
//    double STOP_PUSHING = 15;
    double STOP_SIMULATION = 10;
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////

    // CONSTELLATION
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    AnimationInterface anim("hat-topology.xml");
    anim.EnablePacketMetadata(true);

    int distance = 100 / ((switch_nodes.GetN() * user_nodes.GetN()) - 1);

    confNodes(anim, user_nodes, userPath, "User" + to_string(0), 0, 50, 100);
    confNodes(anim, switch_nodes, switchPath, "Switch" + to_string(ethId), distance, 50, distance);
    confNodes(anim, manager_nodes, managerPath, "Manager" + to_string(managerId), 50, 0);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // SCHEDULE
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for (uint32_t i = 0; i < switch_nodes.GetN(); i++) {
        switch_apps[i]->SetStartTime(Seconds(SWITCH_JOINING));
    }
    for (uint32_t i = 0; i < user_nodes.GetN(); i++) {
        user_apps[i]->SetStartTime(Seconds(USER_JOINING));
    }
    Simulator::Schedule(Seconds(ENABLE_ERROR_RATE), &enablePacketLoss, em);

    Simulator::Schedule(Seconds(USER0_SUBSCRIBING_USER1), &User::subscribe, user_apps[0], "user:1");

    int packetNum = 10;
    int i = 0;
      while (packetNum > i) {
        Simulator::Schedule(Seconds(USER1_PUSHING_START), &User::pushLogToSwitch, user_apps[1]);
        USER1_PUSHING_START += 0.01;
        i++;
    }
//    Simulator::Schedule(Seconds(6), &User::unsubscribe, user_apps[0], "user:1");
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Simulator::Stop(Seconds(STOP_SIMULATION));
    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation

    }

void nodesSetup(const uint32_t userNumbers,
                const uint32_t switchNumbers,
                const uint32_t managerNumbers,
                const double errorRate,
                const double gossipInterval)
                  {
    errorSettings(errorRate);

    // Create node containers
    NodeContainer user_nodes;
    NodeContainer switch_nodes;
    NodeContainer manager_nodes;

    // Create containers for the links between the nodes ///////////////////////////////////////////////////////////////
    NetDeviceContainer switchToSwitchContainer;
    NetDeviceContainer mangerToSwitchContainer;
    NetDeviceContainer userToSwitchContainer;

    manager_nodes.Create(managerNumbers);
    user_nodes.Create(userNumbers);
    switch_nodes.Create(switchNumbers);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    
    // Connect switches to switches and to the manager /////////////////////////////////////////////////////////////////
    for (uint32_t i = 0; i < switchNumbers - 1; i++) {
        switchToSwitchContainer.Add(p2p.Install(switch_nodes.Get(i), switch_nodes.Get(i + 1)));
        mangerToSwitchContainer.Add(p2p.Install(switch_nodes.Get(i), manager_nodes.Get(0)));
    }
    mangerToSwitchContainer.Add(p2p.Install(switch_nodes.Get(switchNumbers - 1), manager_nodes.Get(0)));

    //Connect users to switches ////////////////////////////////////////////////////////////////////////////////////////
    userToSwitchContainer.Add(p2p.Install(user_nodes.Get(0), switch_nodes.Get(0)));
    userToSwitchContainer.Add(p2p.Install(user_nodes.Get(1), switch_nodes.Get(switchNumbers - 1)));

    // Add the applications to the devices /////////////////////////////////////////////////////////////////////////////
    Ptr <User> user_apps[userNumbers];
    Ptr <EthSwitch> switch_apps[switchNumbers];
    Ptr <Manager> manager_apps[managerNumbers];


    addApplicationToNodes<User>(user_apps, user_nodes, 0, gossipInterval);
    addApplicationToNodes<EthSwitch>(switch_apps, switch_nodes, ethId, gossipInterval);
    addApplicationToNodes<Manager>(manager_apps, manager_nodes, managerId, gossipInterval);

    // Add error model (packet loss) ///////////////////////////////////////////////////////////////////////////////////
    ObjectFactory factory;
    factory.SetTypeId("ns3::RateErrorModel");
    Ptr<ErrorModel> em = factory.Create<ErrorModel>();
    em->Disable();

    // Add error model for switch to switch links
    for (uint32_t i = 0; i < switchToSwitchContainer.GetN(); i++) {
        switchToSwitchContainer.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }
/*    for (uint32_t i = 0; i < mangerToSwitchContainer.GetN(); i++) {
        mangerToSwitchContainer.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }*/
    // Add error model for user to switch links
    for (uint32_t i = 0; i < userToSwitchContainer.GetN(); i++) {
//        userToSwitchContainer.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }

    lineTopologyInterface(user_apps, switch_apps, manager_apps, user_nodes, switch_nodes, manager_nodes, p2p, em);

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
    switch_apps[0]->SetStopTime(Seconds(100));

    Simulator::Run(); //run simulation
    Simulator::Destroy(); //end simulation

}


int main(int argc, char *argv[]) {

    CommandLine cmd;
    string topology = "";
    double gossip = 1;
    double errorRate = 0;
    cmd.AddValue("topo", "Test topology", topology);
    cmd.AddValue("gossip", "Gossip time interval", gossip);
    cmd.AddValue("errorRate", "Error rate", errorRate);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    Packet::EnablePrinting();
    Packet::EnableChecking();

    if (topology == "line") {
        nodesSetup(2, 2, 1, errorRate, gossip);
    } else if (topology == "p2p") {
        p2pTopology(0);
    }
    return 0;
}
