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
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include "ns3/lte-helper.h"
#include "ns3/csma-module.h"
#include "ns3/epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/virtual-net-device.h"
#include "ns3/tap-bridge-module.h"

// NI includes
#include "ns3/ni-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NiLwaLwipExt");

// global variables
bool g_niApiEnableTapBridge = false;

//*******************************LWA/LWIP extensions by UOC********************************

Ptr<Socket> lwaapTxSocket;
Ptr<Socket> lwipepTxSocket;

// function for lwa/lwip packet handling transmitted through the LtePdcp::DoTransmitPdcpSdu
void LtePdcpLwaLwipHandler (Ptr< const Packet> p){

  bool clientServerAppEnabled = true; // false for tapbridge
  clientServerAppEnabled = !g_niApiEnableTapBridge; // copy from global value

  uint32_t currentPacketSeqNum=0;
  SeqTsHeader seqTs;

  // copy incoming packet
  Ptr<Packet> currentPacket = p->Copy();

  // remove headers to get sequence number and include lwa/lwip header
  // further this (internal) header removal is needed if packets going through the tapbridge
  LtePdcpHeader pdcpHeader;
  currentPacket->RemoveHeader (pdcpHeader);
  Ipv4Header ipHeader;
  currentPacket->RemoveHeader (ipHeader);
  UdpHeader udpHeader;
  currentPacket->RemoveHeader (udpHeader);
  // remove sequence number and add UDP/IP/PDCP only for Client-Server application, NOT for tapbridge
  if (clientServerAppEnabled)
    {
      currentPacket->RemoveHeader (seqTs);
      currentPacketSeqNum=seqTs.GetSeq ();

      // add headers again
      currentPacket->AddHeader (udpHeader);
      currentPacket->AddHeader (ipHeader);
      currentPacket->AddHeader (pdcpHeader);
    }

  // read lwa/lwip tags added by pdcp layer and add lwa/lwip information
  // as separate header to data packets (won't be removed when transmitting
  // packets via API)
  LwaTag   lwaTag;
  LwipTag  lwipTag;
  PdcpLcid lcidTag;
  uint32_t lcid=0;
  uint32_t bid=0;
  double   lwaMode=0;
  LwaLwipHeader lwaLwipHeader;

  // tag to copy the logical channel id of the frame
  if(currentPacket->FindFirstMatchingByteTag (lcidTag)){
      lcid = lcidTag.Get();
      bid = lcid - 2;
  }
  lwaLwipHeader.SetBearerId (bid);

  // packet handling for lwa packets
  if (currentPacket->PeekPacketTag(lwaTag)){
      // copy additional headers only for Client-Server application, NOT for tapbridge
      if (clientServerAppEnabled)
        {
          //copy the status of LWA activation (LTE, Split, swtiched)
          lwaLwipHeader.SetLwaActivate (lwaTag.Get());
          // add lwa lwip header
          currentPacket->AddHeader (lwaLwipHeader);
          // add current sequence number
          seqTs.SetSeq (currentPacketSeqNum);
          currentPacket->AddHeader (seqTs);
        }
      // send packet over the lwaap socket
      lwaapTxSocket->Send(currentPacket);

      NI_LOG_CONSOLE_DEBUG ("LWA: Sent packet with Sequence Number " << currentPacketSeqNum);
  }
  // packet handling for lwip packets
  if (currentPacket->PeekPacketTag (lwipTag)){
      // copy additional headers only for Client-Server application, NOT for tapbridge
      if (clientServerAppEnabled)
        {
          //copy the status of LWA activation (LTE, Split, swtiched)
          lwaLwipHeader.SetLwipActivate (lwipTag.Get());
          // add lwa lwip header
          currentPacket->AddHeader (lwaLwipHeader);
          // add current sequence number
          seqTs.SetSeq (currentPacketSeqNum);
          currentPacket->AddHeader (seqTs);
        }
      // send packet over the lwipep socket
      lwipepTxSocket->Send(currentPacket);

      NI_LOG_CONSOLE_DEBUG ("LWIP: Sent packet with Sequence Number " << currentPacketSeqNum);
  }
}

