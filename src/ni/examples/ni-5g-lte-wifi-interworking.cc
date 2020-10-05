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
// DALI includes
#include "ns3/dali-module.h"
#include "ns3/dali-lte-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NiLwaLwipExt");

void DualConnectivityLauncher (Ptr<LteEnbNetDevice> MeNB, bool niRemoteControlEnable)
{
  // Specific scenario allows to set static values
  const uint16_t rnti = 1;
  const uint16_t secondaryRnti = 1;
  const uint16_t secondaryCellId = 2;

  if (niRemoteControlEnable)
    {
      if (g_RemoteControlEngine.GetPdb()->getParameterDcLaunchEnable() == true)
        {
          // DC triggered externaly via remote control engine
          //Trigger Dual Configuration Setup on MeNB
          MeNB->GetRrc()->DoRecvRrcSecondaryCellDualConnectivityRequest(rnti, secondaryRnti, secondaryCellId);
          std::cout << "\n\nDALI Dual Connectivity lauched via RC engine: \n\n";
          fflush( stdout );
          return;
        }
      else
        {
          // Schedule Dual Connectivity Setup launcher again to poll getParameterDcLaunchEnable()
          Simulator::Schedule (Seconds (1.0), &DualConnectivityLauncher, MeNB, niRemoteControlEnable);
        }
    }
  else // default case
    {
      //Trigger Dual Configuration Setup on MeNB
      MeNB->GetRrc()->DoRecvRrcSecondaryCellDualConnectivityRequest(rnti, secondaryRnti, secondaryCellId);
      std::cout << "\n\nDALI Dual Connectivity lauched: \n\n";
    }
}

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
  // 1 = server running on a LAN node in same network
  // 2 = UE Terminal Node, server running on LTE UE NetDevice
  // 3 = UE Terminal Node, server running on WIFI NetDevice
  int cientServerConfig = 2;

  // number of station nodes in infrastructure mode
  uint32_t nLteUeNodes = 1;
  uint32_t nLteEnbNodes = 1;

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
  /*
   * chose whether ns-3 instance should run as NIAPI_BS or NIAPI_TS, NIAPI_BSTS used for simulation mode
   *
   * DALI additional options: M (Master), S (Secondary), BS (Base Station), TS (Terminal Station)
   * - NIAPI_BSTS 									(simulated channel, one instance)
   * - NIAPI_MBSTS, NIAPI_SBSTS, 					(simulated channel, two instances)
   * - NIAPI_MBS, NIAPI_MTS, NIAPI_SBS, NIAPI_STS	(NIAPI enabled, four instances)
   */
  std::string niApiDevMode = "NIAPI_BSTS";

  std::string niAfwVersion = NI_AFW_VERSION; // for LTE/WIFI we use 2.2, for 5G-GFDM we use 2.5 see below

  // ====================
  // NI API 5G parameter

  // Activate additional NI API Messages for 5G
  bool niApiLte5gEnabled = false;

  // 5G DL subccarrier spacing. 0=15kHz, 1=30kHz, 2=60kHz. 3=120kHz
  uint32_t dlscs = 0;
  // 5G UL subccarrier spacing
  uint32_t ulscs = 0;
  //SFN to make a change in the SCS value used.
  uint32_t ScsSwitchTargetSfn=0;

  // ====================
  // DALI DualConnectivity parameter

  // Activate DALI Dual Connectivity
  bool daliDualConnectivityEnabled = false;
  // Set File Descriptor Device Name
  std::string fdDeviceName = "eth1";
  // Starting time in seconds for DALI Dual Connectivity Setup
  double dualConnectivityLaunchTime = 5.2;
  // Activate PCDP in-Sequence reordering function
  bool usePdcpInSequenceDelivery = false;

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
  // IP address & Port of ns-3 Wifi MAC API on which packets are sent to NI 802.11 Application Framework PHY (e.g. PXI device)
  std::string niApiWifiSta1RemoteIpAddrTx("127.0.0.1");           // remote address to be used for TX socket opening
  std::string niApiWifiSta1RemotePortTx("12101");                 // remote port to be used for TX socket opening
  std::string niApiWifiSta1LocalPortRx("12701");                  // local port to be used for RX socket opening
  // IP address & Port of ns-3 Wifi MAC API on which packets are received from NI 802.11 Application Framework PHY (e.g. PXI device)
  std::string niApiWifiSta2RemoteIpAddrTx("127.0.0.1");           // remote address to be used for TX socket opening
  std::string niApiWifiSta2RemotePortTx("12102");                 // remote port to be used for TX socket opening
  std::string niApiWifiSta2LocalPortRx("12702");                  // local port to be used for RX socket opening
  // MAC address configurations for 802.11 AFW
  std::string niApiWifiSta1MacAddr("46:6F:4B:75:6D:61");
  std::string niApiWifiSta2MacAddr("46:6F:4B:75:6D:62");
  std::string niApiWifiBssidMacAddr("46:6F:4B:75:6D:61");
  // IP address & Port of ns-3 Lte PHY API on which packets are sent to PHY (e.g. PXI device)
  std::string niApiLteEnbRemoteIpAddrTx("127.0.0.1");             // remote address to be used for TX socket opening
  std::string niApiLteEnbRemotePortTx("12802");                   // remote port to be used for TX socket opening
  std::string niApiLteEnbLocalPortRx("12801");                    // local port to be used for RX socket opening
  // IP address & Port of ns-3 Lte PHY API on which packets are received from PHY (e.g. PXI device)
  std::string niApiLteUeRemoteIpAddrTx("127.0.0.1");              // remote address to be used for TX socket opening
  std::string niApiLteUeRemotePortTx("12801");                    // remote port to be used for TX socket opening
  std::string niApiLteUeLocalPortRx("12802");                     // local port to be used for RX socket opening
  // MCS used by 802.11 AFW
  uint32_t niApiWifiMcs(5);
  //
  double rss = -80; // -dBm
  //
  std::string phyRate = "VhtMcs8";        // PHY bitrate

  // =======================
  // UOC LWIP/LWA extensions

  uint32_t lwaactivate=0;     // disabled=0, LTE+Wifi=1, Wi-Fi=2
  uint32_t lwipactivate=0;    // disabled=0, if 1 all packets will be transmitted via LWIP
  uint32_t dcactivate=0;      // LTE=0, LTE+5G=1, 5G=2
  uint32_t dcLwaLwipSwitch=1; // 0=DC exclusiv, 1=LWA/LWIP possible when DC is enabled

  // MTU size for P2P link between EnB and Wifi AP
  uint32_t xwLwaLinkMtuSize=1500;
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
  cmd.AddValue("cientServerConfig", "Client Server Configuration (Server on 1=LAN Node, 2=UE Node LTE, 3=UE Node WIFI)", cientServerConfig);
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
  cmd.AddValue("niApiLteEnbRemoteIpAddrTx", "Remote IP address for UDP socket on eNB", niApiLteEnbRemoteIpAddrTx);
  cmd.AddValue("niApiLteEnbRemotePortTx", "Remote port for UDP socket on eNB", niApiLteEnbRemotePortTx);
  cmd.AddValue("niApiLteEnbLocalPortRx", "Local RX port of eNB", niApiLteEnbLocalPortRx);
  cmd.AddValue("niApiLteUeRemoteIpAddrTx", "Remote IP address for UDP socket on UE", niApiLteUeRemoteIpAddrTx);
  cmd.AddValue("niApiLteUeRemotePortTx", "Remote port for UDP socket on UE", niApiLteUeRemotePortTx);
  cmd.AddValue("niApiLteUeLocalPortRx", "Local RX port of UE", niApiLteUeLocalPortRx);
  cmd.AddValue("lwaactivate", "Activate LWA interworking (disabled=0, LTE+Wifi=1, Wi-Fi=2)", lwaactivate);
  cmd.AddValue("lwipactivate", "Activate LWIP interworking (disabled=0, Wi-Fi=1)", lwipactivate);
  cmd.AddValue("dcactivate", "Activate DC interworking (LTE=0, LTE+5G=1, 5G=2)", dcactivate);
  // ===================================================
  cmd.AddValue("niApiLte5gEnabled", "Enable additional NI API Messages for 5G", niApiLte5gEnabled);
  cmd.AddValue("setDlScs", "Set initial value for the DL Subcarrier Spacing (SCS). 0=15kHz, 1=30kHz, 2=60kHz. 3=120kHz", dlscs);
  cmd.AddValue("setUlScs", "Set initial value for the UL Subcarrier Spacing (SCS). 0=15kHz, 1=30kHz, 2=60kHz. 3=120kHz", ulscs);
  cmd.AddValue("nextScsSwitchTargetSfn", "Set next SFN to switch the value ",ScsSwitchTargetSfn);
  // ===================================================
  cmd.AddValue("daliDualConnectivityEnabled", "Enable/disable DALI Dual Connectivity configuration", daliDualConnectivityEnabled);
  cmd.AddValue("fdDeviceName", "Interface name for emu FdDevice communication", fdDeviceName);
  cmd.AddValue("dualConnectivityLaunchTime", "Time in seconds when DALI Dual Connectivity packet transmission should be scheduled", dualConnectivityLaunchTime);
  cmd.AddValue("usePdcpInSequenceDelivery", "Enable/disable DALI PDCP In-Sequence reordering function", usePdcpInSequenceDelivery);
  // ===================================================
  cmd.Parse (argc, argv);

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
  if (!daliDualConnectivityEnabled) // daliDualConnectivityEnabled = false
    if (niApiDevMode == "NIAPI_BS")
      {
        // if ns-3 instance is configured as base station it acts as eNB / AP
        niApiLteDevMode  = "NIAPI_ENB";
        niApiWifiDevMode = "NIAPI_AP";
        simStationType   = "BS";
      }
    else if (niApiDevMode == "NIAPI_TS")
      {
        // if ns-3 instance is configured as base station it acts as UE / STA
        niApiLteDevMode  = "NIAPI_UE";
        niApiWifiDevMode = "NIAPI_STA";
        simStationType   = "TS";
      }
    else if (niApiDevMode == "NIAPI_BSTS")
      {
        // ns-3 instance is configures as BS and TS- for debugging if only spectrum phy shall be bypassed
        niApiLteDevMode  = "NIAPI_ALL";
        niApiWifiDevMode = "NIAPI_ALL";
        simStationType   = "BSTS";
      }
    else
    {
      NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed");
    }
  else  // daliDualConnectivityEnabled = true
    {
	  if (niApiDevMode == "NIAPI_MBS" && niApiLteEnabled)
	    {
          // if ns-3 instance is configured as base station it acts as MeNB
          niApiLteDevMode  = "NIAPI_ENB";
          niApiWifiDevMode = "NIAPI_AP";
		  simStationType   = "MBS";
		}
	  else if (niApiDevMode == "NIAPI_MTS" && niApiLteEnabled)
	    {
          // if ns-3 instance is configured as terminal station it acts as MUE
          niApiLteDevMode  = "NIAPI_UE";
          niApiWifiDevMode = "NIAPI_STA";
          simStationType   = "MTS";
		}
	  else if (niApiDevMode == "NIAPI_SBS" && niApiLteEnabled)
	    {
          // if ns-3 instance is configured as base station it acts as SeNB
          niApiLteDevMode  = "NIAPI_ENB";
          niApiWifiDevMode = "NIAPI_AP";
          simStationType   = "SBS";
		}
	  else if (niApiDevMode == "NIAPI_STS" && niApiLteEnabled)
	    {
          // if ns-3 instance is configured as terminal station it acts as SUE
          niApiLteDevMode  = "NIAPI_UE";
          niApiWifiDevMode = "NIAPI_STA";
          simStationType   = "STS";
		}
	  else if (niApiDevMode == "NIAPI_BSTS" && !niApiLteEnabled)
	    {
          // ns-3 instance is configures as BS and TS (Master and Secondary)- for debugging if only spectrum phy shall be bypassed
          niApiLteDevMode  = "NIAPI_ALL";
          niApiWifiDevMode = "NIAPI_ALL";
          simStationType   = "BSTS";
		}
	  else if (niApiDevMode == "NIAPI_MBSTS" && !niApiLteEnabled)
	    {
          // ns-3 instance is configures as MBS and STS- for debugging if only spectrum phy shall be bypassed
          niApiLteDevMode  = "NIAPI_ALL";
          niApiWifiDevMode = "NIAPI_ALL";
          simStationType   = "MBSTS";
		}
	  else if (niApiDevMode == "NIAPI_SBSTS" && !niApiLteEnabled)
	    {
          // ns-3 instance is configures as SBS and STS- for debugging if only spectrum phy shall be bypassed
          niApiLteDevMode  = "NIAPI_ALL";
          niApiWifiDevMode = "NIAPI_ALL";
          simStationType   = "SBSTS";
		}
	  else
	    {
		  if (niApiLteEnabled)
            NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed for DALI Dual Connectivity and NI API for LTE enabled");
		  else
            NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed for DALI Dual Connectivity and NI API for LTE disabled");
		}
    }

  // for 5G-GFDM we use AFW 2.5
  if (niApiLteEnabled && niApiLte5gEnabled)
    niAfwVersion = NI_AFW_VERSION_5G_GFDM;

  // NOTE: check for dali DC and LWA/LWIP misconfiguration
  // NOTE: LWA+DC allowed, LWIP+DC not allowed because of unsolved conflicts
  if (daliDualConnectivityEnabled && lwipactivate > 0)
    NS_FATAL_ERROR ("dual connectivity and LWIP are not allowed to enable simultaneously for now");

  // check and apply 802.11 API settings
  if (niApiWifiSta1RemoteIpAddrTxTmp.empty() || niApiWifiSta1RemotePortTxTmp.empty() || niApiWifiSta1LocalPortRxTmp.empty())
    {
      if (niApiWifiLoopbackEnabled)
        {
          std::cout << "INFO: 802.11 API UDP loop back settings where empty or incomplete, use defaults" << std::endl;
          // apply default settings for local loop back
          niApiWifiSta1RemotePortTx = "12701";
          niApiWifiSta1LocalPortRx = "12702";
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
        if (niApiWifiLoopbackEnabled)
          {
            std::cout << "INFO: 802.11 API UDP loop back settings where empty or incomplete, use defaults" << std::endl;
            // apply default settings for local loop back
            niApiWifiSta2RemotePortTx = "12702";
            niApiWifiSta2LocalPortRx = "12701";
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

  std::cout << "WiFi UDP Loopback:     ";
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

  std::cout << "5G API extensions:     ";
  if (niApiLte5gEnabled == true) std::cout << "enabled" << std::endl;
  else                           std::cout << "disabled" << std::endl;

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

  std::cout << "DALI DC (RRC):         ";
  if (daliDualConnectivityEnabled) std::cout << "enabled" << std::endl;
  else                             std::cout << "disabled" << std::endl;

  std::cout << "DALI DC (PDCP):        ";
  if (dcactivate==1)      std::cout << "partial (LTE+5G/LTE) activated" << std::endl;
  else if (dcactivate==2) std::cout << "activated" << std::endl;
  else                     std::cout << "not activated" << std::endl;

  std::cout << "Client Server Config:  " << cientServerConfig << std::endl;

  std::cout << "NI Module Version:     " << NI_MODULE_VERSION << std::endl;
  std::cout << "Required AFW Version:  " << niAfwVersion << std::endl;



  std::cout << std::endl;

  NI_LOG_CONSOLE_INFO ("Init NI modules ...");
  // deriving thread priorities from ns3 main process
  int ns3Priority = NiUtils::GetThreadPrioriy();
  const int niLoggingPriority = ns3Priority - 10;
  const int niRemoteControlPriority = ns3Priority - 11;

  // adding thread ID of main NS3 thread for possible troubleshooting
  NiUtils::AddThreadInfo("NS3 main thread");

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
             wifiHelp.SetStandard (WIFI_PHY_STANDARD_80211ac);
             wifiHelp.SetRemoteStationManager ("ns3::ConstantRateWifiManager");
  //if (verbose) wifiHelp.EnableLogComponents ();  // Turn on all Wifi logging

  NqosWifiMacHelper wifiApMacHelp, wifiStaMacHelp = NqosWifiMacHelper::Default(); // mac with disabled rate control
                    wifiApMacHelp.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
                    wifiStaMacHelp.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

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
  // Set IP address & Port for ENB and UE
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLteEnbRemoteIpAddrTx", StringValue ( niApiLteEnbRemoteIpAddrTx ));
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLteEnbRemotePortTx", StringValue ( niApiLteEnbRemotePortTx.c_str() ));
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLteEnbLocalPortRx", StringValue ( niApiLteEnbLocalPortRx.c_str() ));
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLteUeRemoteIpAddrTx", StringValue ( niApiLteUeRemoteIpAddrTx.c_str() ));
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLteUeRemotePortTx", StringValue ( niApiLteUeRemotePortTx.c_str() ));
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLteUeLocalPortRx", StringValue ( niApiLteUeLocalPortRx.c_str() ));

  YansWifiChannelHelper wifiChannelHelp = YansWifiChannelHelper::Default ();
                        wifiChannelHelp.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
                        wifiChannelHelp.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  YansWifiPhyHelper     wifiPhyHelp =  YansWifiPhyHelper::Default ();
                        wifiPhyHelp.Set ("RxGain", DoubleValue (0) );
                        wifiPhyHelp.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
                        wifiPhyHelp.SetChannel (wifiChannelHelp.Create ());

  MobilityHelper LteMobilityHelp;
                 LteMobilityHelp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // second mobility helper for WIFI to keep a short distance of STA to AP also in the DC case
  MobilityHelper WifiMobilityHelp;
                 WifiMobilityHelp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  if (daliDualConnectivityEnabled)
    {
      // number of station nodes in infrastructure mode need to be changed for DALI
      nLteUeNodes = 2;
      nLteEnbNodes = 2;
      /*
       * DALI uses Position Allocator to force eNB connection with correspondent UE (Master and Secondary)
       * and reduce interference in simulated physical channel
       */
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      double distance = 10000.0;
      for (uint16_t i = 0; i < nLteUeNodes; i++)
        {
          positionAlloc->Add (Vector(distance * i, 0, 0));
        }
      LteMobilityHelp.SetPositionAllocator(positionAlloc);
    }

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
                ueNodes.Create(nLteUeNodes);
  // lte enb nodes - here fixed to one but can be extended
  NodeContainer enbNodes;
                enbNodes.Create(nLteEnbNodes);

  // install corresponding net devices on all nodes
  NetDeviceContainer p2pWifiDevices  = p2pHelp.Install (MobileNetworkGwNode, wifiApNode.Get(0));
  NetDeviceContainer csmaDevices     = csmaHelp.Install (LanNodes);
  // note: only the first UE node has wifi device, in DC case secondary UE node doesn't have wifi device
  NetDeviceContainer staDevices      = wifiHelp.Install (wifiPhyHelp, wifiStaMacHelp, NodeContainer(ueNodes.Get(0), wifiStaNodes));
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
  Ipv4Address epcIpSubnet     = "11.0.0.0";
  Ipv4Address mmeIpSubnet     = "12.0.0.0";
  Ipv4Address x2IpSubnet      = "13.0.0.0";
  Ipv4Address dcxIpSubnet     = "14.0.0.0";

  uint16_t    DestPort        = 9;   // Discard port (RFC 863)

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


  if (daliDualConnectivityEnabled)
    {
      // Switch to enable/disable LWA functionality (also partial use)
      Config::SetDefault ("ns3::DaliEnbPdcp::PDCPDecLwa", UintegerValue(lwaactivate));
      // Switch to enable/disable LWIP functionality
      Config::SetDefault ("ns3::DaliEnbPdcp::PDCPDecLwip", UintegerValue(lwipactivate));
      // Switch to enable/disable DC functionality
      Config::SetDefault ("ns3::DaliEnbPdcp::PDCPDecDc", UintegerValue(dcactivate));
      // Switch to allow LWA/LWIP when DC is enabled
      Config::SetDefault ("ns3::DaliEnbPdcp::DcLwaLwipSwitch", UintegerValue(dcLwaLwipSwitch));
      // Set DC value in the parameter database
      g_RemoteControlEngine.GetPdb()->setParameterDcDecVariable(dcactivate);
    }

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
  Ptr<Socket> lwaapTxSocket;
  Ptr<Socket> lwipepTxSocket;
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  lwaapTxSocket = Socket::CreateSocket (lwaapNode, tid);
  lwaapTxSocket->Bind ();
  lwaapTxSocket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(wifiIpInterfaces.GetAddress (1)), DestPort));
  lwaapTxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  lwaapTxSocket->SetAllowBroadcast (true);

  // create socket on lwipep socket to enable transmission of lwip packets
  lwipepTxSocket = Socket::CreateSocket (lwipepNode, tid);
  lwipepTxSocket->Bind ();
  lwipepTxSocket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(wifiIpInterfaces.GetAddress (1)), DestPort));
  lwipepTxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  lwipepTxSocket->SetAllowBroadcast (true);

  // setup handler for LWA/LWIP functionality
  LwaLwipHandler lwaLwipHandler(lwaapTxSocket, lwipepTxSocket, niApiEnableTapBridge, wifiIpInterfaces.GetAddress(1));

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
  Simulator::Schedule (MilliSeconds(callbackStartMs),&LwaLwipHandler::Callback_LtePDCPTX, &lwaLwipHandler);

  // initialize routing database and set up the routing tables in the nodes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // initiate IPsec tunnel for LWIP link
  // Note: needs to be called after PopulateRoutingTables
  // Note: here the first UE is selected as end point for the IPSec tunnel
  // Note: LWIP and dualconnectivity have unsolved conflicts, so LWIP can be used only if DC is disabled
  if (!daliDualConnectivityEnabled)
    Tunnel IPSec_tunnel (lwipepNode, ueNodes.Get (0), xwLwipIpInterfaces.GetAddress (0), wifiIpInterfaces.GetAddress (1));

  // ============================================
  // setup 5G values

  // Enable/ disable the 5G API
  Config::SetDefault ("ns3::NiLtePhyInterface::niApiLte5gEnabled",BooleanValue(niApiLte5gEnabled));
  // Use simple round robin scheduler here (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
  std::string LteMacSchedulerType = "ns3::RrFfMacScheduler";
  // Enables the SCS configuration and the GFDM Complaint Round Robin MAC Scheduler only if both
  // are true, meaning that GFDM transmission is desired.
  if (niApiLteEnabled && niApiLte5gEnabled)
    {
      // Set the DL SCS for the ni Phy.
      Config::SetDefault ("ns3::NiLtePhyInterface::niDlScs", UintegerValue (dlscs));
      // Set the UL SCS for the ni Phy.
      Config::SetDefault ("ns3::NiLtePhyInterface::niUlScs", UintegerValue (ulscs));
      // Use simple round robin scheduler with GFDM specific PRB allocation patterns
      LteMacSchedulerType = "ns3::NiGfdmRrFfMacScheduler";
      // Set the SFN to perform SCS change.
      Config::SetDefault ("ns3::NiLtePhyInterface::nextScsSwitchTargetSfn" , UintegerValue(ScsSwitchTargetSfn));
    }
  // Set AFW version to Pipe Tansport layer for named pipe setup
  Config::SetDefault ("ns3::NiPipeTransport::niAfwVersion" , StringValue(niAfwVersion));

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
  Config::SetDefault ("ns3::DaliLteHelper::UseIdealRrc", BooleanValue (false));
  // Disable CQI measurement reports based on PDSCH (based on ns-3 interference model)
  Config::SetDefault ("ns3::DaliLteHelper::UsePdschForCqiGeneration", BooleanValue (false));
  // Use simple round robin scheduler; for GFDM with specific PRB allocation patterns
  Config::SetDefault ("ns3::DaliLteHelper::Scheduler", StringValue (LteMacSchedulerType));

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

  // epc helper is used to create core network incl installation of eNB App, DALI DC new EPC helpers implemented
  Ptr<EpcHelper>  epcHelper;
  if (!daliDualConnectivityEnabled)
    {
      epcHelper = CreateObject<PointToPointEpcHelper> ();
    }
  else if (niApiDevMode == "NIAPI_BSTS")
    epcHelper = CreateObject<DaliEmuOneInstanceEpcHelper> ();
  else
    {
	  epcHelper = CreateObject<DaliEmuSeparatedInstancesEpcHelper> ();
	  epcHelper->SetAttribute ("niApiDevMode", StringValue (niApiDevMode));
    }

  // lte helper is used to install L2/L3 on eNB/UE (PHY,MAC,RRC->PDCP/RLC, eNB NetDevice), DALI DC uses a specific LTE Helper
  Ptr<DaliLteHelper> lteHelper = CreateObject<DaliLteHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  // Use simple round robin scheduler here
  // (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
  lteHelper->SetSchedulerType (LteMacSchedulerType);

  if (daliDualConnectivityEnabled)
    {
	  lteHelper->SetAttribute ("isDaliDualConnectivity", BooleanValue (daliDualConnectivityEnabled));
	  lteHelper->SetAttribute ("niApiDevMode", StringValue (niApiDevMode));
	  // Enable/Disable PCDP in-Sequence reordering function
	  lteHelper->SetAttribute ("usePdcpInSequenceDelivery", BooleanValue (usePdcpInSequenceDelivery));
	  //Set up FdNetDevice name for every DALI EPC/LTE interfaces
      epcHelper->SetAttribute ("sgwDeviceName", StringValue (fdDeviceName));
      epcHelper->SetAttribute ("mmeDeviceName", StringValue (fdDeviceName));
      epcHelper->SetAttribute ("enbS1uDeviceName", StringValue (fdDeviceName));
      epcHelper->SetAttribute ("enbS1MmeDeviceName", StringValue (fdDeviceName));
      epcHelper->SetAttribute ("enbX2DeviceName", StringValue (fdDeviceName));
      epcHelper->SetAttribute ("ueDcxDeviceName", StringValue (fdDeviceName));
      //Set ip address spaces for the different DALI subnetworks
      epcHelper->SetAttribute ("IpMask", Ipv4MaskValue (IpMask));
      epcHelper->SetAttribute ("ueIpSubnet", Ipv4AddressValue (UeIpSubnet));
      epcHelper->SetAttribute ("epcIpSubnet", Ipv4AddressValue (epcIpSubnet));
      epcHelper->SetAttribute ("mmeIpSubnet", Ipv4AddressValue (mmeIpSubnet));
      epcHelper->SetAttribute ("x2IpSubnet", Ipv4AddressValue (x2IpSubnet));
      epcHelper->SetAttribute ("dcxIpSubnet", Ipv4AddressValue (dcxIpSubnet));
      epcHelper->Initialize ();
    }

  Ptr<Node> PacketGwNode = epcHelper->GetPgwNode ();
  // create lte p2p link to mobile network gateway - work around for the global/static routing problem
  NetDeviceContainer p2pLteDevices = p2pHelp.Install (PacketGwNode, MobileNetworkGwNode);
  // assign ip adresses for p2p link
  ipAddressHelp.SetBase (p2pLteIpSubnet, IpMask);
  Ipv4InterfaceContainer p2pLteIpInterfaces = ipAddressHelp.Assign (p2pLteDevices);

  // lte & wifi channel / mobility  model
  WifiMobilityHelp.Install (wifiStaNodes);
  WifiMobilityHelp.Install (wifiApNode);
  LteMobilityHelp.Install (enbNodes);
  LteMobilityHelp.Install (ueNodes);

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

  if (daliDualConnectivityEnabled)
    {
      // Connects eNBs using X2 links
      lteHelper->AddX2Interface (enbNodes);
      // Connects UEs using DCX links
      lteHelper->AddDcxInterface (ueNodes);

      // Attach mUE and MeNB or sUE and SeNB
      uint16_t i = 0;
      uint16_t n = ueNodes.GetN ();
      if ((niApiDevMode == "NIAPI_MBS") || (niApiDevMode == "NIAPI_MTS"))
        {
          i = 0;
          n = 1;
        }
      if ((niApiDevMode == "NIAPI_SBS") || (niApiDevMode == "NIAPI_STS"))
        {
          i = 1;
          n = 2;
        }
      for (i; i < n; i++)
        lteHelper->Attach (ueDevices.Get(i), enbDevices.Get(i));
        // side effect: the default EPS bearer will be activated
    }
  else
    {
      // Attach one UE per eNodeB
      for (uint16_t i = 0; i < ueNodes.GetN (); i++)
        {
          lteHelper->Attach (ueDevices.Get(i), enbDevices.Get(i));
          // side effect: the default EPS bearer will be activated
        }
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

  if (verbose)
    {
      // ========================================================
      std::cout << "MobileNetworkGwNode ID     = " << MobileNetworkGwNode->GetId() << std::endl;
      std::cout << "PacketGwNode ID            = " << PacketGwNode->GetId() << std::endl;
      std::cout << "enbNodes ID                = " << enbNodes.Get(0)->GetId() << std::endl;
      std::cout << "wifiApNode ID              = " << wifiApNode.Get(0)->GetId() << std::endl;
      std::cout << "ueNode ID                  = " << ueNodes.Get(0)->GetId() << std::endl;

      std::cout << std::endl;
      std::cout << "LWA p2p device 0 Mac Addr  = " << xwLwaDevices.Get (0)->GetAddress() << std::endl;
      std::cout << "LWA p2p device 1 Mac Addr  = " << xwLwaDevices.Get (1)->GetAddress() << std::endl;
      std::cout << "WiFi AP device Mac Addr    = " << apDevices.Get (0)->GetAddress() << " ifID = " << apDevices.Get (0)->GetIfIndex() << std::endl;
      std::cout << "WiFi STA device Mac Addr   = " << staDevices.Get (0)->GetAddress() << " ifID = " << staDevices.Get (0)->GetIfIndex()<< std::endl;
      // ========================================================
    }
  // include application
  Ipv4Address  ClientDestAddr  = ServerIPAddr;
  // Convert to time object
  Time packetIntervalSec = Seconds(packetInterval*1E-03);

  Ptr<NiUdpServer> appServer;

  if (!niApiEnableTapBridge && !(niApiDevMode == "NIAPI_SBS" || niApiDevMode == "NIAPI_STS" || niApiDevMode == "NIAPI_SBSTS"))
    {
      // undirectional traffic - https://www.nsnam.org/doxygen/classns3_1_1_udp_client.html
      NiUdpServerHelper     ServerHelp (DestPort);
      NiUdpClientHelper     ClientHelp(ClientDestAddr, DestPort);
      //ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ClientDestAddr));
      //ClientHelp.SetAttribute ("RemotePort", UintegerValue (DestPort));
      ClientHelp.SetAttribute ("MaxPackets", UintegerValue (packetNum));
      ClientHelp.SetAttribute ("Interval", TimeValue (packetIntervalSec));
      ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));

      if ((niApiDevMode == "NIAPI_BS")||(niApiDevMode == "NIAPI_BSTS")||
          (niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS"))
        {
          ApplicationContainer clientApps = ClientHelp.Install (ClientNode);
          clientApps.Start (Seconds (transmTime));
          clientApps.Stop (Seconds (simTime));
        }

      ApplicationContainer serverApps = ServerHelp.Install (ServerNode);
      serverApps.Start (Seconds (1.0));
      serverApps.Stop (Seconds (simTime));

      appServer = ServerHelp.GetServer ();
    }
  else if (niApiEnableTapBridge)
    {
      if ((niApiDevMode == "NIAPI_BS")||(niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_SBS"))
        {
          std::cout << "Install TAP bridge for eNB (" << niApiDevMode << ") host IP node..." << std::endl;
          uint32_t idx = (niApiDevMode == "NIAPI_SBS") ? 1 : 0; // second eNB for SBS, else first eNB
          enbDevices.Get(idx)->SetAddress (Mac48Address::Allocate ());
          TapBridgeHelper tapBridgeENB;
          std::string modeENB = "ConfigureLocal";
          std::string tapNameENB = "NIAPI_TapENB";
          tapBridgeENB.SetAttribute ("Mode", StringValue (modeENB));
          tapBridgeENB.SetAttribute ("DeviceName", StringValue (tapNameENB));
          tapBridgeENB.SetAttribute ("Mtu", UintegerValue(1500) );
          // TODO for SBS the tapbridge needs to be setup differently
          tapBridgeENB.Install (LanNodes.Get(1), csmaDevices.Get(1)); // install to Host Node, works for BS/MBS
        }
      else if((niApiDevMode == "NIAPI_TS")||(niApiDevMode == "NIAPI_MTS")||(niApiDevMode == "NIAPI_STS"))
        {
          //Tap Bridge for UE (LTE)
          std::cout <<"Install TAP bridge for UE (" << niApiDevMode << ") host IP node..." <<std::endl;
          uint32_t idx = (niApiDevMode == "NIAPI_STS") ? 1 : 0; // second UE for STS, else first UE
          ueDevices.Get(idx)->SetAddress(Mac48Address::Allocate());
          TapBridgeHelper tapBridgeUE(Ipv4Address("7.0.0.2"));
          std::string modeUE = "ConfigureLocal";
          std::string tapNameUE = "NIAPI_TapUE";
          tapBridgeUE.SetAttribute("Mode", StringValue(modeUE));
          tapBridgeUE.SetAttribute("DeviceName", StringValue(tapNameUE));
          tapBridgeUE.SetAttribute("Mtu", UintegerValue(1500));
          tapBridgeUE.Install(ueNodes.Get(idx), ueDevices.Get(idx)); // install to UE

          if (niApiDevMode != "NIAPI_STS")
            {
              //Tap Bridge for STA (Wifi)
              std::cout <<"Install TAP bridge for STA (" << niApiDevMode << ") host IP node..." <<std::endl;
              TapBridgeHelper tapBridgeSta;
              std::string modeSta = "ConfigureLocal";
              std::string tapNameSta = "NIAPI_TapSTA";
              tapBridgeSta.SetAttribute("Mode", StringValue(modeSta));
              tapBridgeSta.SetAttribute("DeviceName", StringValue(tapNameSta));
              tapBridgeSta.SetAttribute("Mtu", UintegerValue(1500));
              tapBridgeSta.Install(ueNodes.Get(0), staDevices.Get(0));
            }
        }
    }

  // PCAP debugging
  // p2pHelp.EnablePcapAll("5g_lte_wifi_interworking");
  // p2pHelp.EnablePcap("xwLwaDevice(1)", xwLwaDevices.Get(1), true);
  // csmaHelp.EnablePcapAll ("lte_wifi_csma", true);

  // stop the simulation after simTime seconds
  Simulator::Stop(Seconds(simTime));

  if (niApiLteEnabled && !niApiLteLoopbackEnabled)
    {
      NI_LOG_CONSOLE_INFO ("\n--> Please enable now Rx/Tx in LTE Application Framework!");
    }

  //Get pointers to eNBs LteEnbNetDevice, allowing to access RRC instance.
  NetDevice* MeNB_dev;
  LteEnbNetDevice* MeNB;

  if (((niApiDevMode == "NIAPI_BSTS") && daliDualConnectivityEnabled)||(niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS"))
    {
      MeNB_dev = GetPointer (enbDevices.Get(0));
      MeNB = ( LteEnbNetDevice*)(MeNB_dev);
      // Schedule Dual Connectivity Setup launcher
      Simulator::Schedule (Seconds (dualConnectivityLaunchTime), &DualConnectivityLauncher, MeNB, niRemoteControlEnable);
    }

  std::cout << std::endl;
  std::cout << "[>] Start simulation" << std::endl;
  Simulator::Run ();
  std::cout << "[#] End simulation" << std::endl << std::endl;

  Simulator::Destroy ();

  // check received packets
  if (!niApiEnableTapBridge &&
      ((niApiDevMode == "NIAPI_TS")||(niApiDevMode == "NIAPI_BSTS")||
       (niApiDevMode == "NIAPI_MTS")||(niApiDevMode == "NIAPI_MBSTS")))
    {
      NI_LOG_CONSOLE_INFO ("Received packets: " << appServer->GetReceived()
                           << " / Lost packets: " << packetNum-appServer->GetReceived()
                           << "\n");
    }

  if (((niApiDevMode == "NIAPI_BSTS") && daliDualConnectivityEnabled)||(niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS"))
    MeNB_dev->Unref();

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

