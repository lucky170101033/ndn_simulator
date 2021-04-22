#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

void PrintInfo(Ptr<Node> node){
  long long cache_hit,cache_miss;
  auto l3 = ns3::ndn::L3Protocol::getL3Protocol(node);
  auto fw =  l3-> getForwarder();
  std::tie (cache_hit,cache_miss) = fw->getCacheStats();
  std::cout << ns3::Names::FindName(node) << " " << cache_hit << " "  << cache_miss << '\n';
}

/**
 * This scenario simulates a tree topology (using topology reader module)
 *
 *    /------\      /------\      /------\      /------\
 *    |leaf-1|      |leaf-2|      |leaf-3|      |leaf-4|
 *    \------/      \------/      \------/      \------/
 *         ^          ^                ^           ^
 *         |          |                |           |
 *          \        /                  \         /
 *           \      /                    \       /    10Mbps / 1ms
 *            \    /                      \     /
 *             |  |                        |   |
 *             v  v                        v   v
 *          /-------\                    /-------\
 *          | rtr-1 |                    | rtr-2 |
 *          \-------/                    \-------/
 *                ^                        ^
 *                |                        |
 *                 \                      /  10 Mpbs / 1ms
 *                  +--------+  +--------+
 *                           |  |
 *                           v  v
 *                        /--------\
 *                        |  root  |
 *                        \--------/
 *
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     ./waf --run=ndn-tree-cs-tracers
 */

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-tree.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(100);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumers[4] = {Names::Find<Node>("leaf-1"), Names::Find<Node>("leaf-2"),
                            Names::Find<Node>("leaf-3"), Names::Find<Node>("leaf-4")};
  Ptr<Node> producer = Names::Find<Node>("root");

  for (int i = 0; i < 4; i++) {
    // ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
    consumerHelper.SetAttribute("Frequency", StringValue("20")); // 100 interests a second
    consumerHelper.SetAttribute("NumberOfContents", StringValue("50")); // 10 different contents

    // Each consumer will express the same data /root/<seq-no>
    consumerHelper.SetPrefix("/root");
    ApplicationContainer app = consumerHelper.Install(consumers[i]);
    app.Start(Seconds(0.01 * i));
  }

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  // Register /root prefix with global routing controller and
  // install producer that will satisfy Interests in /root namespace
  ndnGlobalRoutingHelper.AddOrigins("/root", producer);
  producerHelper.SetPrefix("/root");
  producerHelper.Install(producer);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  auto nc = NodeContainer::GetGlobal();
  for (uint32_t i = 0; i < nc.GetN(); i++) {
    Simulator::Schedule ( Seconds(20), PrintInfo, nc.Get(i) );
  }

  Simulator::Stop(Seconds(20.0));

  //ndn::CsTracer::InstallAll("cs-trace-tree.txt", Seconds(20));
  ndn::AppDelayTracer::InstallAll("results/app-delays-trace-tree-EDGE-20-50.txt");

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