// assign callback function to the pdcp object
// NOTE: object is created after bearer release which is after attach procedure
static void
Callback_LtePDCPTX
(void){
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LtePdcp/TxPDUtrace", MakeCallback (&LtePdcpLwaLwipHandler));
}

// tunnel class for lwip ipsec emulation
class Tunnel
{
  Ptr<Socket> m_n3Socket;
  Ptr<Socket> m_n1Socket;
  Ipv4Address m_n3Address;
  Ipv4Address m_n1Address;
  Ptr<UniformRandomVariable> m_rng;
  Ptr<VirtualNetDevice> m_n1Tap;
  Ptr<VirtualNetDevice> m_n3Tap;

  bool
  N1VirtualSend (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
  {
    m_n1Socket->SendTo (packet, 0, InetSocketAddress (m_n3Address, 667));
    return true;
  }

  bool
  N3VirtualSend (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
  {
    m_n3Socket->SendTo (packet, 0, InetSocketAddress (m_n1Address, 667));
    return true;
  }

  void N3SocketRecv (Ptr<Socket> socket)
  {
    Ptr<Packet> packet = socket->Recv (65535, 0);
    m_n3Tap->Receive (packet, 0x0800, m_n3Tap->GetAddress (), m_n3Tap->GetAddress (), NetDevice::PACKET_HOST);
  }

  void N1SocketRecv (Ptr<Socket> socket)
  {
    Ptr<Packet> packet = socket->Recv (65535, 0);
    m_n1Tap->Receive (packet, 0x0800, m_n1Tap->GetAddress (), m_n1Tap->GetAddress (), NetDevice::PACKET_HOST);
  }

public:

  Tunnel (Ptr<Node> n3, Ptr<Node> n1,
          Ipv4Address n3Addr, Ipv4Address n1Addr)
: m_n3Address (n3Addr), m_n1Address (n1Addr)
{
    m_rng = CreateObject<UniformRandomVariable> ();
    m_n3Socket = Socket::CreateSocket (n3, TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_n3Socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 667));
    m_n3Socket->SetRecvCallback (MakeCallback (&Tunnel::N3SocketRecv, this));

    m_n1Socket = Socket::CreateSocket (n1, TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_n1Socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 667));
    m_n1Socket->SetRecvCallback (MakeCallback (&Tunnel::N1SocketRecv, this));

    // n1 tap device
    m_n1Tap = CreateObject<VirtualNetDevice> ();
    m_n1Tap->SetAddress (Mac48Address ("11:00:01:02:03:02"));
    m_n1Tap->SetSendCallback (MakeCallback (&Tunnel::N1VirtualSend, this));
    n1->AddDevice (m_n1Tap);
    Ptr<Ipv4> ipv4 = n1->GetObject<Ipv4> ();
    uint32_t i = ipv4->AddInterface (m_n1Tap);
    ipv4->AddAddress (i, Ipv4InterfaceAddress (Ipv4Address ("11.0.0.254"), Ipv4Mask ("255.255.255.0")));
    ipv4->SetUp (i);

    // n3 tap device
    m_n3Tap = CreateObject<VirtualNetDevice> ();
    m_n3Tap->SetAddress (Mac48Address ("11:00:01:02:03:04"));
    m_n3Tap->SetSendCallback (MakeCallback (&Tunnel::N3VirtualSend, this));
    n3->AddDevice (m_n3Tap);
    ipv4 = n3->GetObject<Ipv4> ();
    i = ipv4->AddInterface (m_n3Tap);
    ipv4->AddAddress (i, Ipv4InterfaceAddress (Ipv4Address ("11.0.0.1"), Ipv4Mask ("255.255.255.0")));
    ipv4->SetUp (i);
}

};
//**********************************************************************************************

