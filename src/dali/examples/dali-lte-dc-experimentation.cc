/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 National Instruments
 * Copyright (c) 2019, Universitat Politecnica de Catalunya
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
 *      NI-API for LTE simple configuration
 *
 *         Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 *      Extension of DALI Dual Connectivity Configuration and Split Bearer between LTE eNBs, Throughput evaluation
 */

#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/config-store.h"

// NI includes
#include "ns3/ni-module.h"
// DALI includes
#include "ns3/dali-module.h"
#include "ns3/dali-lte-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DaliLteDcExperimentation");

void DualConnectivityLauncher (Ptr<LteEnbNetDevice> MeNB)
{
  // Specific scenario allows to set static values
  uint16_t rnti = 1;
  uint16_t secondaryRnti = 1;
  uint16_t secondaryCellId = 2;

  //Trigger Dual Configuration Setup on MeNB
  MeNB->GetRrc()->DoRecvRrcSecondaryCellDualConnectivityRequest(rnti, secondaryRnti, secondaryCellId);
  //std::cout << "\n\nDALI Dual Connectivity lauched: \n\n";
}

Ptr<PacketSink> sink;           /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0; 		/* The value of the last total received bytes */

void
CalculateThroughput ()
{
  const uint64_t calcIntervalMs = 500;
  const double calcFactorMbits = (double) 8 / (1e6 * (double) calcIntervalMs/1e3);
  Time now = Simulator::Now ();		                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * calcFactorMbits;      /* Convert Application RX Packets to MBits. */
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (calcIntervalMs), &CalculateThroughput);
}

