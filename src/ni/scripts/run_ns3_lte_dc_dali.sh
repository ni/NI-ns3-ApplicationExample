#!/bin/bash

# start ns3 wifi build with required options

# source configuration script stored in <ns3_root>/configs/
source $1
#cat $1

# Set parameters of NS3 application

# Set name of ns-3 build with NI API
prog="dali-lte-dc-experimentation"

# get name of the default ethernet interface
DEFAULT_ETH_IF_NAME=$(ip -o -4 route show to default | awk '{print $5}')
# check if PROMISC mode is enabled
ip link show $DEFAULT_ETH_IF_NAME | grep -q "PROMISC"
DEFAULT_ETH_IF_PROMISC=$?
# if not enabled, switch on
if [ "$DEFAULT_ETH_IF_PROMISC" -ne "0" ]; then
  # set promisc mode, needed for emu-fd-net-devices
  sudo ip link set $DEFAULT_ETH_IF_NAME promisc on
fi

echo ""		
case "$2" in
  BSTS) echo "Prepare ns-3 as TS and BS for Lte example."
  # start as terminal station (LTE UE and eNB)
  niApiDevMode="\"NIAPI_BSTS\""	   
  ;;
  MBSTS) echo "Prepare ns-3 as TS and BS for Lte example."
  # start as terminal station (LTE UE and eNB)
  niApiDevMode="\"NIAPI_MBSTS\""	   
  ;;
  SBSTS) echo "Prepare ns-3 as TS and BS for Lte example."
  # start as terminal station (LTE UE and eNB)
  niApiDevMode="\"NIAPI_SBSTS\""	   
  ;;	 	   
  *) echo "Wrong device mode chosen"	
  exit 1
  ;;   
esac
echo ""

# ==============================================================

# create argument string for NS3 application
args="--packetSize=$packetSize --numPackets=$numPackets --interval=$interPacketInterval --simTime=$simTime --transmTime=$transmTime  --niApiDevMode=$niApiDevMode --niApiEnableLogging=$niApiEnableLogging --niApiEnableTapBridge=$niApiEnableTapBridge --niApiLteEnabled=$niApiLteEnabled --niApiLteLoopbackEnabled=$niApiLteLoopbackEnabled --fdDeviceName=$fdDeviceName --daliDualConnectivityEnabled=$daliDualConnectivityEnabled --dualConnectivityLaunchTime=$dualConnectivityLaunchTime --usePdcpInSequenceDelivery=$usePdcpInSequenceDelivery --daliTransportProtocol=$daliTransportProtocol --daliLinkDirection=$daliLinkDirection --dataRate=$dataRate --tcpVariant=$tcpVariant"

# Start NS3 application 
echo "Starting ns-3 application $prog with: $args"
./waf --run $prog --command-template="%s $args"
EXIT_CODE=$?

# disable promisc mode if it was initialy disabled
if [ "$DEFAULT_ETH_IF_PROMISC" -ne "0" ]; then
  # set promisc mode, needed for emu-fd-net-devices
  sudo ip link set $DEFAULT_ETH_IF_NAME promisc off
fi

echo "Leaving ns-3 application with exit code: $EXIT_CODE"
exit $EXIT_CODE