//-------------------------------------------------------------------------------------
// main function
int main (int argc, char *argv[]){

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
  // define client server configuration
  int cientServerConfig   = 1;

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
  std::string niApiLteDevMode  = "NIAPI_ALL";
  // Activate NIAPI for LTE
  bool niApiLteEnabled         = false;
  // Activate NIAPI loopback mode for LTE
  bool niApiLteLoopbackEnabled = false; // true -> UDP loopback, FALSE -> PIPEs are used
  // sinr value in db used for cqi calculation for the ni phy
  double niChSinrValueDb = 10;

  // =====================
  // NI API Wifi parameter

  // Choose between Adhoc and Infrastructure mode - fixed in this simulation to Infra
  std::string niApiWifiConfigMode = "Infrastructure";
  // Configure whether simulator should run as as AP or STA (for Infrastructure mode)
  std::string niApiWifiDevMode    = "NIAPI_ALL";
  // Activate NIAPI for WiFi
  bool niApiWifiEnabled           = false;
  // Activate NIAPI loopback mode for Wifi
  bool niApiWifiLoopbackEnabled   = false;
  // Enable printing out sent/received packet content
  bool niApiWifiEnablePrintMsgContent = false;
  // IP address & Port of ns-3 Wifi MAC API on which packets are sent to PHY (e.g. PXI device)
  std::string niApiWifiSta1RemoteIpAddrTx("127.0.0.1");           // remote address to be used for TX socket opening
  std::string niApiWifiSta1RemotePortTx("12701");                 // remote port to be used for TX socket opening
  std::string niApiWifiSta1LocalPortRx("12702");                  // local port to be used for RX socket opening
  // IP address & Port of ns-3 Wifi MAC API on which packets are received from PHY (e.g. PXI device)
  std::string niApiWifiSta2RemoteIpAddrTx("127.0.0.1");           // remote address to be used for TX socket opening
  std::string niApiWifiSta2RemotePortTx("12702");                 // remote port to be used for TX socket opening
  std::string niApiWifiSta2LocalPortRx("12701");                  // local port to be used for RX socket opening
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

  // =======================
  // UOC LWIP/LWA extensions

  double lwaactivate=0;     // LTE+Wifi=1, Wi-Fi=2
  double lwipactivate=0;    // if 1 all packets will be transmitted via LWIP

  // MTU size for P2P link between EnB and Wifi AP
  double xwLwaLinkMtuSize=1500;
  //delay of p2p link
  std::string xwLwaLinkDelay="0ms";
  //data rate of p2p link
  std::string xwLwaLinkDataRate="100Mbps";

  // ==========================================
  // parse command line for function parameters

  CommandLine cmd;
  std::string niApiWifiSta1RemoteIpAddrTxTmp, niApiWifiSta1RemotePortTxTmp,niApiWifiSta1LocalPortRxTmp;
  std::string niApiWifiSta2RemoteIpAddrTxTmp, niApiWifiSta2RemotePortTxTmp, niApiWifiSta2LocalPortRxTmp;

  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("numPackets", "number of packets generated", packetNum);
  cmd.AddValue("interval", "interval (milliseconds) between packets", packetInterval);
  cmd.AddValue("simTime", "Duration in seconds which the simulation should run", simTime);
  cmd.AddValue("transmTime", "Time in seconds when the packet transmission should be scheduled", transmTime);
  cmd.AddValue("cientServerConfig", "Client Server Configuration", cientServerConfig);
  cmd.AddValue("niApiEnableTapBridge", "Enable/disable TapBridge as external data source/sink", niApiEnableTapBridge);
  cmd.AddValue("niRemoteControlEnable", "Enable/disable Remote Control engine", niRemoteControlEnable);
  cmd.AddValue("niApiEnableLogging", "Set whether to enable NIAPI_DebugLogs", niApiEnableLogging);
  cmd.AddValue("niApiDevMode", "Set whether the simulation should run as BS or Terminal", niApiDevMode);
  cmd.AddValue("niApiWifiEnabled", "Enable NI API for WiFi", niApiWifiEnabled);
  cmd.AddValue("niApiWifiLoopbackEnabled", "Enable/disable UDP loopback mode for WiFi NI API", niApiWifiLoopbackEnabled);
  cmd.AddValue("niApiWifiSta1RemoteIpAddrTx", "Remote IP address for UDP socket on STA1", niApiWifiSta1RemoteIpAddrTxTmp);
  cmd.AddValue("niApiWifiSta1RemotePortTx", "Remote port for UDP socket on STA1", niApiWifiSta1RemotePortTxTmp);
  cmd.AddValue("niApiWifiSta1LocalPortRx", "Local RX port of STA1", niApiWifiSta1LocalPortRxTmp);
  cmd.AddValue("niApiWifiSta2RemoteIpAddrTx", "Remote IP address for UDP socket on STA2", niApiWifiSta2RemoteIpAddrTxTmp);
  cmd.AddValue("niApiWifiSta2RemotePortTx", "Remote port for UDP socket on STA2", niApiWifiSta2RemotePortTxTmp);
  cmd.AddValue("niApiWifiSta2LocalPortRx", "Local RX port of STA2", niApiWifiSta2LocalPortRxTmp);
  cmd.AddValue("niApiWifiSta1MacAddr", "MAC address of STA1 in format ff:ff:ff:ff:ff:ff", niApiWifiSta1MacAddr);
  cmd.AddValue("niApiWifiSta2MacAddr", "MAC address of STA2 in format ff:ff:ff:ff:ff:ff", niApiWifiSta2MacAddr);
  cmd.AddValue("niApiWifiBssidMacAddr", "MAC address of BSSID in format ff:ff:ff:ff:ff:ff", niApiWifiBssidMacAddr);
  cmd.AddValue("niApiWifiMcs", "MCS to be used by the 802.11 AFW", niApiWifiMcs);
  cmd.AddValue("niApiLteEnabled", "Enable NI API for LTE", niApiLteEnabled);
  cmd.AddValue("niApiLteLoopbackEnabled", "Enable/disable UDP loopback mode for LTE NI API", niApiLteLoopbackEnabled);
  cmd.AddValue("lwaactivate", "Activate LWA interworking", lwaactivate);
  cmd.AddValue("lwipactivate", "Activate LWIP interworking", lwipactivate);

  cmd.Parse (argc, argv);

  // store value global
  g_niApiEnableTapBridge = niApiEnableTapBridge;

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
      niApiWifiDevMode = "NIAPI_AP";
      simStationType   = "BS";
  } else if (niApiDevMode == "NIAPI_TS") {
      // if ns-3 instance is configured as base station it acts as UE / STA
      niApiLteDevMode  = "NIAPI_UE";
      niApiWifiDevMode = "NIAPI_STA";
      simStationType   = "TS";
  } else if (niApiDevMode == "NIAPI_BSTS") {
      // ns-3 instance is configures as BS and TS- for debugging if only spectrum phy shall be bypassed
      niApiLteDevMode  = "NIAPI_ALL";
      niApiWifiDevMode = "NIAPI_ALL";
      simStationType   = "BSTS";
  } else {
      NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed");
  }

  // check and apply 802.11 API settings
  if (niApiWifiSta1RemoteIpAddrTxTmp.empty() || niApiWifiSta1RemotePortTxTmp.empty() || niApiWifiSta1LocalPortRxTmp.empty())
    {
      // API settings where empty or incomplete, assign defaults
      niApiWifiSta1RemoteIpAddrTx = "127.0.0.1";
      if (niApiWifiLoopbackEnabled)
        {
          // apply default settings for local loopback
          niApiWifiSta1RemotePortTx = "12701";
          niApiWifiSta1LocalPortRx = "12702";
        }
      else
        {
          // apply settings for connection with SDR / NI 802.11 Application Framework
          niApiWifiSta1RemotePortTx = "12101";
          niApiWifiSta1LocalPortRx = "12701";
        }
    }
    else
      {
        // API settings complete, values passed by arguments
        niApiWifiSta1RemoteIpAddrTx = niApiWifiSta1RemoteIpAddrTxTmp;
        niApiWifiSta1RemotePortTx = niApiWifiSta1RemotePortTxTmp;
        niApiWifiSta1LocalPortRx = niApiWifiSta1LocalPortRxTmp;
      }
    if (niApiWifiSta2RemoteIpAddrTxTmp.empty() || niApiWifiSta2RemotePortTxTmp.empty() || niApiWifiSta2LocalPortRxTmp.empty())
      {
        // API settings where empty or incomplete, assign defaults
        niApiWifiSta2RemoteIpAddrTx = "127.0.0.1";
        if (niApiWifiLoopbackEnabled)
          {
            // apply default settings for local loopback
            niApiWifiSta2RemotePortTx = "12702";
            niApiWifiSta2LocalPortRx = "12701";
          }
        else
          {
            // apply settings for connection with SDR / NI 802.11 Application Framework
            niApiWifiSta2RemotePortTx = "12102";
            niApiWifiSta2LocalPortRx = "12702";
          }
      }
      else
        {
          // API settings complete, values passed by arguments
          niApiWifiSta2RemoteIpAddrTx = niApiWifiSta2RemoteIpAddrTxTmp;
          niApiWifiSta2RemotePortTx = niApiWifiSta2RemotePortTxTmp;
          niApiWifiSta2LocalPortRx = niApiWifiSta2LocalPortRxTmp;
        }

  // print ouf config parameters
  // TODO-NI: replace cout by NI_LOG_CONSOLE_INFO
  std::cout << std::endl;
  std::cout << "-------- NS-3 Configuration -------------" << std::endl;
  std::cout << "Selected WiFi mode:    " << niApiWifiConfigMode << std::endl;
  std::cout << "Running WiFi node as:  ";
  if (niApiWifiDevMode == "NIAPI_ALL") std::cout << "AP and STA" << std::endl;
  else if (niApiWifiDevMode == "NIAPI_AP")    std::cout << "AP" << std::endl;
  else if (niApiWifiDevMode == "NIAPI_STA")   std::cout << "STA" << std::endl;

  std::cout << "WiFi API:              ";
  if (niApiWifiEnabled == true) std::cout << "enabled" << std::endl;
  else                          std::cout << "disabled" << std::endl;

  std::cout << "WiFI UDP Loopback:     ";
  if (niApiWifiLoopbackEnabled == true) std::cout << "enabled" << std::endl;
  else                                  std::cout << "disabled" << std::endl;

  std::cout << "Running LTE node as:   ";
  if (niApiLteDevMode == "NIAPI_ALL") std::cout << "ENB and UE" << std::endl;
  else if (niApiLteDevMode == "NIAPI_ENB")  std::cout << "ENB" << std::endl;
  else if (niApiLteDevMode == "NIAPI_UE")   std::cout << "UE" << std::endl;

  std::cout << "LTE API:               ";
  if (niApiLteEnabled == true) std::cout << "enabled" << std::endl;
  else                         std::cout << "disabled" << std::endl;

  std::cout << "LTE UDP Loopback:      ";
  if (niApiLteLoopbackEnabled == true) std::cout << "enabled" << std::endl;
  else                                 std::cout << "disabled" << std::endl;

  std::cout << "TapBridge:             ";
  if (niApiEnableTapBridge == true) std::cout << "enabled" << std::endl;
  else                              std::cout << "disabled" << std::endl;

  std::cout << "Logging:               ";
  if (niApiEnableLogging == true) std::cout << "enabled" << std::endl;
  else                            std::cout << "disabled" << std::endl;

  std::cout << "Remote control engine: ";
  if (niRemoteControlEnable == true) std::cout << "enabled" << std::endl;
  else                            std::cout << "disabled" << std::endl;

  std::cout << "LWA:                   ";
  if (lwaactivate==1)      std::cout << "partial (LTE+WiFi) activated" << std::endl;
  else if (lwaactivate==2) std::cout << "activated" << std::endl;
  else                     std::cout << "not activated" << std::endl;

  std::cout << "LWIP:                  ";
  if (lwipactivate==1) std::cout << "activated" << std::endl;
  else                 std::cout << "not activated" << std::endl;

  std::cout << "Client Server Config:  " << cientServerConfig << std::endl;

  std::cout << "NI Module Version:     " << NI_MODULE_VERSION << std::endl;
  std::cout << "Required AFW Version:  " << NI_AFW_VERSION << std::endl;

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
      LogFileName = "/tmp/Log_LteWifi_" + simStationType + ".txt";
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

  // p2p helper
  PointToPointHelper p2pHelp;
                     p2pHelp.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
                     p2pHelp.SetChannelAttribute ("Delay", StringValue ("2ms"));
                     p2pHelp.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  // csma helper
  CsmaHelper csmaHelp;
             csmaHelp.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
             csmaHelp.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // wifi helper
  Ssid       ssid = Ssid ("wifi-default");

  WifiHelper wifiHelp;
             wifiHelp.SetStandard (WIFI_PHY_STANDARD_80211b); // WIFI_PHY_STANDARD_80211ac does not work correctly - no assoc req/resp
             wifiHelp.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue (phyMode),
                                    "ControlMode", StringValue (phyMode));
  //if (verbose) wifiHelp.EnableLogComponents ();  // Turn on all Wifi logging

  NqosWifiMacHelper wifiApMacHelp, wifiStaMacHelp = NqosWifiMacHelper::Default(); // mac with disabled rate control
                    wifiApMacHelp.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
                    wifiStaMacHelp.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  // NI Wifi API Configuration
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
  Config::SetDefault ("ns3::NiWifiMacInterface::niApiWifiMcs", IntegerValue(niApiWifiMcs));

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

  // main router node to connect lan with wifi and lte mobile networks
  Ptr<Node> MobileNetworkGwNode = CreateObject<Node> ();
  // wifi station nodes
  NodeContainer wifiStaNodes;
                wifiStaNodes.Create (0);
  // wifi access point nodes - here fixed to one but can be extended
  NodeContainer wifiApNode;
                wifiApNode.Create (1);
  // lan nodes connected via ethernet - added mobile network gateway here
  NodeContainer LanNodes;
                LanNodes.Add (MobileNetworkGwNode);
                LanNodes.Create (2);
  // lte user terminal nodes
  NodeContainer ueNodes;
                ueNodes.Create(1);
  // lte enb nodes - here fixed to one but can be extended
  NodeContainer enbNodes;
                enbNodes.Create(1);

  // install corresponding net devices on all nodes
  NetDeviceContainer p2pWifiDevices  = p2pHelp.Install (MobileNetworkGwNode, wifiApNode.Get(0));
  NetDeviceContainer csmaDevices     = csmaHelp.Install (LanNodes);
  // note: all ue nodes have an lte and wifi net device
  NetDeviceContainer staDevices      = wifiHelp.Install (wifiPhyHelp, wifiStaMacHelp, NodeContainer(ueNodes, wifiStaNodes));
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
  Ipv4Address xwLwaSubnet     = "20.1.2.0";
  Ipv4Address xwLwipSubnet    = "20.1.3.0";

  // create wifi p2p link to mobile network gateway
  ipAddressHelp.SetBase (p2pWifiIpSubnet, IpMask);
  Ipv4InterfaceContainer p2pWifiIpInterfaces = ipAddressHelp.Assign (p2pWifiDevices);

  // create lan network with stations - sta0 is mobile network gateway
  ipAddressHelp.SetBase (csmaIpSubnet, IpMask);
  Ipv4InterfaceContainer LanIpInterfaces = ipAddressHelp.Assign (csmaDevices);
  Ipv4Address LanGwIpAddr = LanIpInterfaces.GetAddress (0);//"10.1.1.1";

  // create wifi network - ap0 is connected to mobile network gateway
  ipAddressHelp.SetBase (WifiIpSubnet, IpMask);
  Ipv4InterfaceContainer wifiIpInterfaces;
                         wifiIpInterfaces.Add (ipAddressHelp.Assign (apDevices));
                         wifiIpInterfaces.Add (ipAddressHelp.Assign (staDevices));
  Ipv4Address WifiGwIpAddr = wifiIpInterfaces.GetAddress (0);//"10.1.2.1";

  // ==========================
  // LWA/LWIP extensions by UOC

  // Switch to enable/disable LWA functionality (also partial use)
  Config::SetDefault ("ns3::LtePdcp::PDCPDecLwa", UintegerValue(lwaactivate));
  // Switch to enable/disable LWIP functionality (also partial use)
  Config::SetDefault ("ns3::LtePdcp::PDCPDecLwip", UintegerValue(lwipactivate));

  // Set LWA and LWIP values in the parameter database
  g_RemoteControlEngine.GetPdb()->setParameterLwaDecVariable(lwaactivate);
  g_RemoteControlEngine.GetPdb()->setParameterLwipDecVariable(lwipactivate);

  // create p2p helper for LWA/Xw link
  PointToPointHelper p2pHelpXw;
                     p2pHelpXw.SetDeviceAttribute ("DataRate", StringValue (xwLwaLinkDataRate));
                     p2pHelpXw.SetChannelAttribute ("Delay", StringValue (xwLwaLinkDelay));
                     p2pHelpXw.SetDeviceAttribute ("Mtu", UintegerValue (xwLwaLinkMtuSize));

  // create lwaap node
  Ptr<Node> lwaapNode = CreateObject<Node> ();
  // create lwaap node
  Ptr<Node> lwipepNode = CreateObject<Node> ();

  // install p2p net devices to create parallel lwa and lwip links
  NetDeviceContainer xwLwaDevices  = p2pHelpXw.Install (lwaapNode, wifiApNode.Get(0));
  NetDeviceContainer xwLwipDevices = p2pHelpXw.Install (lwipepNode, wifiApNode.Get(0));

  // install ip stack on additional nodes
  ipStackHelp.Install(lwaapNode);
  ipStackHelp.Install(lwipepNode);

  // assign ip adresses to lwa / lwip links
  Ipv4AddressHelper XwAddress;
  XwAddress.SetBase (xwLwaSubnet, "255.255.255.0");
  Ipv4InterfaceContainer xwLwaIpInterfaces = XwAddress.Assign (xwLwaDevices);
  XwAddress.SetBase (xwLwipSubnet, "255.255.255.0");
  Ipv4InterfaceContainer xwLwipIpInterfaces = XwAddress.Assign (xwLwipDevices);

  // create socket on lwaap socket to enable transmission of lwa packets
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  lwaapTxSocket = Socket::CreateSocket (lwaapNode, tid);
  lwaapTxSocket->Bind ();
  lwaapTxSocket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(wifiIpInterfaces.GetAddress (1)), 9));
  lwaapTxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  lwaapTxSocket->SetAllowBroadcast (true);

  // create socket on lwipep socket to enable transmission of lwip packets
  lwipepTxSocket = Socket::CreateSocket (lwipepNode, tid);
  lwipepTxSocket->Bind ();
  lwipepTxSocket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(wifiIpInterfaces.GetAddress (1)), 9));
  lwipepTxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  lwipepTxSocket->SetAllowBroadcast (true);

  // schedule function call to connect locally defined Lte_pdcpDlTxPDU traceback function to LTE PDCP Tx
  // Important: First function call call needs to be scheduled AFTER initial attach procedure is finished successfully.
  //            Thats way a callbackStart delay is added here as a dirty hack
  // TODO-NI: find a better solution to connect callback to PDCP (without timing constaint)
  const uint32_t assumedLteAttachDelayMs = 1000;
  if ((transmTime * 1000) < assumedLteAttachDelayMs) {
      // limit start time for packet generation
      transmTime = assumedLteAttachDelayMs / 1000;
      NI_LOG_CONSOLE_DEBUG("StartTime for packet generation too small! Adapted to: " << transmTime << " second(s)");
  }
  // start call back 100ms before datageneration
  uint32_t callbackStartMs = uint32_t (transmTime * 1000 - 100);
  Simulator::Schedule (MilliSeconds(callbackStartMs),&Callback_LtePDCPTX);

  // initialize routing database and set up the routing tables in the nodes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // initiate IPsec tunnel for LWIP link
  // Note: needs to be called after PopulateRoutingTables
  // Note: here the first UE is selected as end point for the IPSec tunnel
  Tunnel IPSec_tunnel (lwipepNode, ueNodes.Get (0), xwLwipIpInterfaces.GetAddress (0), wifiIpInterfaces.GetAddress (1));

  // ===================================================================================================================
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
      //StaticRouting->SetDefaultRoute (Ipv4Address (WifiGwIpAddr), 1);
      //StaticRouting->AddNetworkRouteTo (Ipv4Address (WifiIpSubnet), Ipv4Mask (IpMask),  Ipv4Address (WifiGwIpAddr), 1);
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
      StaticRouting->SetDefaultRoute (Ipv4Address (WifiGwIpAddr), 1);
      //StaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"),  Ipv4Address (WifiGwIpAddr), 1);
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


  // define client and server nodes

  // user always LAN node as client
  NodeContainer ClientNode     = LanNodes.Get (1);
  Ipv4Address   ClientIpAddr   = LanIpInterfaces.GetAddress (1);
  NodeContainer ServerNode     = LanNodes.Get (2);
  Ipv4Address   ServerIPAddr   = LanIpInterfaces.GetAddress (2);

  switch (cientServerConfig) {
    case 2:
      // UE station #1
      ServerNode     = ueNodes.Get (0);
      ServerIPAddr   = ueIpInterfaces.GetAddress (0);
      break;
    case 3:
      // UE station #1 but Wifi netdevice
      ServerNode     = ueNodes.Get (0);
      ServerIPAddr   = wifiIpInterfaces.GetAddress (1);
      break;
      //case 4:
      // wifi sta #1
      //  ServerNode     = wifiStaNodes.Get (0);
      //  ServerIPAddr   = wifiIpInterfaces.GetAddress (2);
      //  break;
    default: //case 1:
      // LAN node in same network
      ServerNode     = LanNodes.Get (2);
      ServerIPAddr   = LanIpInterfaces.GetAddress (2);
  }

  // print addresses
  std::cout << std::endl;
  std::cout << "-------- NS-3 Topology Information ------" << std::endl;
  std::cout << "Number of ETH devices      = " << LanIpInterfaces.GetN() << std::endl;
  std::cout << "Number of WiFi devices     = " << wifiIpInterfaces.GetN() << std::endl;
  std::cout << "Number of LTE UE devices   = " << ueIpInterfaces.GetN() << std::endl;
  std::cout << "Router GW IP Addr          = " << LanGwIpAddr << std::endl;
  std::cout << "WiFi Net GW IP Addr        = " << p2pWifiIpInterfaces.GetAddress(1) << std::endl;
  std::cout << "WiFi AP IP Addr            = " << WifiGwIpAddr << std::endl;
  std::cout << "WiFI STA#1 IP Addr         = " << wifiIpInterfaces.GetAddress(1) << std::endl;
  std::cout << "LTE Net GW IP Addr         = " << p2pLteIpInterfaces.GetAddress(1) << std::endl;
  std::cout << "LTE EPC PGW IP Addr        = " << PacketGwNode->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal () << std::endl;
  std::cout << "LTE UE#1 IP Addr           = " << ueIpInterfaces.GetAddress(0) << std::endl;
  std::cout << "Xw LWA IP Addr             = " << xwLwaIpInterfaces.GetAddress (0) << std::endl;
  std::cout << "Xw LWIP IP Addr            = " << xwLwipIpInterfaces.GetAddress (0) << std::endl;
  std::cout << "Client IP Addr             = " << ClientIpAddr << std::endl;
  std::cout << "Server IP Addr             = " << ServerIPAddr << std::endl;
  std::cout << std::endl;

  // include application
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

          //Tap Bridge for STA (Wifi)
          std::cout <<"Install TAP bridge for Sta host IP node..." <<std::endl;
          TapBridgeHelper tapBridgeSta;
          std::string modeSta = "ConfigureLocal";
          std::string tapNameSta = "NIAPI_TapSTA";
          tapBridgeSta.SetAttribute("Mode", StringValue(modeSta));
          tapBridgeSta.SetAttribute("DeviceName", StringValue(tapNameSta));
          tapBridgeSta.SetAttribute("Mtu", UintegerValue(1500));
          tapBridgeSta.Install(ueNodes.Get(0), staDevices.Get(0));
      }
    }

  // PCAP debugging
  // p2pHelp.EnablePcapAll("lte_wifi_plus_lwa_lwip");
  // p2pHelp.EnablePcap("lte_wifi_p2p", p2pWifiDevices.Get(0), true);
  // csmaHelp.EnablePcapAll ("lte_wifi_csma", true);

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

  Simulator::Destroy ();

  // check received packets
  if ((niApiDevMode == "NIAPI_TS")||(niApiDevMode == "NIAPI_BSTS"))
    {
      NI_LOG_CONSOLE_INFO ("Received packets: " << appServer->GetReceived()
                           << " / Lost packets: " << packetNum-appServer->GetReceived()
                           << "\n");
    }

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

