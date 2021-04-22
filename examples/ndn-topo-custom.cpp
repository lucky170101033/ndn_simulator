/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-tree-cs-tracers.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include <string.h>

namespace ns3 {

void PrintInfo(Ptr<Node> node){
  long long cache_hit,cache_miss;
  auto l3 = ns3::ndn::L3Protocol::getL3Protocol(node);
  auto fw =  l3-> getForwarder();
  std::tie (cache_hit,cache_miss) = fw->getCacheStats();
  std::cout << ns3::Names::FindName(node) << " " << cache_hit << " "  << cache_miss << '\n';
}

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/cluster-tree-topo.txt");
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
  Ptr<Node> consumers[48];
  int k = 0;
  for (int i = 0; i < 4; i++) {
      std::string t[4] = {"000","001","010","011"};
      for(int j = 0; j < 4; j++) {
        std::string t1[3] = {"0","1","1_"};
        for(int l = 0; l < 3; l++) {  
            std::string nodeName = "Node" + std::to_string(i+1) + t[j] + t1[l] ;
            consumers[k++] = Names::Find<Node>(nodeName);
        }
      }
  }
  Ptr<Node> producer = Names::Find<Node>("Producer1");

  for (int i = 0; i < 48; i++) {
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
    consumerHelper.SetAttribute("Frequency", StringValue("20"));        // 10 interests a second
    consumerHelper.SetAttribute("NumberOfContents", StringValue("250")); // 100 different contents

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

  //ndn::CsTracer::InstallAll("cs-trace.txt", Seconds(1));
  ndn::AppDelayTracer::InstallAll("results/app-delays-trace-LCE-20-250.txt");
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