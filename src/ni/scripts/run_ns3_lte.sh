#!/bin/bash

# start ns3 wifi build with required options

# source configuration script stored in <ns3_root>/configs/
source $1
#cat $1

# Set parameters of NS3 application

# Set name of ns-3 build with NI API
prog="ni-lte-simple"

echo ""		
case "$2" in
  BS) echo "Prepare ns-3 as BS for Lte example."
  # start as base station (LTE eNB)
  niApiDevMode="\"NIAPI_BS\""
  ;;	  
  TS) echo "Prepare ns-3 as TS for Lteexample."
  # start as terminal station (LTE UE)
  niApiDevMode="\"NIAPI_TS\""	   
  ;;
  BSTS) echo "Prepare ns-3 as TS and BS for Lte example."
  # start as terminal station (LTE UE and eNB)
  niApiDevMode="\"NIAPI_BSTS\""	   
  ;;	   
  *) echo "Wrong device mode chosen"	
  exit 1
  ;;   
esac
echo ""

# ==============================================================

# create argument string for NS3 application
args="--packetSize=$packetSize --numPackets=$numPackets --interval=$interPacketInterval --simTime=$simTime --transmTime=$transmTime  --niApiDevMode=$niApiDevMode --niApiEnableLogging=$niApiEnableLogging --niApiEnableTapBridge=$niApiEnableTapBridge --niApiLteEnabled=$niApiLteEnabled --niApiLteLoopbackEnabled=$niApiLteLoopbackEnabled"

# Start NS3 application 
echo "Starting ns-3 application $prog with: $args"
./waf --run $prog --command-template="%s $args"
EXIT_CODE=$?
echo "Leaving ns-3 application with exit code: $EXIT_CODE"
exit $EXIT_CODE
