#!/bin/bash

# start ns3 wifi build with required options

# source configuration script stored in <ns3_root>/configs/
source $1
#cat $1

# Set parameters of NS3 application

# Set name of ns-3 build with NI API
prog="ni-lte-wifi-extended"

echo ""		
case "$2" in
  BS) echo "Prepare ns-3 as BS for LteWifi example."
  # start as base station (LTE eNB + WIFI AP)
  niApiDevMode="\"NIAPI_BS\""
  ;;	  
  TS) echo "Prepare ns-3 as TS for LteWifi example."
  # start as terminal station (LTE UE + WIFI STA)
  niApiDevMode="\"NIAPI_TS\""	   
  ;;
  BSTS) echo "Prepare ns-3 as TS for LteWifi example."
  # start as terminal station (LTE UE + WIFI STA)
  niApiDevMode="\"NIAPI_BSTS\""	   
  ;;	   
  *) echo "Wrong device mode chosen"	
  exit 1
  ;;   
esac
echo ""

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

# Set client server configuration
cientServerConfig=$cientServerConfig

# Enable/disable logging
niApiEnableLogging=$niApiEnableLogging

# Enable/disable loopback mode
niApiWifiLoopbackEnabled=$niApiWifiLoopbackEnabled

# Enable/disable TapBridge
niApiEnableTapBridge=$niApiEnableTapBridge

# Enable/disable printing out the contents of sent/received messages
niApiWifiEnablePrintMsgContent="\"false\""

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

# Activate, partial activate or do not activate LWA
lwaactivate=$lwaactivate

# Activate IPSEC tunnel
lwipactivate=$lwipactivate

# Enable/disable Remote Control
niRemoteControlEnable=$niRemoteControlEnable

# ==============================================================

# create argument string for NS3 application
args="--packetSize=$packetSize --numPackets=$numPackets --interval=$interval --simTime=$simTime --transmTime=$transmTime  --niApiDevMode=$niApiDevMode --niApiWifiEnabled=$niApiWifiEnabled --niApiEnableLogging=$niApiEnableLogging --niApiWifiLoopbackEnabled=$niApiWifiLoopbackEnabled --niApiEnableTapBridge=$niApiEnableTapBridge --niRemoteControlEnable=$niRemoteControlEnable --niApiWifiSta1RemoteIpAddrTx=$niApiWifiSta1RemoteIpAddrTx --niApiWifiSta1RemotePortTx=$niApiWifiSta1RemotePortTx --niApiWifiSta1LocalPortRx=$niApiWifiSta1LocalPortRx --niApiWifiSta2RemoteIpAddrTx=$niApiWifiSta2RemoteIpAddrTx --niApiWifiSta2RemotePortTx=$niApiWifiSta2RemotePortTx --niApiWifiSta2LocalPortRx=$niApiWifiSta2LocalPortRx --niApiWifiSta1MacAddr=$niApiWifiSta1MacAddr --niApiWifiSta2MacAddr=$niApiWifiSta2MacAddr --niApiWifiBssidMacAddr=$niApiWifiBssidMacAddr --niApiWifiMcs=$niApiWifiMcs --niApiLteEnabled=$niApiLteEnabled --niApiLteLoopbackEnabled=$niApiLteLoopbackEnabled --lwaactivate=$lwaactivate --lwipactivate=$lwipactivate --cientServerConfig=$cientServerConfig"

# Start NS3 application 
echo "Starting ns-3 application $prog with: $args"
./waf --run $prog --command-template="%s $args"
EXIT_CODE=$?
echo "Leaving ns-3 application with exit code: $EXIT_CODE"
exit $EXIT_CODE
