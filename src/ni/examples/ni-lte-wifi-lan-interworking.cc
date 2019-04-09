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

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"

// LAN-WiFi-LTE Network Topology
//
//  Sta1 Sta2  Sta3
//   *    *     *           AP
//                          |
//   *    *   eNB-EPC-PGW-MobGW  n1   n2   n3
//  UE1  UE2                |    |    |    |
//                          ================
//                                LAN

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("lte_wifi_interworking");

void OwnSend(Ptr<Socket> socket1, Ptr<Socket> socket2, uint32_t pktSize,
             uint32_t pktCount, Time pktInterval)
{

   // Declaring sequence header count as STATIC since it has to be initialized only once i.e. @first time only.
  static uint32_t SeqHeaderCount = 0;

  if(pktCount>0)
    {
      // create packet for destination address #1
      Ptr<Packet> packet1;
      if (pktSize > 32)
        {
          // include real payload in the packet
          uint8_t buffer[pktSize];
          for (uint32_t i = 0; i < pktSize; i++) buffer[i] = rand() % 0xff;
          packet1 = Create<Packet> (buffer  , pktSize );

        }
      else
        {
          // send zero filled packet
          packet1 = Create<Packet> (pktSize);
        }

      // create and add sequence header
      SeqTsHeader seqTs1;
      seqTs1.SetSeq (++SeqHeaderCount);
      packet1->AddHeader (seqTs1);

      // send packet to destination address #1
      if ((socket1->Send (packet1)) >= 0)
        {
          // get destination IP address of socket for debug print out
          Address addr;
          socket1->GetPeerName (addr);
          InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr);

          NI_LOG_CONSOLE_DEBUG ("NI.CLIENT: sent " << pktSize
                                << " bytes to " << iaddr.GetIpv4 ()
                                << " Uid: " << packet1->GetUid ()
                                << " Sequence number: " << seqTs1.GetSeq ()
                                << " Time: " << (Simulator::Now ()).GetSeconds ());
        }
      else
        {
          NI_LOG_CONSOLE_DEBUG ("NI.CLIENT: Error while sending " << pktSize << std::endl);
        }

      if (socket2!=0){
          // create packet for destination address #2
          Ptr<Packet> packet2;
          if (pktSize > 32)
            {
              // include real payload in the packet
              uint8_t buffer[pktSize];
              for (uint32_t i = 0; i < pktSize; i++) buffer[i] = rand() % 0xff;
              packet2 = Create<Packet> (buffer, pktSize);

            }
          else
            {
              // send zero filled packet
              packet2 = Create<Packet> (pktSize);
            }

          // create and add sequence header
          SeqTsHeader seqTs2;
          seqTs2.SetSeq (++SeqHeaderCount);
          packet2->AddHeader (seqTs2);

          // send packet to destination address #2
          if ((socket2->Send (packet2)) >= 0)
            {
              // get destination IP address of socket for debug print out
              Address addr;
              socket2->GetPeerName (addr);
              InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr);

              NI_LOG_CONSOLE_DEBUG ("NI.CLIENT: sent " << pktSize
                                    << " bytes to " << iaddr.GetIpv4 ()
                                    << " Uid: " << packet2->GetUid ()
                                    << " Sequence number: " << seqTs2.GetSeq ()
                                    << " Time: " << (Simulator::Now ()).GetSeconds ());
            }
          else
            {
              NI_LOG_CONSOLE_DEBUG ("NI.CLIENT: Error while sending " << pktSize << std::endl);
            }
      }

      // schedule next packet generation event based on packet interval time
      Simulator::Schedule (pktInterval, &OwnSend, socket1, socket2, pktSize, pktCount-1, pktInterval);

    }
}

