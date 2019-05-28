#!/bin/bash

# Setup
# -------------------------------     -----------------     -------------------------      --------------
# | mmWave UD                   |     | BS            |     | TS                     |     | TS_HOST_PC |
# | DATA_SRC_IP / DATA_SRC_PORT | --> | BS_TAP_BRIDGE | --> | TS_LTE/WIFI_TAP_BRIDGE | --> |            |
# ------------------------------- ETH ----------------- OTA -------------------------- ETH --------------

# terminal station specific settings
TS_LTE_TAP_BRIDGE=7.0.0.2
TS_WIFI_TAP_BRIDGE=10.1.2.2
TS_LTE_TAP_BRIDGE_PORT=8990
TS_WIFI_TAP_BRIDGE_PORT=8991
TS_LWX_TAP_BRIDGE_PORT=9
# terminal host pc specific settings
TS_HOST_PC_IP=10.89.14.21
TS_HOST_PC_PORT_LTE=8990
TS_HOST_PC_PORT_WIFI=8991

# needs to be false if NS-3 is started from LabVIEW
START_FROM_CMD_LINE=false

# clean up remainings from last call
echo "--> kill all running traffic forwarding scripts..."
ps aux | grep -i traffic_fwd | awk '{print $1}' | xargs sudo kill -9 &
sleep 2

# command line option
if [ "$START_FROM_CMD_LINE" = true ]; then
  echo "--> start NS-3 in BS mode form command line ..."
  cd ../../../
  ./ni_start.sh LteWiFi BS config_lwa_lwip_ext_two_stations &
  cd -
else
  echo "--> assuming NS-3 in BS mode was started from Application Framework (AFW) ..."
fi

echo "--> wait 35 seconds..."
sleep 35

echo "--> Set IP addresses to Tap Bridges (in case NS-3 does it not correctly)"
ifconfig NIAPI_TapUE $TS_LTE_TAP_BRIDGE netmask 255.255.255.0
ifconfig NIAPI_TapSTA $TS_WIFI_TAP_BRIDGE netmask 255.255.255.0
sleep 5

echo "--> start packet forwarding (incoming packets from TS (LTE UE) tap bridge to TS host PC)"
python traffic_fwd.py $TS_LTE_TAP_BRIDGE $TS_LTE_TAP_BRIDGE_PORT $TS_HOST_PC_IP $TS_HOST_PC_PORT_LTE &

echo "--> start packet forwarding (incoming packets from TS (WIFI STA over LWA/LWIP) tap bridge to TS host PC)"
python traffic_fwd.py $TS_WIFI_TAP_BRIDGE $TS_LWX_TAP_BRIDGE_PORT $TS_HOST_PC_IP $TS_HOST_PC_PORT_WIFI &

echo "--> start packet forwarding (incoming packets from TS (WIFI STA) tap bridge to TS host PC)"
python traffic_fwd.py $TS_WIFI_TAP_BRIDGE $TS_WIFI_TAP_BRIDGE_PORT $TS_HOST_PC_IP $TS_HOST_PC_PORT_WIFI &