int
main (int argc, char *argv[])
{

  bool verbose = false;
  bool pcapTracing = false;
  bool printRoutingTables = false;

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

   // ====================
   // DALI DualConnectivity parameter

   // Activate DALI Dual Connectivity
   bool daliDualConnectivityEnabled = false;
   // Set File Descriptor Device Name
   std::string fdDeviceName = "eth1";
   // Starting time in seconds for DALI Dual Connectivity Setup
   double dualConnectivityLaunchTime = 4;
   // Activate PCDP in-Sequence reordering function
   bool usePdcpInSequenceDelivery = false;
   // Selection of application for Evaluation ( UDP / TCP / NI)
   std::string daliTransportProtocol = "NI";
   // Set Link Direction for Evaluation ( DOWNLINK / UPLINK )
   std::string daliLinkDirection = "DOWNLINK";
   // Data transmission Rate (Application layer / desired rate)
   std::string dataRate = "60Mbps";
   // TCP application variant type
   std::string tcpVariant = "TcpNewReno";
   // dual connectivity configuration
   uint32_t dcactivate=1; // MeNB/UE=0, MeNB/UE+SeNB/UE=1 (split bearer), SeNB/UE=2

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("numPackets", "number of packets generated", packetNum);
  cmd.AddValue("interval", "interval (milliseconds) between packets", packetInterval);
  cmd.AddValue("simTime", "Duration in seconds which the simulation should run", simTime);
  cmd.AddValue("transmTime", "Time in seconds when the packet transmission should be scheduled", transmTime);
  cmd.AddValue("niApiEnableTapBridge", "Enable/disable TapBridge as external data source/sink", niApiEnableTapBridge);
  cmd.AddValue("niApiEnableLogging", "Set whether to enable NIAPI_DebugLogs", niApiEnableLogging);
  cmd.AddValue("niApiDevMode", "Set whether the simulation should run as Master or Secondary, and BS or Terminal", niApiDevMode);
  cmd.AddValue("niApiLteEnabled", "Enable NI API for LTE", niApiLteEnabled);
  cmd.AddValue("niApiLteLoopbackEnabled", "Enable/disable UDP loopback mode for LTE NI API", niApiLteLoopbackEnabled);
  cmd.AddValue("daliDualConnectivityEnabled", "Enable/disable DALI Dual Connectivity configuration", daliDualConnectivityEnabled);
  cmd.AddValue("fdDeviceName", "Interface name for emu FdDevice communication", fdDeviceName);
  cmd.AddValue("dualConnectivityLaunchTime", "Time in seconds when DALI Dual Connectivity packet transmission should be scheduled", dualConnectivityLaunchTime);
  cmd.AddValue("usePdcpInSequenceDelivery", "Enable/disable DALI PDCP In-Sequence reordering function", usePdcpInSequenceDelivery);
  cmd.AddValue("daliTransportProtocol", "Set whether the simulation should use, NI (application service), UDP or TCP as transport protocol", daliTransportProtocol);
  cmd.AddValue("daliLinkDirection", "Set simulation traffic direction, UPLINK / DOWNLINK", daliLinkDirection);
  cmd.AddValue("dataRate", "Data transmission Rate (Application layer desired rate for UDP)", dataRate);
  cmd.AddValue("tcpVariant", "Transport protocol to use: TcpNewReno, "
  		  	   "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
    		   "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ", tcpVariant);
  cmd.Parse(argc, argv);

  // Activate the ns-3 real time simulator
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  // Set real time mode (need RT preempt patch?) -> real time mode with synchronization mode set to Hard Limit does not work properly?
  Config::SetDefault ("ns3::RealtimeSimulatorImpl::SynchronizationMode", StringValue("BestEffort")); //BestEffort
  //Config::SetDefault ("ns3::RealtimeSimulatorImpl::HardLimit", StringValue("+100000000.0ns"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true)); // needed for tapBridge
  //Set RLC_AM mode TS 36.000 "ONLY RLC AM BEARER CAN BE CONFIGURED FOR THE SPLIT BEARER"
    //Disabled RLC_AM to Run on TUD Test Bed
  //Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (ns3::LteEnbRrc::RLC_AM_ALWAYS));
  //Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (128 * 1024));
  // LTE error model configuration
  // Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  // Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
  // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1024 * 1024));
  // Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue(Seconds(0.1)));

  // ============================
  // General NI API Configuration

  // write different log files for each stations
  std::string simStationType = "default";
  if (!daliDualConnectivityEnabled)
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
        simStationType   = "BS";
    } else {
        NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed");
    }
  else
  {
	if (niApiDevMode == "NIAPI_MBS" && niApiLteEnabled){
		// if ns-3 instance is configured as base station it acts as MeNB
		niApiLteDevMode  = "NIAPI_ENB";
		simStationType   = "MBS";
	} else if (niApiDevMode == "NIAPI_MTS" && niApiLteEnabled) {
		// if ns-3 instance is configured as terminal station it acts as MUE
		niApiLteDevMode  = "NIAPI_UE";
		simStationType   = "MTS";
	} else if (niApiDevMode == "NIAPI_SBS" && niApiLteEnabled) {
		// if ns-3 instance is configured as base station it acts as SeNB
		niApiLteDevMode  = "NIAPI_ENB";
		simStationType   = "SBS";
	} else if (niApiDevMode == "NIAPI_STS" && niApiLteEnabled) {
		// if ns-3 instance is configured as terminal station it acts as SUE
		niApiLteDevMode  = "NIAPI_UE";
		simStationType   = "STS";
	} else if (niApiDevMode == "NIAPI_BSTS" && !niApiLteEnabled) {
		// ns-3 instance is configures as BS and TS (Master and Secondary)- for debugging if only spectrum phy shall be bypassed
		niApiLteDevMode  = "NIAPI_ALL";
		simStationType   = "BSTS";
	} else if (niApiDevMode == "NIAPI_MBSTS" && !niApiLteEnabled) {
		// ns-3 instance is configures as MBS and STS- for debugging if only spectrum phy shall be bypassed
		niApiLteDevMode  = "NIAPI_ALL";
		simStationType   = "MBSTS";
	} else if (niApiDevMode == "NIAPI_SBSTS" && !niApiLteEnabled) {
		// ns-3 instance is configures as SBS and STS- for debugging if only spectrum phy shall be bypassed
		niApiLteDevMode  = "NIAPI_ALL";
		simStationType   = "SBSTS";
	} else {
		if (niApiLteEnabled)
		  NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed for DALI Dual Connectivity and NI API for LTE enabled");
		else
		  NS_FATAL_ERROR ("niApiDevMode " << niApiDevMode << " not allowed for DALI Dual Connectivity and NI API for LTE disabled");
	}

  }

  // Evaluations DALI / NI Configuration
  if (!(daliTransportProtocol == "NI" || daliTransportProtocol == "UDP" || daliTransportProtocol == "TCP")) NS_FATAL_ERROR ("ORCA - DALI Dual Connectivity Evaluations can consider only NI, TCP or UDP applications");
  if (!(daliLinkDirection == "DOWNLINK" || daliLinkDirection == "UPLINK")) NS_FATAL_ERROR ("ORCA - DALI Dual Connectivity Evaluations can consider only DOWNLINK or UPLINK Link Direction");

  if (daliTransportProtocol == "TCP")
  {
	tcpVariant = std::string ("ns3::") + tcpVariant;
	// Select TCP variant
	if (tcpVariant.compare ("ns3::TcpWestwoodPlus") == 0)
	{
	  // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
	  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
	  // the default protocol type in ns3::TcpWestwood is WESTWOOD
	  Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
	}
	else
	{
	  TypeId tcpTid;
	  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
	  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
	}
	/* Configure TCP Options */
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));
  }

  // print ouf config parameters
  // TODO-NI: replace cout by NI_LOG_CONSOLE_INFO
  std::cout << std::endl;
  std::cout << "-------- NS-3 Configuration -------------" << std::endl;


  std::cout << "Running LTE node as:        ";
  if (niApiLteDevMode == "NIAPI_ALL") std::cout << "ENB and UE" << std::endl;
  else if (niApiLteDevMode == "NIAPI_ENB")  std::cout << "ENB" << std::endl;
  else if (niApiLteDevMode == "NIAPI_UE")   std::cout << "UE" << std::endl;

  std::cout << "LTE API:                    ";
  if (niApiLteEnabled == true) std::cout << "enabled" << std::endl;
  else                         std::cout << "disabled" << std::endl;

  std::cout << "LTE UDP Loopback:           ";
  if (niApiLteLoopbackEnabled == true) std::cout << "enabled" << std::endl;
  else                                 std::cout << "disabled" << std::endl;

  std::cout << "TapBridge:                  ";
  if (niApiEnableTapBridge == true) std::cout << "enabled" << std::endl;
  else                              std::cout << "disabled" << std::endl;

  std::cout << "Logging:                    ";
  if (niApiEnableLogging == true) std::cout << "enabled" << std::endl;
  else                            std::cout << "disabled" << std::endl;
  //std::cout << "disabled" << std::endl;

  std::cout << "DALI Dual Connectivity:     ";
  if (daliDualConnectivityEnabled) std::cout << "enabled" << std::endl;
  else                             std::cout << "disabled" << std::endl;

  if (!niApiEnableTapBridge){
    std::cout << "DALI Throughput Evaluation: ";
    if (daliTransportProtocol == "NI")
  	  if (daliLinkDirection == "DOWNLINK") 	std::cout << "NI APP / UDP DOWNLINK" << std::endl;
  	  else 									std::cout << "NI APP / UDP UPLINK" << std::endl;
    if (daliTransportProtocol == "UDP"){
	  if (daliLinkDirection == "DOWNLINK") 	std::cout << "UDP DOWNLINK" << std::endl;
	  else 									std::cout << "UDP UPLINK" << std::endl;
	  std::cout << "DALI desired DataRate:      ";
	  std::cout << dataRate << std::endl;
    }
    if (daliTransportProtocol == "TCP")
      if (daliLinkDirection == "DOWNLINK")  std::cout << "TCP DOWNLINK, Variant: " << tcpVariant << std::endl;
  	  else 								    std::cout << "TCP UPLINK, Variant: " << tcpVariant << std::endl;
  }

  std::cout << "NI Module Version:          " << NI_MODULE_VERSION << std::endl;
  //std::cout << "Required AFW Version:   " << NI_AFW_VERSION << std::endl;

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

  // p2p helper
  PointToPointHelper p2pHelp;
                     p2pHelp.SetDeviceAttribute ("DataRate", StringValue ("100Gbps"));
                     p2pHelp.SetChannelAttribute ("Delay", StringValue ("2ms"));
                     p2pHelp.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  MobilityHelper MobilityHelp;
                 MobilityHelp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

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
    MobilityHelp.SetPositionAllocator(positionAlloc);

  }

  // ip helper
  InternetStackHelper     ipStackHelp;
  Ipv4AddressHelper       ipAddressHelp;
  Ipv4Mask                IpMask = "255.255.255.0";
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // Create a single RemoteHost
   NodeContainer LanNodes;
                 LanNodes.Create (1);
   Ptr<Node> remoteHost = LanNodes.Get (0);
   // lte user terminal nodes
   NodeContainer ueNodes;
                 ueNodes.Create(nLteUeNodes);
   // lte enb nodes - Extended for DALI, two eNbs and two UEs
   NodeContainer enbNodes;
                 enbNodes.Create(nLteEnbNodes);

   // install ip stacks on Lan nodes
   ipStackHelp.Install (LanNodes);

   // configure ip address spaces and gateways for the different subnetworks
   Ipv4Address p2pLteIpSubnet  = "10.1.2.0";
   Ipv4Address UeIpSubnet      = "7.0.0.0";
   Ipv4Address epcIpSubnet     = "11.0.0.0";
   Ipv4Address mmeIpSubnet     = "12.0.0.0";
   Ipv4Address x2IpSubnet      = "13.0.0.0";
   Ipv4Address dcxIpSubnet     = "14.0.0.0";


   /*// initialize routing database and set up the routing tables in the nodes*/
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   // ==========================
   // setup the lte network - note that lte is configured after "PopulateRoutingables" as static routing is used for epc

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
   if (niApiLteEnabled)
     {
       // NI LTE Application Framework supports 20MHz bandwidth -> 100 resource blocks
       // Set downlink transmission bandwidth in number of resource blocks -> set to 20MHz default here
       Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (100));
       // Set uplink transmission bandwidth in number of resource blocks -> set to 20MHz default here
       Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (100));
     }
   else
     {
       // Set downlink transmission bandwidth in number of resource blocks -> set to 10MHz default here
       Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (50));
       // Set uplink transmission bandwidth in number of resource blocks -> set to 10MHz default here
       Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (50));
     }
   // Disable ideal and use real RRC protocol with RRC PDUs (cf https://www.nsnam.org/docs/models/html/lte-design.html#real-rrc-protocol-model)
   Config::SetDefault ("ns3::DaliLteHelper::UseIdealRrc", BooleanValue (false));
   // Disable CQI measurement reports based on PDSCH (based on ns-3 interference model)
   Config::SetDefault ("ns3::DaliLteHelper::UsePdschForCqiGeneration", BooleanValue (false));
   // Use simple round robin scheduler here (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
   Config::SetDefault ("ns3::DaliLteHelper::Scheduler", StringValue ("ns3::RrFfMacScheduler"));
   // Switch to enable/disable DC functionality
   Config::SetDefault ("ns3::DaliEnbPdcp::PDCPDecDc", UintegerValue(dcactivate));
   // Switch to allow LWA/LWIP when DC is enabled
   Config::SetDefault ("ns3::DaliEnbPdcp::DcLwaLwipSwitch", UintegerValue(0));
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
   g_RemoteControlEngine.GetPdb()->setParameterDcDecVariable(dcactivate);
   g_RemoteControlEngine.GetPdb()->setParameterManualLteUeChannelSinrEnable(false);
   g_RemoteControlEngine.GetPdb()->setParameterLteUeChannelSinr(niChSinrValueDb);

   // epc helper is used to create core network incl installation of eNB App, DALI DC new EPC helpers implemented
   Ptr<EpcHelper>  epcHelper;
   if (!daliDualConnectivityEnabled)
   {
     //epcHelper = CreateObject<PointToPointEpcHelper> ();
	 epcHelper = CreateObject<DaliEmuSeparatedInstancesEpcHelper> ();
	 epcHelper->SetAttribute ("niApiDevMode", StringValue (niApiDevMode));
   }
   else if (niApiDevMode == "NIAPI_BSTS")
     epcHelper = CreateObject<DaliEmuOneInstanceEpcHelper> ();
   else
   {
	 epcHelper = CreateObject<DaliEmuSeparatedInstancesEpcHelper> ();
	 epcHelper->SetAttribute ("niApiDevMode", StringValue (niApiDevMode));
   }
   // lte helper is used to install L2/L3 on eNB/UE (PHY,MAC,RRC->PDCP/RLC, eNB NetDevice), DALI DC uses especific LTE Helper
   Ptr<DaliLteHelper>                lteHelper = CreateObject<DaliLteHelper> ();
   lteHelper->SetEpcHelper (epcHelper);

   // Use simple round robin scheduler here
   // (cf https://www.nsnam.org/docs/models/html/lte-design.html#round-robin-rr-scheduler)
   lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");


       // Configure LteHelper DALI attributes
	 lteHelper->SetAttribute ("isDaliDualConnectivity", BooleanValue (daliDualConnectivityEnabled));
	 lteHelper->SetAttribute ("usePdcpInSequenceDelivery", BooleanValue (usePdcpInSequenceDelivery));
	 lteHelper->SetAttribute ("niApiDevMode", StringValue (niApiDevMode));
	 lteHelper->SetAttribute ("daliLinkDirection", StringValue (daliLinkDirection));
	   // Configure EpcHelper DALI attributes
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


   Ptr<Node> PacketGwNode = epcHelper->GetPgwNode ();
   // create lte p2p link to Remote Host
   NetDeviceContainer p2pLteDevices = p2pHelp.Install (PacketGwNode, remoteHost);
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
   ipStackHelp.Install (ueNodes);
   Ipv4InterfaceContainer ueIpInterfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevices));

   // Set the default gateway for the lte ue nodes - will be used for all outgoing packets for this node
   for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
     {
       Ptr<Node> TmpNode = ueNodes.Get (u);
       Ptr<Ipv4StaticRouting> StaticRouting = ipv4RoutingHelper.GetStaticRouting (TmpNode->GetObject<Ipv4> ());
       StaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
     }

   if (daliDualConnectivityEnabled)
    {
     // Connects eNBs using X2 links
     lteHelper->AddX2Interface (enbNodes);
     // Connects UEs using DCX links
     lteHelper->AddDcxInterface (ueNodes);
	}

   // Attach BS ans UE nodes, For DALI: mUE and MeNB or sUE and SeNB
   uint16_t j = 0;
   uint16_t n = ueNodes.GetN ();
   if ((niApiDevMode == "NIAPI_MBS") || (niApiDevMode == "NIAPI_MTS"))
   {
	 j = 0;
	 n = 1;
   }
   if ((niApiDevMode == "NIAPI_SBS") || (niApiDevMode == "NIAPI_STS"))
   {
	 j = 1;
	 n = 2;
   }
   for (uint16_t i = j; i < n; i++)
	 lteHelper->Attach (ueDevices.Get(i), enbDevices.Get(i));
   // side effect: the default EPS bearer will be activated

   Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
   remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (UeIpSubnet), Ipv4Mask ("255.0.0.0"), 1);

   if (printRoutingTables)
   {
	   // print global routing tables
	 Ipv4GlobalRoutingHelper globalRouting;
	 Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routing_global", std::ios::out);
	 globalRouting.PrintRoutingTableAllAt (Seconds(0.1), routingStream );

	 // print static routing tables
	 Ptr<OutputStreamWrapper> testprint = Create<OutputStreamWrapper>("routing_static", std::ios::out);
	 Ptr<Node> PrintRouteNode;
	 Ptr<Ipv4StaticRouting> PrintRouteNodeStaticRouting;
	 for (int i=0; i < LanNodes.GetN(); i++)
	 {
	   PrintRouteNode = LanNodes.Get (i);
	   PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
	   PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);
	   NiUtils::PrintNodeInfo("LanNodes(" + std::to_string(i) + ")", PrintRouteNode);
	 }
	 PrintRouteNode = PacketGwNode;
	 PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
	 PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);
	 NiUtils::PrintNodeInfo("PacketGwNode", PrintRouteNode);
	 PrintRouteNode = enbNodes.Get (0);
	 PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
	 PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);
	 NiUtils::PrintNodeInfo("enbNode(0)", PrintRouteNode);
	 PrintRouteNode = ueNodes.Get (0);
	 PrintRouteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (PrintRouteNode->GetObject<Ipv4> ());
	 PrintRouteNodeStaticRouting->PrintRoutingTable(testprint);
	 NiUtils::PrintNodeInfo("ueNode(0)", PrintRouteNode);
   }

   // ==========================
   // application config

   NodeContainer ClientNode, ServerNode;
   Ipv4Address   ClientIpAddr, ServerIPAddr;
   if (daliLinkDirection == "DOWNLINK")
   {
	   // use Remote Host node as client and UE as server
	 ClientNode     = LanNodes.Get (0);
	 ClientIpAddr   = p2pLteIpInterfaces.GetAddress (1);
	 ServerNode     = ueNodes.Get (0);
	 ServerIPAddr   = ueIpInterfaces.GetAddress (0);
   }
   if (daliLinkDirection == "UPLINK")
   {
	   // use Remote Host node as server and UE as client
	 ClientNode     = ueNodes.Get (0);
	 ClientIpAddr   = ueIpInterfaces.GetAddress (0);
	 ServerNode     = LanNodes.Get (0);
	 ServerIPAddr   = p2pLteIpInterfaces.GetAddress (1);
   }

   // print addresses
   std::cout << std::endl;
   std::cout << "-------- NS-3 Topology Information ------" << std::endl;
   //std::cout << "Number of ETH devices      = " << LanIpInterfaces.GetN() << std::endl;
   std::cout << "Number of LTE UE devices   = " << ueIpInterfaces.GetN() << std::endl;
   std::cout << "Number of LTE eNB devices  = " << enbNodes.GetN () << std::endl;
   //std::cout << "Router GW IP Addr          = " << LanGwIpAddr << std::endl;
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

   if (!niApiEnableTapBridge && !(niApiDevMode == "NIAPI_SBS" || niApiDevMode == "NIAPI_STS" || niApiDevMode == "NIAPI_SBSTS"))
     {
	   if (daliTransportProtocol == "NI")
	   {
		   // undirectional traffic - https://www.nsnam.org/doxygen/classns3_1_1_udp_client.html
	     NiUdpServerHelper     ServerHelp (DestPort);
	     NiUdpClientHelper     ClientHelp(ClientDestAddr, DestPort);
	     //ClientHelp.SetAttribute ("RemoteAddress", AddressValue (ClientDestAddr));
	     //ClientHelp.SetAttribute ("RemotePort", UintegerValue (DestPort));
	     ClientHelp.SetAttribute ("MaxPackets", UintegerValue (packetNum));
	     ClientHelp.SetAttribute ("Interval", TimeValue (packetIntervalSec));
	     ClientHelp.SetAttribute ("PacketSize", UintegerValue (packetSize));

	     if ((niApiDevMode == "NIAPI_BS") ||(niApiDevMode == "NIAPI_BSTS")||
		 	     (niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS") ){
	    	 if (daliLinkDirection == "DOWNLINK") {
	    		 ApplicationContainer clientApps = ClientHelp.Install (ClientNode);
	    		 clientApps.Start (Seconds (transmTime));
	    		 clientApps.Stop (Seconds (simTime));
	    	 }
	     }
	     if ((niApiDevMode == "NIAPI_TS")||(niApiDevMode == "NIAPI_BSTS")||
		 	     (niApiDevMode == "NIAPI_MTS")||(niApiDevMode == "NIAPI_MBSTS") ){
	    	 if (daliLinkDirection == "UPLINK") {
	    		 ApplicationContainer clientApps = ClientHelp.Install (ClientNode);
	    		 clientApps.Start (Seconds (transmTime));
	    		 clientApps.Stop (Seconds (simTime));
	    	 }
	     }

	     ApplicationContainer serverApps = ServerHelp.Install (ServerNode);
	     serverApps.Start (Seconds (2.5));
	     serverApps.Stop (Seconds (simTime));

	     appServer = ServerHelp.GetServer ();
	   }
	   else
	   {
	     ApplicationContainer clientApps;
	     ApplicationContainer serverApps;
	     if (daliTransportProtocol == "UDP")
	     {
	 	   PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), DestPort));
	 	   serverApps.Add (dlPacketSinkHelper.Install (ServerNode));

	 	   OnOffHelper dlClient("ns3::UdpSocketFactory", Address ());
	 	   dlClient.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000]"));
	 	   dlClient.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	 	   dlClient.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
	 	   dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
	 	   AddressValue remoteAddress (InetSocketAddress (ServerIPAddr, DestPort));
	 	   dlClient.SetAttribute ("Remote", remoteAddress);

	 	   if ((niApiDevMode == "NIAPI_BS") ||(niApiDevMode == "NIAPI_BSTS")||
	 	     (niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS") ){
	 		 if (daliLinkDirection == "DOWNLINK") {
	 		   clientApps.Add (dlClient.Install (ClientNode));
	 		   clientApps.Start (Seconds (transmTime));
	 		   clientApps.Stop (Seconds (simTime));
	 		 }
	 	   }
	 	  if ((niApiDevMode == "NIAPI_TS") ||(niApiDevMode == "NIAPI_BSTS")||
	 	  	(niApiDevMode == "NIAPI_MTS")||(niApiDevMode == "NIAPI_MBSTS") ){
	 		if (daliLinkDirection == "UPLINK") {
	 		  clientApps.Add (dlClient.Install (ClientNode));
	 		  clientApps.Start (Seconds (transmTime));
	 		  clientApps.Stop (Seconds (simTime));
	 		}
	 	  }
	     }
	     else if(daliTransportProtocol == "TCP")
	     {
	       PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), DestPort));
	       serverApps.Add (dlPacketSinkHelper.Install (ServerNode));

	       BulkSendHelper dlClient ("ns3::TcpSocketFactory", Address ());
	       dlClient.SetAttribute ("MaxBytes", UintegerValue(0)); //0 is maximum
	       AddressValue remoteAddress (InetSocketAddress (ServerIPAddr, DestPort));
		   dlClient.SetAttribute ("Remote", remoteAddress);

		   if ((niApiDevMode == "NIAPI_BS") ||(niApiDevMode == "NIAPI_BSTS")||
			  (niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS") ){
			 if (daliLinkDirection == "DOWNLINK") {
			   clientApps.Add (dlClient.Install (ClientNode));
			   clientApps.Start (Seconds (transmTime));
			   clientApps.Stop (Seconds (simTime));
			 }
		   }
		   if ((niApiDevMode == "NIAPI_TS") ||(niApiDevMode == "NIAPI_BSTS")||
		   	  (niApiDevMode == "NIAPI_MTS")||(niApiDevMode == "NIAPI_MBSTS") ){
			 if (daliLinkDirection == "UPLINK") {
			   clientApps.Add (dlClient.Install (ClientNode));
			   clientApps.Start (Seconds (transmTime));
			   clientApps.Stop (Seconds (simTime));
			 }
		   }
	     }
	   sink = StaticCast<PacketSink> (serverApps.Get (0));
	   serverApps.Start (Seconds (2.5));
	   serverApps.Stop (Seconds (simTime));
	   Simulator::Schedule (Seconds (transmTime + 0.1), &CalculateThroughput/*, serverApps, clientApps, simTime*/);
	   }
     }
   else if (niApiEnableTapBridge)
     {
	   bool noBs = false;
	   bool noTs = false;
       if ((niApiDevMode == "NIAPI_BS")||(niApiDevMode == "NIAPI_MBS")){
           std::cout << "Install TAP bridge for BS host IP node..." << std::endl;
           enbDevices.Get(0)->SetAddress(Mac48Address::Allocate());
           TapBridgeHelper tapBridgeENB;
           std::string modeENB = "ConfigureLocal";
           std::string tapNameENB = "NIAPI_TapENB";
           tapBridgeENB.SetAttribute ("Mode", StringValue (modeENB));
           tapBridgeENB.SetAttribute ("DeviceName", StringValue (tapNameENB));
           tapBridgeENB.SetAttribute ("Mtu", UintegerValue(1500) );
           tapBridgeENB.Install (LanNodes.Get(0), p2pLteDevices.Get(1));
       }
       else
    	   noBs = true;

       if((niApiDevMode == "NIAPI_TS")||(niApiDevMode == "NIAPI_MTS")){
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
       else
    	   noTs = true;

       if (noBs && noTs)
	     NS_FATAL_ERROR ("TAP DEVICES NOT IMPLEMENTED FOR SELECTED niApiDevMode");
     }

   if (pcapTracing == true)
     {
       p2pHelp.EnablePcapAll("lte_simple_p2p");
       lteHelper->EnableTraces ();
     }

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
       Simulator::Schedule (Seconds (dualConnectivityLaunchTime), &DualConnectivityLauncher, MeNB);
     }

   std::cout << std::endl;
   std::cout << "[>] Start simulation" << std::endl;
   Simulator::Run ();
   std::cout << "[#] End simulation" << std::endl << std::endl;

   // check received packets
   if (!niApiEnableTapBridge && (daliTransportProtocol == "NI") &&
       ((daliLinkDirection == "DOWNLINK" && (niApiDevMode == "NIAPI_TS" || niApiDevMode == "NIAPI_MTS")) ||
        (daliLinkDirection == "UPLINK" && (niApiDevMode == "NIAPI_BS" || niApiDevMode == "NIAPI_MBS")) ||
        (niApiDevMode == "NIAPI_BSTS")||(niApiDevMode == "NIAPI_MBSTS")))
     {
       NI_LOG_CONSOLE_INFO ("Received packets: " << appServer->GetReceived()
                            << " / Lost packets: " << packetNum-appServer->GetReceived()
                            << "\n");
     }

   if (((niApiDevMode == "NIAPI_BSTS") && daliDualConnectivityEnabled)||(niApiDevMode == "NIAPI_MBS")||(niApiDevMode == "NIAPI_MBSTS"))
     MeNB_dev->Unref();

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

