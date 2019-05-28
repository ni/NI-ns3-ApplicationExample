#!/bin/bash

# Setup
# -------------------------------     -----------------     -------------------------      --------------
# | DATA SOURCE PC              |     | BS            |     | TS                     |     | TS_HOST_PC |
# | DATA_SRC_IP / DATA_SRC_PORT | --> | BS_TAP_BRIDGE | --> | TS_LTE/WIFI_TAP_BRIDGE | --> |            |
# ------------------------------- ETH ----------------- OTA -------------------------- ETH --------------

# gateway in NS-3 topology
MOBILE_NETWORK_GATEWAY=10.1.1.1
# LTE / WIFI sub nets
LTE_SUBNET=7.0.0.0
WIFI_SUBNET=10.1.2.0
BS_TAP_BRIDGE=10.1.1.2
# terminal station specific settings
TS_LTE_TAP_BRIDGE=7.0.0.2
TS_WIFI_TAP_BRIDGE=10.1.2.2
TS_LTE_TAP_BRIDGE_PORT=8990
TS_WIFI_TAP_BRIDGE_PORT=8991
# external data e.g from mmWave UD (back haul link) or external PC
DATA_SRC_IP=10.89.14.41
DATA_SRC_PORT_LTE=60000
DATA_SRC_PORT_WIFI=60001
# ip and port where external data is received (BS eth0 IP)
DATA_RX_IP=10.89.14.33
# needs to be false if NS-3 is started from LabVIEW
START_FROM_CMD_LINE=false

# clean up remainings from last call
echo "--> kill all running traffic forwarding scripts..."
ps aux | grep -i traffic_fwd | awk '{print $1}' | xargs sudo kill -9 &
sleep 2

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

echo "--> wait 15 seconds for NS-3 / AFW start up..."
sleep 15

echo "--> add network route in order to route all packets addressed to subnet $WIFI_SUBNET towards tap bridge NIAPI_TapENB"
# /24 means subnetmask of 255.255.255.0
route add -net $WIFI_SUBNET/24 gw $MOBILE_NETWORK_GATEWAY dev NIAPI_TapENB

echo "--> add network route in order to route all packets addressed to subnet $LTE_SUBNET towards tap bridge NIAPI_TapENB"
# /24 means subnetmask of 255.255.255.0
route add -net $LTE_SUBNET/24 gw $MOBILE_NETWORK_GATEWAY dev NIAPI_TapENB

echo "--> start packet forwarding (incoming packets from $DATA_SRC_IP received on $DATA_RX_IP/$DATA_SRC_PORT_LTE will be forwarded to ns-3 TS LTE tap bridge $TS_LTE_TAP_BRIDGE/$TS_LTE_TAP_BRIDGE_PORT)"
python traffic_fwd.py $DATA_RX_IP $DATA_SRC_PORT_LTE $TS_LTE_TAP_BRIDGE $TS_LTE_TAP_BRIDGE_PORT &

echo "--> start packet forwarding (incoming packets from $DATA_SRC_IP received on $DATA_RX_IP/$DATA_SRC_PORT_WIFI will be forwarded to ns-3 TS WIFI tap bridge $TS_WIFI_TAP_BRIDGE/$TS_WIFI_TAP_BRIDGE_PORT)"
python traffic_fwd.py $DATA_RX_IP $DATA_SRC_PORT_WIFI $TS_WIFI_TAP_BRIDGE $TS_WIFI_TAP_BRIDGE_PORT &
