#!/bin/bash

# Setup
# -------------------------------     -----------------     -------------------------      --------------     -------------------
# | Robot Ctrl PC               |     | BS            |     | TS                     |     |            |     | ROBOT / RasPi   |
# | DATA_SRC_IP / DATA_SRC_PORT | --> | BS_TAP_BRIDGE | --> | TS_LTE/WIFI_TAP_BRIDGE | --> | TS_HOST_PC | --> | TS_E2E_IP       |
# ------------------------------- ETH ----------------- OTA -------------------------- ETH --------------     -------------------

# gateway in NS-3 topology
MOBILE_NETWORK_GATEWAY=10.1.1.1
# E2E ports
E2E_5G_PORT_BS=60102
# 5G sub nets
FG_SUBNET=7.0.0.0
BS_TAP_BRIDGE=10.1.1.2
# 5G UE terminal station specific settings
TS_5G_TAP_BRIDGE=7.0.0.2
TS_5G_TAP_BRIDGE_PORT=8990
# external data e.g from Robot Ctrl PC or external PC
DATA_SRC_IP=10.89.14.57
DATA_SRC_PORT_5G=60002
# ip and port where external data is received (5G BS eth0 IP)
DATA_RX_IP=10.89.14.13
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

echo "--> enable IP forwarding ..."
sysctl -w net.ipv4.ip_forward=1
sleep 5

# command line option
if [ "$START_FROM_CMD_LINE" = true ]; then
  echo "--> start NS-3 in BS mode form command line ..."
  cd ../../../
  ./ni_start.sh LteWiFi BS config_lwa_lwip_ext_two_stations &
  cd -
else
  echo "--> assuming NS-3 in BS mode was started from Application Framework (AFW) ..."
fi

echo "--> wait some seconds..."
sleep 5

echo "--> add network route in order to route all packets addressed to subnet $FG_SUBNET towards tap bridge NIAPI_TapENB"
# /24 means subnetmask of 255.255.255.0
route add -net $FG_SUBNET/24 gw $MOBILE_NETWORK_GATEWAY dev NIAPI_TapENB

echo "--> start packet forwarding (incoming packets from $DATA_SRC_IP received on $DATA_RX_IP/$DATA_SRC_PORT_5G will be forwarded to ns-3 TS LTE tap bridge $TS_5G_TAP_BRIDGE/$TS_5G_TAP_BRIDGE_PORT)"
python traffic_fwd.py $DATA_RX_IP $DATA_SRC_PORT_5G $TS_5G_TAP_BRIDGE $TS_5G_TAP_BRIDGE_PORT &

echo "--> start packet forwarding for E2E latency measurement (incoming packets from $DATA_SRC_IP received on $DATA_RX_IP/$E2E_5G_PORT_BS will be forwarded to ns-3 TS LTE tap bridge $TS_5G_TAP_BRIDGE/$E2E_5G_PORT_BS)"
python traffic_fwd.py $DATA_RX_IP $E2E_5G_PORT_BS $TS_5G_TAP_BRIDGE $E2E_5G_PORT_BS &

trap ctrl_c SIGINT SIGTERM SIGTSTP

echo "--> waiting for signals"
while [ "$return_script" = false ]
do
  #echo "running"
  sleep 1
done
echo "--> end of script"