void OwnStartApplication(NodeContainer SourceN, Ipv4Address Dest1, Ipv4Address Dest2, uint16_t DestinationPort,
                         uint32_t packetSize, uint32_t numPackets, Time TimeStarter , Time intPktInterval)
{
  // define two sockets for two destination adresses
  Ptr<Socket> m_socket1;
  Ptr<Socket> m_socket2;

  // Getting the pointer to node from the incoming client node container
  Ptr<Node> SourceNode  = SourceN.Get(0);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  m_socket1 = Socket::CreateSocket ( SourceNode, tid);
  m_socket1->Bind ();
  // connecting the sockets to destination address and destination port.
  m_socket1->Connect (InetSocketAddress (Dest1, DestinationPort));
  // set null callbacks
  m_socket1->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket1->SetAllowBroadcast (true);
  //NI_LOG_CONSOLE_DEBUG ("Create UDP source socket #1");

  // configure 2nd socket
  if (Dest2.IsEqual("0.0.0.0")){
      m_socket2=0;
  }
  else{
      m_socket2 = Socket::CreateSocket (SourceNode, tid);
      m_socket2->Bind ();
      m_socket2->Connect (InetSocketAddress (Dest2, DestinationPort));
      m_socket2->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket2->SetAllowBroadcast (true);
      //NI_LOG_CONSOLE_DEBUG ("Create UDP source socket #2");
  }

  // schedule first call of own_send function with socket parameters
  Simulator::Schedule(TimeStarter, &OwnSend, m_socket1, m_socket2, packetSize, numPackets, intPktInterval);
}


