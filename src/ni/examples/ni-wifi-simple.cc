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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/tap-bridge-module.h"

// NI includes
#include "ns3/ni-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NiWifiSimple");

int main (int argc, char *argv[])
{
  bool verbose = false;

  // enable native ns-3 logging - note: can also be used if compiled in "debug" mode
  if (verbose)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_LOGIC);
      //LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_LOGIC);
      //LogComponentEnable ("Ipv4Interface", LOG_LEVEL_LOGIC);
      //LogComponentEnable ("ArpL3Protocol", LOG_LEVEL_LOGIC);
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
  uint32_t nWifiStaNodes = 1;

  // ========================
  // general NI API parameter

  // Activate logging using NI API log files
  bool niApiEnableLogging = true;
  // Set log file names
  std::string LogFileName;
  // Enable TapBridge as data source and data sink
  bool niApiEnableTapBridge = false;

  // =====================
  // NI API Wifi parameter

  // Choose between Adhoc and Infrastructure mode
  std::string niApiWifiConfigMode = "Infrastructure";
  // Configure whether simulator should run as STA1 or STA2 (for Adhoc mode), as AP or STA (for Infrastructure mode)
  std::string niApiWifiDevMode    = "NIAPI_ALL";
  // Activate NIAPI for Wifi
  bool niApiWifiEnabled           = false;
  // Activate NIAPI loopback mode for Wifi
  bool niApiWifiLoopbackEnabled   = false;
  // selected station (if number of stations >1)
  int niApiWifiDeviceSelect = 1;
  // Enable printing out sent/received packet content
  bool niApiWifiEnablePrintMsgContent = false;

  bool niApiConfirmationMessage = true;
  // IP address & Port of ns-3 Wifi MAC API on which packets are sent to PHY (e.g. PXI device)
  std::string niApiWifiSta1RemoteIpAddrTx("127.0.0.1");           // remote address to be used for TX socket opening
  std::string niApiWifiSta1RemotePortTx("12101");                 // remote port to be used for TX socket opening
  std::string niApiWifiSta1LocalPortRx("12701");                  // local port to be used for RX socket opening
  // IP address & Port of ns-3 Wifi MAC API on which packets are received from PHY (e.g. PXI device)
  std::string niApiWifiSta2RemoteIpAddrTx("127.0.0.1");           // remote address to be used for TX socket opening
  std::string niApiWifiSta2RemotePortTx("12102");                 // remote port to be used for TX socket opening
  std::string niApiWifiSta2LocalPortRx("12702");                  // local port to be used for RX socket opening
  // MAC address configurations for 802.11 AFW
  std::string niApiWifiSta1MacAddr("46:6F:4B:75:6D:61");
  std::string niApiWifiSta2MacAddr("46:6F:4B:75:6D:62");
  std::string niApiWifiBssidMacAddr("46:6F:4B:75:6D:61");
  // MCS used by 802.11 AFW
  uint32_t niApiWifiMcs(5);
  //
  std::string phyMode ("DsssRate1Mbps");
  //
  double rss = -80; // -dBm
  //
  std::string phyRate = "VhtMcs8";        // PHY bitrate

  CommandLine cmd;

  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("numPackets", "number of packets generated", packetNum);
  cmd.AddValue("interval", "interval (milliseconds) between packets", packetInterval);
  cmd.AddValue("simTime", "Duration in seconds which the simulation should run", simTime);
  cmd.AddValue("transmTime", "Time in seconds when the packet transmission should be scheduled", transmTime);
  cmd.AddValue("niApiWifiConfigMode", "Set whether the simulation should run in Adhoc or Infrastructure mode", niApiWifiConfigMode);
  cmd.AddValue("niApiWifiDevMode", "Set whether the simulation should run as STA1, STA2 (for Adhoc mode), "
               "as AP and STA (for Infrastructure mode) or none of them", niApiWifiDevMode);
  cmd.AddValue("niApiWifiEnabled", "Enable NI API", niApiWifiEnabled);
  cmd.AddValue("niApiEnableLogging", "Set whether to enable NI_LOG_DEBUGs", niApiEnableLogging);
  cmd.AddValue("niApiWifiLoopbackEnabled", "Enable/disable UDP Loopback on MAC High", niApiWifiLoopbackEnabled);
  cmd.AddValue("niApiEnableTapBridge", "Enable/disable TapBridge as external data source/sink", niApiEnableTapBridge);
  cmd.AddValue("niApiWifiEnablePrintMsgContent", "Set whether the simulation should print out the contents of sent/received packets", niApiWifiEnablePrintMsgContent);
  cmd.AddValue("niApiWifiSta1RemoteIpAddrTx", "Remote IP address for UDP socket on STA1", niApiWifiSta1RemoteIpAddrTx);
  cmd.AddValue("niApiWifiSta1RemotePortTx", "Remote port for UDP socket on STA1", niApiWifiSta1RemotePortTx);
  cmd.AddValue("niApiWifiSta1LocalPortRx", "Local RX port of STA1", niApiWifiSta1LocalPortRx);
  cmd.AddValue("niApiWifiSta2RemoteIpAddrTx", "Remote IP address for UDP socket on STA2", niApiWifiSta2RemoteIpAddrTx);
  cmd.AddValue("niApiWifiSta2RemotePortTx", "Remote port for UDP socket on STA2", niApiWifiSta2RemotePortTx);
  cmd.AddValue("niApiWifiSta2LocalPortRx", "Local RX port of STA2", niApiWifiSta2LocalPortRx);
  cmd.AddValue("niApiWifiSta1MacAddr", "MAC address of STA1 in format ff:ff:ff:ff:ff:ff", niApiWifiSta1MacAddr);
  cmd.AddValue("niApiWifiSta2MacAddr", "MAC address of STA2 in format ff:ff:ff:ff:ff:ff", niApiWifiSta2MacAddr);
  cmd.AddValue("niApiWifiBssidMacAddr", "MAC address of BSSID in format ff:ff:ff:ff:ff:ff", niApiWifiBssidMacAddr);
  cmd.AddValue("niApiWifiMcs", "MCS to be used by the 802.11 AFW", niApiWifiMcs);
  cmd.AddValue("niApiWifiStationNum", "Set whether the device should run as STA1 or STA2 in Infrastructure mode", nWifiStaNodes);
  cmd.AddValue("niApiWifiDeviceSelect", "Set whether the device should run as STA1 or STA2 in Infrastructure mode", niApiWifiDeviceSelect);

  cmd.Parse (argc, argv);

  // Activate the ns-3 real time simulator
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  // Set real time mode (need RT preempt patch?) -> real time mode with synchronization mode set to Hard Limit does not work properly?
  Config::SetDefault ("ns3::RealtimeSimulatorImpl::SynchronizationMode", StringValue("BestEffort")); //BestEffort
  //Config::SetDefault ("ns3::RealtimeSimulatorImpl::HardLimit", StringValue("+100000000.0ns"));

  // ============================
  // General NI API Configuration

  std::cout << std::endl;
  std::cout << "-------- NS-3 Configuration -------------" << std::endl;
  std::cout << "\nSelected WiFi mode: \t" << niApiWifiConfigMode << std::endl;
  std::cout << "Running node as: \t";
  if (niApiWifiConfigMode == "Adhoc")
    {
      if (niApiWifiDevMode == "NIAPI_NONE") std::cout << "NoneOfThem" << std::endl;
      else if (niApiWifiDevMode == "NIAPI_STA1") std::cout << "STA1" << std::endl;
      else if (niApiWifiDevMode == "NIAPI_STA2") std::cout << "STA2" << std::endl;
      else if (niApiWifiDevMode == "NIAPI_ALL") std::cout << "Both Sta1 and Sta2" << std::endl;
    }
  else if (niApiWifiConfigMode == "Infrastructure")
    {
      if (niApiWifiDevMode == "NIAPI_NONE") std::cout << "NoneOfThem" << std::endl;
      else if (niApiWifiDevMode == "NIAPI_AP") std::cout << "AP" << std::endl;
      else if (niApiWifiDevMode == "NIAPI_STA") std::cout << "STA" << std::endl;
      else if (niApiWifiDevMode == "NIAPI_ALL") std::cout << "AP and STA both" << std::endl;
    }

  std::cout << "WiFi API: \t\t";
  if (niApiWifiEnabled == true) std::cout << "enabled" << std::endl;
  else                          std::cout << "disabled" << std::endl;

  std::cout << "WiFi UDP Loopback: \t";
  if (niApiWifiLoopbackEnabled == true) std::cout << "enabled" << std::endl;
  else                                  std::cout << "disabled" << std::endl;

  std::cout << "TapBridge: \t\t";
  if (niApiEnableTapBridge == true) std::cout << "enabled" << std::endl;
  else                              std::cout << "disabled" << std::endl;

  std::cout << "Logging: \t\t";
  if (niApiEnableLogging == true) std::cout << "enabled" << std::endl;
  else                            std::cout << "disabled" << std::endl;

  std::cout << "NI Module Version: \t" << NI_MODULE_VERSION << std::endl;
  std::cout << "Required AFW Version: \t" << NI_AFW_VERSION << std::endl;

  // write different log files for each STA
  std::string stationType = "default";

  if (niApiWifiConfigMode == "Adhoc")
    {
      if (niApiWifiDevMode == "NIAPI_STA1") stationType = "STA1";
      else if (niApiWifiDevMode == "NIAPI_STA2") stationType = "STA2";
    }

  else if (niApiWifiConfigMode == "Infrastructure")
    {
      if (niApiWifiDevMode == "NIAPI_AP") stationType = "AP";
      else if (niApiWifiDevMode == "NIAPI_STA") stationType = "STA";
    }

  std::cout << "Simulation time: " << simTime << " s = " << int(simTime/3600) << " h " << (int(simTime/60) % 60) << " min\n\n";

  NI_LOG_CONSOLE_INFO ("Init NI modules ...");
  int ns3priority = NiUtils::GetThreadPrioriy();
  int niLoggingPriority = ns3priority - 10;
  // adding thread ID of main NS3 thread for possible troubleshooting
  NiUtils::AddThreadInfo(pthread_self(), "NS3 main thread");

  // install signal handlers in order to print debug information to std::out in case of an error
  NiUtils::InstallSignalHandler();

  if (niApiEnableLogging){
      std::string LogFileName = "/tmp/Log_Wifi_" + stationType + ".txt";
      NiLoggingInit(LOG__LEVEL_ALL | LOG__CONSOLE_DEBUG, LogFileName, NI_LOG__INSTANT_WRITE_DISABLE, niLoggingPriority);
  }

  // =========================
  // NI Wifi API Configuration

  if (niApiWifiConfigMode == "Adhoc")
    {
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType", StringValue ( niApiWifiDevMode.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::enableNiApi", BooleanValue (niApiWifiEnabled));
      Config::SetDefault ("ns3::NiWifiMacInterface::enableNiApiLoopback", BooleanValue (niApiWifiLoopbackEnabled));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiEnablePrintMsgContent", BooleanValue (niApiWifiEnablePrintMsgContent));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiConfirmationMessage", BooleanValue (niApiConfirmationMessage));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta1RemoteIpAddrTx", StringValue ( niApiWifiSta1RemoteIpAddrTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta1RemotePortTx", StringValue ( niApiWifiSta1RemotePortTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta1LocalPortRx", StringValue ( niApiWifiSta1LocalPortRx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta2RemoteIpAddrTx", StringValue ( niApiWifiSta2RemoteIpAddrTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta2RemotePortTx", StringValue ( niApiWifiSta2RemotePortTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta2LocalPortRx", StringValue ( niApiWifiSta2LocalPortRx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta1MacAddr", StringValue (niApiWifiSta1MacAddr.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiSta2MacAddr", StringValue (niApiWifiSta2MacAddr.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiBssidMacAddress", StringValue (niApiWifiBssidMacAddr.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiMcs", IntegerValue(niApiWifiMcs));
    }
  else if (niApiWifiConfigMode == "Infrastructure")
    {
      // Set default configurations for AP MAC High
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType", StringValue ( niApiWifiDevMode.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::enableNiApi", BooleanValue (niApiWifiEnabled));
      Config::SetDefault ("ns3::NiWifiMacInterface::enableNiApiLoopback", BooleanValue (niApiWifiLoopbackEnabled));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiApRemoteIpAddrTx", StringValue ( niApiWifiSta1RemoteIpAddrTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiApRemotePortTx", StringValue ( niApiWifiSta1RemotePortTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiApLocalPortRx", StringValue ( niApiWifiSta1LocalPortRx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiStaRemoteIpAddrTx", StringValue ( niApiWifiSta2RemoteIpAddrTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiStaRemotePortTx", StringValue ( niApiWifiSta2RemotePortTx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiStaLocalPortRx", StringValue ( niApiWifiSta2LocalPortRx.c_str() ));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiApMacAddress", StringValue (niApiWifiSta1MacAddr.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiStaMacAddress", StringValue (niApiWifiSta2MacAddr.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiBssidMacAddress", StringValue (niApiWifiBssidMacAddr.c_str()));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiEnablePrintMsgContent", BooleanValue (niApiWifiEnablePrintMsgContent));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiConfirmationMessage", BooleanValue (niApiConfirmationMessage));
      Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiMcs", IntegerValue(niApiWifiMcs));
      Config::SetDefault ("ns3::NiWifiMacInterface::NumOfStations", IntegerValue (nWifiStaNodes));
    }

  // wifi helper
  Ssid       ssid = Ssid ("wifi-default");

  WifiHelper wifiHelp;
             wifiHelp.SetStandard (WIFI_PHY_STANDARD_80211b); // WIFI_PHY_STANDARD_80211ac does not work correctly - no assoc req/resp
             wifiHelp.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                               "DataMode", StringValue (phyMode),
                                               "ControlMode", StringValue (phyMode));
  //if (verbose) wifiHelp.EnableLogComponents ();  // Turn on all Wifi logging

  NqosWifiMacHelper wifiApMacHelp, wifiStaMacHelp, wifiAdHocMacHelp = NqosWifiMacHelper::Default(); // mac with disabled rate control
                    wifiApMacHelp.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
                    wifiStaMacHelp.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
                    wifiAdHocMacHelp.SetType ("ns3::AdhocWifiMac");

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  YansWifiChannelHelper wifiChannelHelp = YansWifiChannelHelper::Default ();
                        wifiChannelHelp.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
                        wifiChannelHelp.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  YansWifiPhyHelper     wifiPhyHelp =  YansWifiPhyHelper::Default ();
                        wifiPhyHelp.Set ("RxGain", DoubleValue (0) );
                        wifiPhyHelp.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
                        wifiPhyHelp.SetChannel (wifiChannelHelp.Create ());

  MobilityHelper MobilityHelp;
                 MobilityHelp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // ip helper
  InternetStackHelper     ipStackHelp;
  Ipv4AddressHelper       ipAddressHelp;
  Ipv4Mask                IpMask = "255.255.255.0";
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // wifi station nodes
  NodeContainer wifiStaNodes;
                wifiStaNodes.Create (nWifiStaNodes);
  // wifi access point nodes - here fixed to one but can be extended
  NodeContainer wifiApNode;
                wifiApNode.Create (1);

  // install corresponding net devices on wifi nodes
  NetDeviceContainer staDevices;
  NetDeviceContainer apDevice;

  if (niApiWifiConfigMode == "Infrastructure")
    {
      // install ap device
      apDevice  = wifiHelp.Install (wifiPhyHelp, wifiApMacHelp, wifiApNode);

      for (uint32_t index = 0; index < nWifiStaNodes; index++)
        {
          if (((index+1) == niApiWifiDeviceSelect) && ((niApiWifiDevMode == "NIAPI_STA")||(niApiWifiDevMode == "NIAPI_ALL")))
            {
              // set api device type to NIAPI_STA to create udp socket for api connection
              Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType",StringValue ( "NIAPI_STA"));

              NI_LOG_CONSOLE_INFO ("Station Device #" << niApiWifiDeviceSelect <<" installed");
            }
          else {
              // set api device type to NIAPI_NONE to not create udp socket
              Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType",StringValue ( "NIAPI_NONE"));
          }
          // install station devices
          staDevices.Add(wifiHelp.Install (wifiPhyHelp, wifiStaMacHelp, wifiStaNodes.Get (index)));
        }
    }
  else if (niApiWifiConfigMode == "Adhoc")
    {
      // open sockets just on the first node if simulation is running as STA1
      if ((niApiWifiDevMode == "NIAPI_ALL") || (niApiWifiDevMode == "NIAPI_STA1"))
        {
          Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType",StringValue ( "NIAPI_STA1"));

          NI_LOG_CONSOLE_INFO ("Station Device #1 installed");
        }
      else {
          // set api device type to NIAPI_NONE to not create udp socket for station 2
          Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType",StringValue ( "NIAPI_NONE"));
      }
      // install station 1 device
      apDevice  = wifiHelp.Install (wifiPhyHelp, wifiAdHocMacHelp, wifiApNode);

      // open sockets just on the second node if simulation is running as STA2
      if ((niApiWifiDevMode == "NIAPI_ALL") || (niApiWifiDevMode == "NIAPI_STA2"))
        {
          Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType", StringValue ( "NIAPI_STA2"));

          NI_LOG_CONSOLE_INFO ("Station Device #2 installed");
        }
      else {
          // set api device type to NIAPI_NONE to not create udp socket for station 1
          Config::SetDefault ("ns3::NiWifiMacInterface::niApiDevType",StringValue ( "NIAPI_NONE"));
      }
      // install station 2 device
      staDevices.Add(wifiHelp.Install (wifiPhyHelp, wifiAdHocMacHelp, wifiStaNodes.Get (0)));
    }

  // wifi channel / mobility  model
  MobilityHelp.Install (wifiApNode);
  MobilityHelp.Install (wifiStaNodes);

  // install ip stacks on all nodes
  ipStackHelp.Install (wifiApNode);
  ipStackHelp.Install (wifiStaNodes);

  // configure ip address spaces and gateways for the different subnetworks
  Ipv4Address WifiIpSubnet    = "10.1.2.0";

  // create wifi network - ap0 is connected to mobile network gateway
  ipAddressHelp.SetBase (WifiIpSubnet, IpMask);
  Ipv4InterfaceContainer wifiIpInterfaces;
                         wifiIpInterfaces.Add (ipAddressHelp.Assign (apDevice));
                         wifiIpInterfaces.Add (ipAddressHelp.Assign (staDevices));

  // initialize routing database and set up the routing tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  if (!niApiEnableTapBridge)
    {
      NodeContainer ClientNode     = wifiApNode.Get (0);
      Ipv4Address   ClientIpAddr   = wifiIpInterfaces.GetAddress (0);
      std::cout << "Client IP Addr   = " << ClientIpAddr << std::endl;

      NodeContainer ServerNode    = wifiStaNodes.Get (0);
      Ipv4Address   ServerIPAddr  = wifiIpInterfaces.GetAddress (1);

      Ipv4Address  ClientDestAddr = ServerIPAddr;
      uint16_t     DestPort = 9;   // Discard port (RFC 863)
      // Convert to time object
      Time packetIntervalSec = Seconds(packetInterval*1E-03);

      // undirectional traffic - https://www.nsnam.org/doxygen/classns3_1_1_udp_client.htm->
      NiUdpServerHelper     ServerHelp (DestPort);
      NiUdpClientHelper     ClientHelp(ClientDestAddr, DestPort);
      ClientHelp.SetAttribute ("MaxPackets", UintegerValue (packetNum));
      ClientHelp.SetAttribute ("Interval", TimeValue (packetIntervalSec));
      ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));

      ApplicationContainer clientApps;

      if ((niApiWifiDevMode == "NIAPI_AP")||(niApiWifiDevMode == "NIAPI_ALL")||(niApiWifiDevMode == "NIAPI_STA1"))
        {
          // create multiple clients to send packets to all involved stations in the network
          for(uint8_t index = 1; index <= nWifiStaNodes; index++ )
            {
              ServerNode     = wifiStaNodes.Get (niApiWifiDeviceSelect-1);
              ServerIPAddr   = wifiIpInterfaces.GetAddress (niApiWifiDeviceSelect);
              std::cout << "Server#" << (uint16_t)index <<" IP Addr = " << ServerIPAddr << std::endl;
              ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ServerIPAddr));
              ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize - (index*200)));
              clientApps.Add(ClientHelp.Install (ClientNode));
            }
          clientApps.Start (Seconds (transmTime));
          clientApps.Stop (Seconds (simTime));
        }

      ApplicationContainer serverApps;
      serverApps = ServerHelp.Install (wifiStaNodes);
      //		for(uint8_t index = 0; index < nWifiStaNodes; index++ )
      //		{
      //			serverApps= ServerHelp.Install (wifiStaNodes.Get(index));
      //			serverApps.Start (Seconds (index+2));
      //			serverApps.Stop (Seconds (simTime+(index*10)));
      //		}

      serverApps.Start (Seconds (1.0));
      serverApps.Stop (Seconds (simTime+10));
    }
  else // running program with TAP device as data source/sink
    {
      std::string mode = "ConfigureLocal";
      std::string tapName = "";

      std::cout << "Using " << mode << " mode for TAP device configuration." << std::endl;
      std::cout <<"Install TAP bridge for " << stationType << " IP node in " << niApiWifiConfigMode << " mode.\n" <<std::endl;

      if ((niApiWifiDevMode == "NIAPI_STA1") || (niApiWifiDevMode == "NIAPI_AP"))
        {
          if (niApiWifiDevMode == "NIAPI_STA1") tapName = "tapSta1";
          else if (niApiWifiDevMode == "NIAPI_AP") tapName = "tapAp";

          TapBridgeHelper tapBridge (wifiIpInterfaces.GetAddress(0));
          tapBridge.SetAttribute ("Mode", StringValue (mode));
          tapBridge.SetAttribute ("DeviceName", StringValue (tapName));
          tapBridge.SetAttribute ("Mtu", UintegerValue (1500));
          tapBridge.Install (wifiApNode.Get (0), apDevice.Get (0));
        }

      else if ((niApiWifiDevMode == "NIAPI_STA2") || (niApiWifiDevMode == "NIAPI_STA"))
        {
          if (niApiWifiDevMode == "NIAPI_STA2") tapName = "tapSta2";
          else if (niApiWifiDevMode == "NIAPI_STA") tapName = "tapSta";

          TapBridgeHelper tapBridge (wifiIpInterfaces.GetAddress(1));
          tapBridge.SetAttribute ("Mode", StringValue (mode));
          tapBridge.SetAttribute ("DeviceName", StringValue (tapName));
          tapBridge.SetAttribute ("Mtu", UintegerValue (1500));
          tapBridge.Install (wifiStaNodes.Get (0), staDevices.Get (0));
        }
    }

  // ==========================================================================================

  // stop the simulation after simTime seconds
  Simulator::Stop(Seconds(simTime));

  std::cout << std::endl;
  std::cout << "[>] Start simulation" << std::endl;
  Simulator::Run ();
  std::cout << "[#] End simulation" << std::endl << std::endl;

  Simulator::Destroy ();

  // De-init NI modules
  NI_LOG_CONSOLE_INFO ("De-init NI modules ...");
  if (niApiEnableLogging)
    {
      NiLoggingDeInit();
    }

  NI_LOG_CONSOLE_INFO ("---- Program end! ---- ");
  return 0;
}
