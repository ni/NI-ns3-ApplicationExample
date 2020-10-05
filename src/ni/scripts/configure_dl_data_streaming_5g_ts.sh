#!/bin/bash

# Setup
# -------------------------------     -----------------     -------------------------      --------------     -------------------
# | Robot Ctrl PC               |     | BS            |     | TS                     |     |            |     | ROBOT / RasPi   |
# | DATA_SRC_IP / DATA_SRC_PORT | --> | BS_TAP_BRIDGE | --> | TS_LTE/WIFI_TAP_BRIDGE | --> | TS_HOST_PC | --> | TS_E2E_IP       |
# ------------------------------- ETH ----------------- OTA -------------------------- ETH --------------     -------------------

# E2E latency measurement
E2E_5G_PORT_TS=60102
E2E_5G_PORT_SRC=61102
# terminal station specific settings
TS_5G_TAP_BRIDGE=7.0.0.2
TS_5G_TAP_BRIDGE_PORT=8990
# terminal host pc specific settings
TS_E2E_IP=10.89.14.49 # Robot IP
TS_HOST_PC_IP=10.89.14.41 # Routing App PC
TS_HOST_PC_PORT_5G=60002
# needs to be false if NS-3 is started from LabVIEW
START_FROM_CMD_LINE=false
return_script=false

function cleanup() {
  echo "--> kill all running traffic forwarding scripts..."
  ps aux | grep -i traffic_fwd | awk '{print $1}' | xargs sudo kill -9 &
  sleep 2
}

function ctrl_c() {
  echo "--> terminating..."
  cleanup
  return_script=true
  echo "--> exit script"
  exit 1
}

# clean up remainings from last call
cleanup

# command line option
if [ "$START_FROM_CMD_LINE" = true ]; then
  echo "--> start NS-3 in BS mode form command line ..."
  cd ../../../
  ./ni_start.sh LteWiFi TS config_lwa_lwip_ext_two_stations &
  cd -
else
  echo "--> assuming NS-3 in TS mode was started from Application Framework (AFW) ..."
fi

echo "--> wait some seconds..."
sleep 5

echo "--> Set IP addresses to Tap Bridges (in case NS-3 does it not correctly)"
ifconfig NIAPI_TapUE $TS_5G_TAP_BRIDGE netmask 255.255.255.0
sleep 5

echo "--> start packet forwarding (incoming packets from TS (5G UE) tap bridge to TS host PC)"
python traffic_fwd.py $TS_5G_TAP_BRIDGE $TS_5G_TAP_BRIDGE_PORT $TS_HOST_PC_IP $TS_HOST_PC_PORT_5G &

echo "--> start packet forwarding for E2E measurement (incoming packets from TS (5G UE) tap bridge to TS robot)"
python traffic_fwd.py $TS_5G_TAP_BRIDGE $E2E_5G_PORT_TS $TS_E2E_IP $E2E_5G_PORT_SRC &

trap ctrl_c SIGINT SIGTERM SIGTSTP

echo "--> waiting for signals"
while [ "$return_script" = false ]
do
  #echo "running"
  sleep 1
done
echo "--> end of script"