int 
main (int argc, char *argv[])
{
  bool tracing = true;
  bool verbose = true;

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);

      std::cout <<"\nNI logging enabled" <<std::endl;
      // Init ni real time logging - can also be used if compiled in "optimized" mode
      // install signal handlers in order to print debug information to std::out in case of an error
      NiUtils::InstallSignalHandler();
      int ns3priority = NiUtils::GetThreadPrioriy();
      int niLoggingPriority = ns3priority - 10;
      std::string LogFileName = "/tmp/Log_LteWifi_interworking.txt";
      NiLoggingInit(LOG__LEVEL_WARN | LOG__CONSOLE_DEBUG, LogFileName, NI_LOG__INSTANT_WRITE_DISABLE, niLoggingPriority);
      // adding thread ID of main NS3 thread for possible troubleshooting
      NiUtils::AddThreadInfo(pthread_self(), "NS3 main thread");
    }


  // define client server configuration
  int clientAppConfig    = 3;
  // define client server configuration
  int cientServerConfig  = 2;

  uint32_t nLanNodes     = 3;
  uint32_t nWifiStaNodes = 1;
  uint16_t nLteUeNodes   = 1;

  uint32_t numPackets    = 2;
  uint32_t packetSize    = 1000;

  CommandLine cmd;
  cmd.AddValue ("nLanNodes", "Number of \"extra\" CSMA nodes/devices", nLanNodes);
  cmd.AddValue ("nWifiStaNodes", "Number of wifi STA devices", nWifiStaNodes);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifiStaNodes > 18)
    {
      std::cout << "nWifiStaNodes should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  Ptr<OutputStreamWrapper> testprint = Create<OutputStreamWrapper>("routingtestprint", std::ios::out);
  Ptr<Node> PrintRouteNode;
  Ptr<Ipv4StaticRouting> PrintRouteNodeStaticRouting;

  // p2p helper
  PointToPointHelper p2pHelp;
                     p2pHelp.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
                     p2pHelp.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // csma helper
  CsmaHelper csmaHelp;
             csmaHelp.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
             csmaHelp.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // wifi helper
  Ssid                  ssid = Ssid ("ns-3-ssid");
  WifiHelper            wifiHelp;
                        wifiHelp.SetStandard (WIFI_PHY_STANDARD_80211ac);
  //wifiHelp.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifiHelp.SetRemoteStationManager ("ns3::ConstantRateWifiManager");
  WifiMacHelper         wifiApMacHelp, wifiStaMacHelp;
                        wifiStaMacHelp.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
                        wifiApMacHelp.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  YansWifiChannelHelper wifiChannelHelp = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper     wifiPhyHelp = YansWifiPhyHelper::Default ();
                        wifiPhyHelp.SetChannel (wifiChannelHelp.Create ());
  MobilityHelper        MobilityHelp;
                        MobilityHelp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // ip helper
  InternetStackHelper     ipStackHelp;
  Ipv4AddressHelper       ipAddressHelp;
  Ipv4Mask                IpMask = "255.255.255.0";
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // create node containers

  // main router node to connect lan with wifi and lte mobile networks
  Ptr<Node> MobileNetworkGwNode = CreateObject<Node> ();
  // wifi station nodes
  NodeContainer wifiStaNodes;
                wifiStaNodes.Create (nWifiStaNodes);
  // wifi access point nodes - here fixed to one but can be extended
  NodeContainer wifiApNode;
                wifiApNode.Create (1);
  // lan nodes connected via ethernet - added mobile network gateway here
  NodeContainer LanNodes;
                LanNodes.Add (MobileNetworkGwNode);
                LanNodes.Create (nLanNodes);
  //LanNodes.Add (wifiApNode);
  // lte user terminal nodes
  NodeContainer ueNodes;
                ueNodes.Create(nLteUeNodes);
  // lte enb nodes - here fixed to one but can be extended
  NodeContainer enbNodes;
                enbNodes.Create(1);

  // install corresponding net devices on all nodes
  NetDeviceContainer p2pWiFiDevices  = p2pHelp.Install (MobileNetworkGwNode, wifiApNode.Get(0));
  NetDeviceContainer csmaDevices     = csmaHelp.Install (LanNodes);
  // note: all ue nodes have an lte and wifi net device
  NetDeviceContainer staDevices      = wifiHelp.Install (wifiPhyHelp, wifiStaMacHelp, NodeContainer(wifiStaNodes, ueNodes));
  //NetDeviceContainer staUeDevices    = wifiHelp.Install (wifiPhyHelp, wifiStaMacHelp, ueNodes);
  NetDeviceContainer apDevices       = wifiHelp.Install (wifiPhyHelp, wifiApMacHelp, wifiApNode);

  // install ip stacks on all nodes
  ipStackHelp.Install (LanNodes);
  ipStackHelp.Install (wifiStaNodes);
  ipStackHelp.Install (wifiApNode);
  ipStackHelp.Install (ueNodes);

  // configure ip address spaces and gateways for the different subnetworks
  Ipv4Address csmaIpSubnet    = "10.1.1.0";
  Ipv4Address WifiIpSubnet    = "10.1.2.0";
  Ipv4Address p2pWifiIpSubnet = "10.1.3.0";
  Ipv4Address p2pLteIpSubnet  = "10.1.4.0";
  Ipv4Address UeIpSubnet      = "7.0.0.0";

  // create wifi p2p link to mobile network gateway
  ipAddressHelp.SetBase (p2pWifiIpSubnet, IpMask);
  Ipv4InterfaceContainer p2pWifiIpInterfaces;
                         p2pWifiIpInterfaces = ipAddressHelp.Assign (p2pWiFiDevices);

  // create lan network with stations - sta0 is mobile network gateway
  ipAddressHelp.SetBase (csmaIpSubnet, IpMask);
  Ipv4InterfaceContainer LanIpInterfaces = ipAddressHelp.Assign (csmaDevices);
  Ipv4Address LanGwIpAddr = LanIpInterfaces.GetAddress (0);//"10.1.1.1";

  // create wifi network - ap0 is connected to mobile network gateway
  ipAddressHelp.SetBase (WifiIpSubnet, IpMask);
  Ipv4InterfaceContainer wifiIpInterfaces;
                         wifiIpInterfaces.Add (ipAddressHelp.Assign (apDevices));
                         wifiIpInterfaces.Add (ipAddressHelp.Assign (staDevices));
  //wifiIpInterfaces.Add (ipAddressHelp.Assign (staUeDevices));
  Ipv4Address WiFiGwIpAddr = wifiIpInterfaces.GetAddress (0);//"10.1.2.1";

  // initialize routing database and set up the routing tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // lte helper
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  Ptr<LteHelper>              lteHelper = CreateObject<LteHelper> ();
                              lteHelper->SetEpcHelper (epcHelper);
                              // Use simple round robin scheduler here
                              // (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
                              lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  // Set Downlink transmission bandwidth in number of Resource Blocks -> set to 20MHz default here
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (100));

  // setup the lte network - note that lte is configured after "PopulateRoutingTables" as static routing is used for epc

  Ptr<Node> PacketGwNode = epcHelper->GetPgwNode ();
  // create lte p2p link to mobile network gateway - work around for the global/static routing problem
  NetDeviceContainer p2pLteDevices = p2pHelp.Install (PacketGwNode, MobileNetworkGwNode);
  // assign ip adresses for p2p link
  ipAddressHelp.SetBase (p2pLteIpSubnet, IpMask);
  Ipv4InterfaceContainer p2pLteIpInterfaces = ipAddressHelp.Assign (p2pLteDevices);

  // lte & wifi channel / mobility  model
  MobilityHelp.Install (wifiStaNodes);
  MobilityHelp.Install (wifiApNode);
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
      StaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 2);
      //StaticRouting->SetDefaultRoute (Ipv4Address (WiFiGwIpAddr), 1);
      //StaticRouting->AddNetworkRouteTo (Ipv4Address (WifiIpSubnet), Ipv4Mask (IpMask),  Ipv4Address (WiFiGwIpAddr), 1);
    }

  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < ueNodes.GetN (); i++)
    {
      lteHelper->Attach (ueDevices.Get(i), enbDevices.Get(0));
      // side effect: the default EPS bearer will be activated
    }

  // Set the default gateway for the wifi stations
  for (uint32_t u = 0; u < wifiStaNodes.GetN (); ++u)
    {
      Ptr<Node> TmpNode = wifiStaNodes.Get (u);
      Ptr<Ipv4StaticRouting> StaticRouting = ipv4RoutingHelper.GetStaticRouting (TmpNode->GetObject<Ipv4> ());
      StaticRouting->SetDefaultRoute (Ipv4Address (WiFiGwIpAddr), 1);
      //StaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"),  Ipv4Address (WiFiGwIpAddr), 1);
    }

  // Set the default gateway for the lan stations
  for (uint32_t u = 1; u < LanNodes.GetN (); ++u)
    {
      Ptr<Node> TmpNode = LanNodes.Get (u);
      Ptr<Ipv4StaticRouting> StaticRouting = ipv4RoutingHelper.GetStaticRouting (TmpNode->GetObject<Ipv4> ());
      StaticRouting->SetDefaultRoute (Ipv4Address (LanGwIpAddr), 1);
      //StaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"),  Ipv4Address (LanGwIpAddr), 1);
    }

  // add route to lte network in wifi ap gateway
  Ptr<Ipv4StaticRouting> ApGwNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (wifiApNode.Get (0)->GetObject<Ipv4> ());
  ApGwNodeStaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"), 1);
  // add route to lte network in lan mobile gateway
  Ptr<Ipv4StaticRouting> MobileNetworkGwNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (MobileNetworkGwNode->GetObject<Ipv4> ());
  MobileNetworkGwNodeStaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"), 3);
  //MobileNetworkGwNodeStaticRouting->AddNetworkRouteTo (Ipv4Address (WifiIpSubnet), Ipv4Mask (IpMask), 2);
  // add route from lte network to lan and wifi networks in lte packet gateway
  Ptr<Ipv4StaticRouting> pgwStaticRouting = ipv4RoutingHelper.GetStaticRouting (PacketGwNode->GetObject<Ipv4> ());
  pgwStaticRouting->AddNetworkRouteTo (Ipv4Address (csmaIpSubnet), Ipv4Mask (IpMask), 2);
  pgwStaticRouting->AddNetworkRouteTo (Ipv4Address (WifiIpSubnet), Ipv4Mask (IpMask), 2);

  // print routing tables
  PrintRouteNode = MobileNetworkGwNode;
  PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
  PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);

  PrintRouteNode = PacketGwNode;
  PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
  PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);

  PrintRouteNode = LanNodes.Get (nLanNodes);
  PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
  PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);

  PrintRouteNode = wifiStaNodes.Get (0);
  PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
  PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);

  PrintRouteNode = ueNodes.Get (0)		    ;
  PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
  PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);

  //std::pair< Ptr< Ipv4 >, uint32_t > m_ipv4=LanIpInterfaces.Get (nLanNodes+1);
  //std::pair< Ptr< Ipv4 >, uint32_t > m_ipv4=p2pLteIpInterfaces.Get (1);
  //std::cout << "Forwarding = " << std::get<0>(m_ipv4)->IsForwarding (std::get<1>(m_ipv4)) << std::endl;

  // define client and server nodes

  // user always LAN node as client
  NodeContainer ClientNode     = LanNodes.Get (1);
  NodeContainer ClientNode1    = LanNodes.Get (2);
  Ipv4Address   ClientIpAddr   = LanIpInterfaces.GetAddress (1);
  NodeContainer ServerNode     = LanNodes.Get (2);
  Ipv4Address   ServerIPAddr   = LanIpInterfaces.GetAddress (2);
  Ipv4Address   ServerIPAddr1  = LanIpInterfaces.GetAddress (2);

  std::cout << "Use Client Server Config: " << cientServerConfig << std::endl;
  switch (cientServerConfig) {
    case 2:
      // UE station #1
      ServerNode     = ueNodes.Get (0);
      ServerIPAddr   = ueIpInterfaces.GetAddress (0);
      ServerIPAddr1  = wifiIpInterfaces.GetAddress (2);
      break;
    case 3:
      // UE station #1 but WiFi netdevice
      ServerNode     = ueNodes.Get (0);
      ServerIPAddr   = wifiIpInterfaces.GetAddress (2);
      ServerIPAddr1  = wifiIpInterfaces.GetAddress (2);
      break;
    case 4:
      // wifi sta #1
      ServerNode     = wifiStaNodes.Get (0);
      ServerIPAddr   = wifiIpInterfaces.GetAddress (1);
      ServerIPAddr1  = wifiIpInterfaces.GetAddress (1);
      break;
    default: //case 1:
      // LAN node in same network
      ServerNode     = LanNodes.Get (2);
      ServerIPAddr   = LanIpInterfaces.GetAddress (2);
      ServerIPAddr1  = LanIpInterfaces.GetAddress (2);
  }

  // print addresses
  std::cout << "Number of ETH devices      = " << LanIpInterfaces.GetN() << std::endl;
  std::cout << "Number of WiFi devices     = " << wifiIpInterfaces.GetN() << std::endl;
  std::cout << "Number of LTE devices      = " << ueIpInterfaces.GetN() << std::endl;
  std::cout << "Router GW IP Addr          = " << LanGwIpAddr << std::endl;
  std::cout << "WiFi Net GW IP Addr        = " << p2pWifiIpInterfaces.GetAddress (1) << std::endl;
  std::cout << "WiFi AP IP Addr            = " << WiFiGwIpAddr << std::endl;
  std::cout << "WiFI STA#1 IP Addr         = " << wifiIpInterfaces.GetAddress(1) << std::endl;
  std::cout << "LTE Net GW IP Addr         = " << p2pLteIpInterfaces.GetAddress (1) << std::endl;
  std::cout << "LTE EPC PGW IP Addr        = " << PacketGwNode->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal () << std::endl;
  std::cout << "LTE UE#1 IP Addr           = " << ueIpInterfaces.GetAddress(0) << std::endl;
  std::cout << "LTE UE#1 STA IP Addr       = " << wifiIpInterfaces.GetAddress(nWifiStaNodes+1) << std::endl;
  std::cout << "Client IP Addr             = " << ClientIpAddr << std::endl;
  std::cout << "Server IP Addr             = " << ServerIPAddr << std::endl;


  // include application
  Ipv4Address  ClientDestAddr  = ServerIPAddr;
  Ipv4Address  ClientDestAddr1 = ServerIPAddr1;
  uint16_t     DestPort   = 9;   // Discard port (RFC 863)
  Time         interPacketInterval = Seconds(1.0);
  Time         startTime = Seconds(2.0);

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  // undirectional traffic - https://www.nsnam.org/doxygen/classns3_1_1_udp_client.html
  UdpServerHelper      ServerHelp (DestPort);
  UdpClientHelper      ClientHelp (ClientDestAddr, DestPort);
  // bidirectional traffic - https://www.nsnam.org/doxygen/classns3_1_1_udp_echo_client.html
  UdpEchoServerHelper  echoServerHelp (DestPort);
  UdpEchoClientHelper  echoClientHelp (ClientDestAddr, DestPort);
  // random on/off traffic - https://www.nsnam.org/doxygen/classns3_1_1_on_off_application.html
  OnOffHelper OnOffHelp ("ns3::UdpSocketFactory", Address (InetSocketAddress (ClientDestAddr, DestPort)));
  //
  std::cout << "Used Client App: " << clientAppConfig;
  switch (clientAppConfig) {
    case 2:
      std::cout << " -> standard uni directional traffic with two client/server" << std::endl;
      // client config
      ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ClientDestAddr));
      ClientHelp.SetAttribute ("RemotePort", UintegerValue (DestPort));
      ClientHelp.SetAttribute ("MaxPackets", UintegerValue (numPackets));
      ClientHelp.SetAttribute ("Interval", TimeValue (interPacketInterval));
      ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));
      clientApps = ClientHelp.Install (ClientNode);
      // change destination address for 2nd client node
      ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ClientDestAddr1));
      //ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));
      clientApps.Add (ClientHelp.Install (ClientNode1));
      //
      clientApps.Start (startTime);
      clientApps.Stop (Seconds (10.0));
      //
      serverApps = ServerHelp.Install (ServerNode);
      serverApps.Start (startTime);
      serverApps.Stop (Seconds (10.0));
      break;
    case 3:
      std::cout << " -> uni directional traffic with from one client to two destination addresses" << std::endl;
      // initialize and start own_start application to send packets from client nodeto two destination addresses
      OwnStartApplication(ClientNode, ClientDestAddr, ClientDestAddr1, DestPort,
                          packetSize, numPackets, startTime, interPacketInterval);
      //
      serverApps = ServerHelp.Install (ServerNode);
      serverApps.Start (startTime);
      serverApps.Stop (Seconds (10.0));
      break;
    case 4:
      std::cout << " -> bi-directional echo client (ping)" << std::endl;
      // client config
      echoClientHelp.SetAttribute ("MaxPackets", UintegerValue (numPackets));
      echoClientHelp.SetAttribute ("Interval", TimeValue (interPacketInterval));
      echoClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));
      //
      clientApps = echoClientHelp.Install (ClientNode);
      clientApps.Start (startTime);
      clientApps.Stop (Seconds (10.0));
      serverApps = echoServerHelp.Install (ServerNode);
      break;
    case 5:
      std::cout << " -> random traffic (voice)" << std::endl;
      // client config
      clientApps = OnOffHelp.Install (ClientNode);
      clientApps.Start (startTime);
      clientApps.Stop (Seconds (10.0));
      //
      serverApps = ServerHelp.Install (ServerNode);
      serverApps.Start (startTime);
      serverApps.Stop (Seconds (10.0));
      break;
    default: //case 1:
      std::cout << " -> standard uni directional traffic with one client/server" << std::endl;
      /*// client config
      ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ClientDestAddr));
      ClientHelp.SetAttribute ("RemotePort", UintegerValue (DestPort));
      ClientHelp.SetAttribute ("MaxPackets", UintegerValue (numPackets));
      ClientHelp.SetAttribute ("Interval", TimeValue (interPacketInterval));
      ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));
      clientApps = ClientHelp.Install (ClientNode);
      //
      clientApps.Start (startTime);
      clientApps.Stop (Seconds (10.0));*/

      // initialize and start own_start application to send packets from client nodeto two destination addresses
      OwnStartApplication(ClientNode, ClientDestAddr, "0.0.0.0", DestPort,
                          packetSize, numPackets, startTime, interPacketInterval);

      //
      serverApps = ServerHelp.Install (ServerNode);
      serverApps.Start (startTime);
      serverApps.Stop (Seconds (10.0));
  }

  std::cout << "Start simulation \n";

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      p2pHelp.EnablePcapAll ("lte_wifi_vko_p2p", apDevices.Get (0));
      csmaHelp.EnablePcap ("lte_wifi_vko_csma", csmaDevices.Get (0), true);
      wifiPhyHelp.EnablePcap ("lte_wifi_vko_wifi", staDevices);
    }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
