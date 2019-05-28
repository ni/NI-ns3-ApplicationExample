/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 National Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Vincent Kotzsch <vincent.kotzsch@ni.com>
 *         Clemens Felber <clemens.felber@ni.com>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/csma-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/config-store.h"

// NI includes
#include "ns3/ni-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NiLteSimple");

int
main (int argc, char *argv[])
{

  bool verbose = false;

   // enable native ns-3 logging - note: can also be used if compiled in "debug" mode
   if (verbose)
     {
       LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
       LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
       LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
       LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
       LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
       LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
       //LogComponentEnable ("ArpL3Protocol", LOG_LEVEL_LOGIC);
       LogComponentEnable ("EpcSgwPgwApplication", LOG_LEVEL_DEBUG);
       LogComponentEnable ("EpcEnbApplication", LOG_LEVEL_DEBUG);
       LogComponentEnable ("LteEnbNetDevice", LOG_LEVEL_DEBUG);
       LogComponentEnable ("LteEnbRrc", LOG_LEVEL_DEBUG);
       LogComponentEnable ("LtePdcp", LOG_LEVEL_DEBUG);

       LogComponentEnable ("RrFfMacScheduler", LOG_LEVEL_DEBUG);
       LogComponentEnable ("LteEnbMac", LOG_LEVEL_DEBUG);
       LogComponentEnable ("LteEnbPhy", LOG_LEVEL_DEBUG);
       LogComponentEnable ("NiLtePhyInterface", LOG_LEVEL_DEBUG);
     }

   // Simulation time in seconds
   double simTime    = 10;
   // Starting time in seconds for packet transmission for ns-3 generated packets
   double transmTime = 5;

   // Time between packet generation events at client app in milliseconds
   double   packetInterval = 500;
   // Number of ns-3 generated packets
   uint32_t packetNum      = 5;
   // Packet sizes for ns-3 generated packets
   uint32_t packetSize     = 1000;

   // number of station nodes in infrastructure mode
   uint32_t nLteUeNodes = 1;

   // ========================
   // general NI API parameter

   // Activate logging using NI API log files
   bool niApiEnableLogging = true;
   // Set log file names
   std::string LogFileName;
   // Enable remote control engine
   bool niRemoteControlEnable = false;
   // Enable TapBridge as data source and data sink
   bool niApiEnableTapBridge = false;
   // chose whether ns-3 instance should run as NIAPI_BS or NIAPI_TS, NIAPI_BSTS used for simulation mode
   std::string niApiDevMode = "NIAPI_BSTS";

   // ====================
   // NI API LTE parameter

  // Choose between NIAPI_eNB or NIAPI_UE mode, NIAPI_ALL used for simulation mode
  std::string niApiLteDevMode = "NIAPI_ALL";
  // Activate NIAPI for LTE
  bool niApiLteEnabled = false;
  // Activate NIAPI loopback mode for LTE
  bool niApiLteLoopbackEnabled = false; // true UDP, false Pipes
  // sinr value in db used for cqi calculation for the ni phy
  double niChSinrValueDb = 10;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("numPackets", "number of packets generated", packetNum);
  cmd.AddValue("interval", "interval (milliseconds) between packets", packetInterval);
  cmd.AddValue("simTime", "Duration in seconds which the simulation should run", simTime);
  cmd.AddValue("transmTime", "Time in seconds when the packet transmission should be scheduled", transmTime);
  cmd.AddValue("niApiEnableTapBridge", "Enable/disable TapBridge as external data source/sink", niApiEnableTapBridge);
  cmd.AddValue("niApiEnableLogging", "Set whether to enable NIAPI_DebugLogs", niApiEnableLogging);
  cmd.AddValue("niApiDevMode", "Set whether the simulation should run as BS or Terminal", niApiDevMode);
  cmd.AddValue("niApiLteEnabled", "Enable NI API for LTE", niApiLteEnabled);
  cmd.AddValue("niApiLteLoopbackEnabled", "Enable/disable UDP loopback mode for LTE NI API", niApiLteLoopbackEnabled);
  cmd.Parse(argc, argv);

  // Activate the ns-3 real time simulator
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  // Set real time mode (need RT preempt patch?) -> real time mode with synchronization mode set to Hard Limit does not work properly?
  Config::SetDefault ("ns3::RealtimeSimulatorImpl::SynchronizationMode", StringValue("BestEffort")); //BestEffort
  //Config::SetDefault ("ns3::RealtimeSimulatorImpl::HardLimit", StringValue("+100000000.0ns"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true)); // needed for tapBridge

  // ============================
  // General NI API Configuration

  // write different log files for each stations
  std::string simStationType = "default";
  if (niApiDevMode == "NIAPI_BS"){
      // if ns-3 instance is configured as base station it acts as eNB / AP
      niApiLteDevMode  = "NIAPI_ENB";
      simStationType   = "BS";
  } else if (niApiDevMode == "NIAPI_TS") {
      // if ns-3 instance is configured as base station it acts as UE / STA
      niApiLteDevMode  = "NIAPI_UE";
      simStationType   = "TS";
  } else if (niApiDevMode == "NIAPI_BSTS") {
      // ns-3 instance is configures as BS and TS- for debugging if only spectrum phy shall be bypassed
      niApiLteDevMode  = "NIAPI_ALL";
      simStationType   = "BSTS";
  } else {
      NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed");
  }

  // print ouf config parameters
  // TODO-NI: replace cout by NI_LOG_CONSOLE_INFO
  std::cout << std::endl;
  std::cout << "-------- NS-3 Configuration -------------" << std::endl;

  std::cout << "Running LTE node as:  ";
  if (niApiLteDevMode == "NIAPI_ALL") std::cout << "ENB and UE" << std::endl;
  else if (niApiLteDevMode == "NIAPI_ENB")  std::cout << "ENB" << std::endl;
  else if (niApiLteDevMode == "NIAPI_UE")   std::cout << "UE" << std::endl;

  std::cout << "LTE API:              ";
  if (niApiLteEnabled == true) std::cout << "enabled" << std::endl;
  else                         std::cout << "disabled" << std::endl;

  std::cout << "LTE UDP Loopback:     ";
  if (niApiLteLoopbackEnabled == true) std::cout << "enabled" << std::endl;
  else                                 std::cout << "disabled" << std::endl;

  std::cout << "TapBridge:            ";
  if (niApiEnableTapBridge == true) std::cout << "enabled" << std::endl;
  else                              std::cout << "disabled" << std::endl;

  std::cout << "Logging:              ";
  if (niApiEnableLogging == true) std::cout << "enabled" << std::endl;
  else                            std::cout << "disabled" << std::endl;

  std::cout << "NI Module Version:    " << NI_MODULE_VERSION << std::endl;
  std::cout << "Required AFW Version: " << NI_AFW_VERSION << std::endl;

  std::cout << std::endl;

  NI_LOG_CONSOLE_INFO ("Init NI modules ...");
  // deriving thread priorities from ns3 main process
  int ns3Priority = NiUtils::GetThreadPrioriy();
  const int niLoggingPriority = ns3Priority - 10;
  const int niRemoteControlPriority = ns3Priority - 11;

  // adding thread ID of main NS3 thread for possible troubleshooting
  NiUtils::AddThreadInfo(pthread_self(), "NS3 main thread");

  // install signal handlers in order to print debug information to std::out in case of an error
  NiUtils::InstallSignalHandler();

  if (niApiEnableLogging){
      // Init ni real time logging - can also be used if compiled in "optimized" mode
      LogFileName = "/tmp/Log_Lte_" + simStationType + ".txt";
      NiLoggingInit(LOG__LEVEL_WARN | LOG__CONSOLE_DEBUG, LogFileName, NI_LOG__INSTANT_WRITE_DISABLE, niLoggingPriority);
  }

  // Start RemoteControlEngine
  // use globally defined instance in RemoteControlÈngine class
  if (niRemoteControlEnable)
    {
      g_RemoteControlEngine.Initialize("ns3", 1500, niRemoteControlPriority);
    }

  // ==========================
  // configure helper functions

  // csma helper
  CsmaHelper csmaHelp;
             csmaHelp.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
             csmaHelp.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // p2p helper
  PointToPointHelper p2pHelp;
                     p2pHelp.SetDeviceAttribute ("DataRate", StringValue ("100Gbps"));
                     p2pHelp.SetChannelAttribute ("Delay", StringValue ("2ms"));
                     p2pHelp.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  MobilityHelper MobilityHelp;
                 MobilityHelp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // ip helper
  InternetStackHelper     ipStackHelp;
  Ipv4AddressHelper       ipAddressHelp;
  Ipv4Mask                IpMask = "255.255.255.0";
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // main router node to connect lan with lte mobile network
   Ptr<Node> MobileNetworkGwNode = CreateObject<Node> ();
   // lan nodes connected via ethernet - added mobile network gateway here
   NodeContainer LanNodes;
                 LanNodes.Add (MobileNetworkGwNode);
                 LanNodes.Create (2);
   // lte user terminal nodes
   NodeContainer ueNodes;
                 ueNodes.Create(nLteUeNodes);
   // lte enb nodes - here fixed to one but can be extended
   NodeContainer enbNodes;
                 enbNodes.Create(1);

   // install corresponding net devices on all nodes
   NetDeviceContainer csmaDevices = csmaHelp.Install (LanNodes);

   // install ip stacks on all nodes
   ipStackHelp.Install (LanNodes);
   ipStackHelp.Install (ueNodes);

   // configure ip address spaces and gateways for the different subnetworks
   Ipv4Address csmaIpSubnet    = "10.1.1.0";
   Ipv4Address p2pLteIpSubnet  = "10.1.2.0";
   Ipv4Address UeIpSubnet      = "7.0.0.0";

   // create lan network with stations - sta0 is mobile network gateway
   ipAddressHelp.SetBase (csmaIpSubnet, IpMask);
   Ipv4InterfaceContainer LanIpInterfaces = ipAddressHelp.Assign (csmaDevices);
   Ipv4Address LanGwIpAddr = LanIpInterfaces.GetAddress (0);//"10.1.1.1";

   // initialize routing database and set up the routing tables in the nodes
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   // ==========================
   // setup the lte network - note that lte is configured after "PopulateRoutingTables" as static routing is used for epc

   // Set the device type mode for the ni phy applied for this ns-3 instance
   Config::SetDefault ("ns3::NiLtePhyInterface::niApiDevType", StringValue (niApiLteDevMode));
   // Enable / disable the use of ni api for the ni phy
   Config::SetDefault ("ns3::NiLtePhyInterface::enableNiApi", BooleanValue (niApiLteEnabled));
   // Enable / disable the use of ni api udp loopback mode for the ni phy
   Config::SetDefault ("ns3::NiLtePhyInterface::enableNiApiLoopback", BooleanValue (niApiLteLoopbackEnabled));
   // Set the default channel sinr value in db used for cqi calculation for the ni phy
   Config::SetDefault ("ns3::NiLtePhyInterface::niChSinrValueDb", DoubleValue (niChSinrValueDb));
   // Set the CQI report period for the ni phy
   Config::SetDefault ("ns3::NiLtePhyInterface::niCqiReportPeriodMs", UintegerValue (100));

   // Set downlink transmission bandwidth in number of resource blocks -> set to 20MHz default here
   Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (100));
   // Disable ideal and use real RRC protocol with RRC PDUs (cf https://www.nsnam.org/docs/models/html/lte-design.html#real-rrc-protocol-model)
   Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
   // Disable CQI measurement reports based on PDSCH (based on ns-3 interference model)
   Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (false));
   // Use simple round robin scheduler here (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
   Config::SetDefault ("ns3::LteHelper::Scheduler", StringValue ("ns3::RrFfMacScheduler"));
   // Set the adpative coding and modulation model to Piro as this is the simpler one
   Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
   // Disable HARQ as this not support by PHY and not implemented in NI API
   Config::SetDefault ("ns3::RrFfMacScheduler::HarqEnabled", BooleanValue (false));
   // Set CQI timer threshold in sec - depends on CQI report frequency to be set in NiLtePhyInterface
   Config::SetDefault ("ns3::RrFfMacScheduler::CqiTimerThreshold", UintegerValue (1000));
   // Set the length of the window (in TTIs) for the reception of the random access response (RAR); the resulting RAR timeout is this value + 3 ms
   Config::SetDefault ("ns3::LteEnbMac::RaResponseWindowSize", UintegerValue(10));
   // Set ConnectionTimeoutDuration, after a RA attempt, if no RRC Connection Request is received before this time, the UE context is destroyed.
   // Must account for reception of RAR and transmission of RRC CONNECTION REQUEST over UL GRANT.
   //Config::SetDefault ("ns3::LteEnbRrc::ConnectionRequestTimeoutDuration", StringValue ("+60000000.0ns"));

   // update remote control database related parameters
   g_RemoteControlEngine.GetPdb()->setParameterManualLteUeChannelSinrEnable(false);
   g_RemoteControlEngine.GetPdb()->setParameterLteUeChannelSinr(niChSinrValueDb);

   // epc helper is used to create core network incl installation of eNB App
   Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
   // lte helper is used to install L2/L3 on eNB/UE (PHY,MAC,RRC->PDCP/RLC, eNB NetDevice)
   Ptr<LteHelper>              lteHelper = CreateObject<LteHelper> ();
   lteHelper->SetEpcHelper (epcHelper);

   // Use simple round robin scheduler here
   // (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
   lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

   Ptr<Node> PacketGwNode = epcHelper->GetPgwNode ();
   // create lte p2p link to mobile network gateway - work around for the global/static routing problem
   NetDeviceContainer p2pLteDevices = p2pHelp.Install (PacketGwNode, MobileNetworkGwNode);
   // assign ip adresses for p2p link
   ipAddressHelp.SetBase (p2pLteIpSubnet, IpMask);
   Ipv4InterfaceContainer p2pLteIpInterfaces = ipAddressHelp.Assign (p2pLteDevices);

   // lte channel / mobility  model
   MobilityHelp.Install (enbNodes);
   MobilityHelp.Install (ueNodes);

   // Install lte net devices to the nodes
   NetDeviceContainer enbDevices = lteHelper->InstallEnbDevice (enbNodes);
   NetDeviceContainer ueDevices  = lteHelper->InstallUeDevice (ueNodes);

   // Install the ip stack on the ue nodes
   Ipv4InterfaceContainer ueIpInterfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevices));

   // Set the default gateway for the lte ue nodes - will be used for all outgoing packets for this node
   for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
     {
       Ptr<Node> TmpNode = ueNodes.Get (u);
       Ptr<Ipv4StaticRouting> StaticRouting = ipv4RoutingHelper.GetStaticRouting (TmpNode->GetObject<Ipv4> ());
       StaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
     }

   // Attach one UE per eNodeB
   for (uint16_t i = 0; i < ueNodes.GetN (); i++)
     {
       lteHelper->Attach (ueDevices.Get(i), enbDevices.Get(0));
       // side effect: the default EPS bearer will be activated
     }

   // Set the default gateway for the lan stations
   for (uint32_t u = 1; u < LanNodes.GetN (); ++u)
     {
       Ptr<Node> TmpNode = LanNodes.Get (u);
       Ptr<Ipv4StaticRouting> StaticRouting = ipv4RoutingHelper.GetStaticRouting (TmpNode->GetObject<Ipv4> ());
       StaticRouting->SetDefaultRoute (Ipv4Address (LanGwIpAddr), 1);
       //StaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"),  Ipv4Address (LanGwIpAddr), 1);
     }

   // add route to lte network in lan mobile gateway
   Ptr<Ipv4StaticRouting> MobileNetworkGwNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (MobileNetworkGwNode->GetObject<Ipv4> ());
   MobileNetworkGwNodeStaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"), 2);
   // add route from lte network to lan network in lte packet gateway
   Ptr<Ipv4StaticRouting> pgwStaticRouting = ipv4RoutingHelper.GetStaticRouting (PacketGwNode->GetObject<Ipv4> ());
   pgwStaticRouting->AddNetworkRouteTo (Ipv4Address (csmaIpSubnet), Ipv4Mask (IpMask), 2);

   // ==========================
   // application config

   // user always LAN node as client
   NodeContainer ClientNode     = LanNodes.Get (1);
   Ipv4Address   ClientIpAddr   = LanIpInterfaces.GetAddress (1);
   //NodeContainer ClientNode     = ueNodes.Get (0);
   //Ipv4Address   ClientIpAddr   = ueIpInterfaces.GetAddress (0);
   NodeContainer ServerNode     = ueNodes.Get (0);
   Ipv4Address   ServerIPAddr   = ueIpInterfaces.GetAddress (0);
   //NodeContainer ServerNode     = LanNodes.Get (2);
   //Ipv4Address   ServerIPAddr   = LanIpInterfaces.GetAddress (2);

   // print addresses
   std::cout << std::endl;
   std::cout << "-------- NS-3 Topology Information ------" << std::endl;
   std::cout << "Number of ETH devices      = " << LanIpInterfaces.GetN() << std::endl;
   std::cout << "Number of LTE UE devices   = " << ueIpInterfaces.GetN() << std::endl;
   std::cout << "Router GW IP Addr          = " << LanGwIpAddr << std::endl;
   std::cout << "LTE Net GW IP Addr         = " << p2pLteIpInterfaces.GetAddress(1) << std::endl;
   std::cout << "LTE EPC PGW IP Addr        = " << PacketGwNode->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal () << std::endl;
   std::cout << "LTE UE#1 IP Addr           = " << ueIpInterfaces.GetAddress(0) << std::endl;
   std::cout << "Client IP Addr             = " << ClientIpAddr << std::endl;
   std::cout << "Server IP Addr             = " << ServerIPAddr << std::endl;
   std::cout << std::endl;

   Ipv4Address  ClientDestAddr  = ServerIPAddr;
   uint16_t     DestPort = 9;   // Discard port (RFC 863)
   // Convert to time object
   Time packetIntervalSec = Seconds(packetInterval*1E-03);

   Ptr<NiUdpServer> appServer;

   if (!niApiEnableTapBridge)
     {
       // undirectional traffic - https://www.nsnam.org/doxygen/classns3_1_1_udp_client.html
       NiUdpServerHelper     ServerHelp (DestPort);
       NiUdpClientHelper     ClientHelp(ClientDestAddr, DestPort);
       //ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ClientDestAddr));
       //ClientHelp.SetAttribute ("RemotePort", UintegerValue (DestPort));
       ClientHelp.SetAttribute ("MaxPackets", UintegerValue (packetNum));
       ClientHelp.SetAttribute ("Interval", TimeValue (packetIntervalSec));
       ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));

       if ((niApiDevMode == "NIAPI_BS")||(niApiDevMode == "NIAPI_BSTS")){
           ApplicationContainer clientApps = ClientHelp.Install (ClientNode);
           clientApps.Start (Seconds (transmTime));
           clientApps.Stop (Seconds (simTime));
       }

       ApplicationContainer serverApps = ServerHelp.Install (ServerNode);
       serverApps.Start (Seconds (1.0));
       serverApps.Stop (Seconds (simTime));

       appServer = ServerHelp.GetServer ();
     }
   else
     {
       if (niApiDevMode == "NIAPI_BS"){
           std::cout << "Install TAP bridge for BS host IP node..." << std::endl;
           enbDevices.Get (0)->SetAddress (Mac48Address::Allocate ());
           TapBridgeHelper tapBridgeENB;
           std::string modeENB = "ConfigureLocal";
           std::string tapNameENB = "NIAPI_TapENB";
           tapBridgeENB.SetAttribute ("Mode", StringValue (modeENB));
           tapBridgeENB.SetAttribute ("DeviceName", StringValue (tapNameENB));
           tapBridgeENB.SetAttribute ("Mtu", UintegerValue(1500) );
           tapBridgeENB.Install (LanNodes.Get(1), csmaDevices.Get(1));
       }
       else if(niApiDevMode == "NIAPI_TS"){
           //Tap Bridge for UE (LTE)
           std::cout <<"Install TAP bridge for UE host IP node..." <<std::endl;
           ueDevices.Get(0)->SetAddress(Mac48Address::Allocate());
           TapBridgeHelper tapBridgeUE(Ipv4Address("7.0.0.2"));
           std::string modeUE = "ConfigureLocal";
           std::string tapNameUE = "NIAPI_TapUE";
           tapBridgeUE.SetAttribute("Mode", StringValue(modeUE));
           tapBridgeUE.SetAttribute("DeviceName", StringValue(tapNameUE));
           tapBridgeUE.SetAttribute("Mtu", UintegerValue(1500));
           tapBridgeUE.Install(ueNodes.Get(0), ueDevices.Get(0));
       }
     }

   // Uncomment to enable PCAP tracing
   //p2ph.EnablePcapAll("lena-epc-first");

   // stop the simulation after simTime seconds
   Simulator::Stop(Seconds(simTime));

   if (niApiLteEnabled && !niApiLteLoopbackEnabled)
     {
       NI_LOG_CONSOLE_INFO ("\n--> Please enable now Rx/Tx in LTE Application Framework!");
     }

   std::cout << std::endl;
   std::cout << "[>] Start simulation" << std::endl;
   Simulator::Run ();
   std::cout << "[#] End simulation" << std::endl << std::endl;

   // check received packets
   if ((niApiDevMode == "NIAPI_TS")||(niApiDevMode == "NIAPI_BSTS"))
     {
       NI_LOG_CONSOLE_INFO ("Received packets: " << appServer->GetReceived()
                            << " / Lost packets: " << packetNum-appServer->GetReceived()
                            << "\n");
     }

   Simulator::Destroy ();

   // De-init NI modules
   NI_LOG_CONSOLE_INFO ("De-init NI modules ...");
   if (niApiEnableLogging)
     {
       NiLoggingDeInit();
     }
   // Close RemoteControlEngine
   if (niRemoteControlEnable)
     {
       g_RemoteControlEngine.Deinitialize();
     }

   NI_LOG_CONSOLE_INFO ("-------- Program end! -------------------");

  return 0;
}

