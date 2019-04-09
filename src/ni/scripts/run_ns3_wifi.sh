#!/bin/bash

# start ns3 wifi build with required options

# source configuration script stored in <ns3_root>/configs/
source $1
#cat $1
echo ""

# Set parameters of NS3 application

# Set name of ns-3 build with NI API
prog="ni-wifi-simple"

case "$2" in
	Adhoc) case "$3" in
	   Sta1) echo "Prepare ns-3 as STA1 in Adhoc mode."
	   # start in Adhoc mode
	   niApiWifiConfigMode="\"Adhoc\""
	   # start as Sta1
	   niApiWifiDevMode="\"NIAPI_STA1\""
	   # Enable/disable the NI API
	   niApiWifiEnabled="\"true\""
	   ;;
	   
	   Sta2) echo "Prepare ns-3 as STA2 in Adhoc mode."
	   # start in Adhoc mode
	   niApiWifiConfigMode="\"Adhoc\""
	   # start as Sta2
	   niApiWifiDevMode="\"NIAPI_STA2\""
	   # Enable/disable the NI API
	   niApiWifiEnabled="\"true\""
	   ;;
	   
	   *) echo "Prepare normal ns-3 application in Adhoc mode."
	   # start in Adhoc mode
	   niApiWifiConfigMode="\"Adhoc\""
	   # Enable/disable the NI API
	   niApiWifiEnabled="\"false\""
	esac
	;;
	
	Infra) case "$3" in
	   Ap) echo "Prepare ns-3 as AP in Infrastructure mode."
	   # start in Infrastructure mode
	   niApiWifiConfigMode="\"Infrastructure\""
	   # start as Ap
	   niApiWifiDevMode="\"NIAPI_AP\""
	   # Enable/disable the NI API
	   niApiWifiEnabled="\"true\""
	   ;;
	   
	   Sta) echo "Prepare ns-3 as STA in Infrastructure mode."
	   # start in Infrastructure mode
	   niApiWifiConfigMode="\"Infrastructure\""
	   # start as Sta
	   niApiWifiDevMode="\"NIAPI_STA\""
	   # Enable/disable the NI API
	   niApiWifiEnabled="\"true\""
	   ;;
	   
	   *) echo "Prepare normal ns-3 application in Infrastructure mode."
	   # start in Infrastructure mode
	   niApiWifiConfigMode="\"Infrastructure\""
	   # Enable/disable the NI API
	   niApiWifiEnabled="\"false\""
	esac
	;;

	*) echo "Error: No Wi-Fi mode chosen."
esac

# ====================== Core parameters =======================

# Set size of application packet sent
packetSize=$packetSize

# Set number of packets generated
numPackets=$numPackets

# Set time interval between packet transmissions in ms
interval=$interPacketInterval

# Set simulation time in s
simTime=$simTime

# Set time in when the packet transmission is scheduled
transmTime=$transmTime

# Enable/disable logging
niApiEnableLogging=$niApiEnableLogging

# Enable/disable loopback mode
niApiWifiLoopbackEnabled=$niApiWifiLoopbackEnabled

# Enable/disable TapBridge
niApiEnableTapBridge=$niApiEnableTapBridge

# Enable/disable printing out the contents of sent/received messages
niApiWifiEnablePrintMsgContent=$niApiWifiEnablePrintMsgContent

# ==============================================================

# =============== UDP IP address and port config ===============

# IP address of STA1
niApiWifiSta1RemoteIpAddrTx=$niApiWifiSta1RemoteIpAddrTx

# TX port of STA1 (used as remote port for STA2)
niApiWifiSta1RemotePortTx=$niApiWifiSta1RemotePortTx

# RX Port of STA1
niApiWifiSta1LocalPortRx=$niApiWifiSta1LocalPortRx

# IP address of STA2
niApiWifiSta2RemoteIpAddrTx=$niApiWifiSta2RemoteIpAddrTx

# TX port of STA2 (used as remote port for STA1)
niApiWifiSta2RemotePortTx=$niApiWifiSta2RemotePortTx

# RX Port of STA2
niApiWifiSta2LocalPortRx=$niApiWifiSta2LocalPortRx

# ==============================================================

# ==================== Additional parameters ===================

# MAC Address of Sta1
niApiWifiSta1MacAddr=$niApiWifiSta1MacAddr

# MAC Address of Sta2
niApiWifiSta2MacAddr=$niApiWifiSta2MacAddr

# MAC Address of BSSID
niApiWifiBssidMacAddr=$niApiWifiBssidMacAddr

# Set MCS that will be used by the 802.11 AFW's PHY
niApiWifiMcs=$niApiWifiMcs

# ==============================================================

# create argument string for NS3 application
args="--packetSize=$packetSize --numPackets=$numPackets --interval=$interval --simTime=$simTime --transmTime=$transmTime --niApiWifiConfigMode=$niApiWifiConfigMode --niApiWifiDevMode=$niApiWifiDevMode --niApiWifiEnabled=$niApiWifiEnabled --niApiEnableLogging=$niApiEnableLogging --niApiWifiLoopbackEnabled=$niApiWifiLoopbackEnabled --niApiEnableTapBridge=$niApiEnableTapBridge --niApiWifiEnablePrintMsgContent=$niApiWifiEnablePrintMsgContent --niApiWifiSta1RemoteIpAddrTx=$niApiWifiSta1RemoteIpAddrTx --niApiWifiSta1RemotePortTx=$niApiWifiSta1RemotePortTx --niApiWifiSta1LocalPortRx=$niApiWifiSta1LocalPortRx --niApiWifiSta2RemoteIpAddrTx=$niApiWifiSta2RemoteIpAddrTx --niApiWifiSta2RemotePortTx=$niApiWifiSta2RemotePortTx --niApiWifiSta2LocalPortRx=$niApiWifiSta2LocalPortRx --niApiWifiSta1MacAddr=$niApiWifiSta1MacAddr --niApiWifiSta2MacAddr=$niApiWifiSta2MacAddr --niApiWifiBssidMacAddr=$niApiWifiBssidMacAddr --niApiWifiMcs=$niApiWifiMcs"

# Start NS3 application 
echo "Starting ns-3 application $prog with: $args"
./waf --run $prog --command-template="%s $args"
EXIT_CODE=$?
echo "Leaving ns-3 application with exit code: $EXIT_CODE"
exit $EXIT_CODE
