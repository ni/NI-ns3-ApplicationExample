#!/bin/bash

RUN_NS3_LTE="./src/ni/scripts/run_ns3_lte.sh src/ni/scripts/configs/"
RUN_NS3_WIFI="./src/ni/scripts/run_ns3_wifi.sh src/ni/scripts/configs/"
RUN_NS3_LTE_WIFI_EXT="./src/ni/scripts/run_ns3_lte_wifi_ext.sh src/ni/scripts/configs/"

EXIT_CODE=1
SHOW_HELP=0

if [ "$#" -ne 3 ]; then
    echo "Illegal number of parameters"
    SHOW_HELP=1
else
  case "$1" in
    Adhoc) 
      case "$2" in
        Sta1) echo "Start ns-3 as STA1 in Adhoc mode with $3."
          $RUN_NS3_WIFI$3 Adhoc $2
          EXIT_CODE=$?
          ;;
        Sta2) echo "Start ns-3 as STA2 in Adhoc mode with $3."
          $RUN_NS3_WIFI$3 Adhoc $2
          EXIT_CODE=$?
          ;;
        *) 
          echo "Error: No device chosen. Please choose between 'Sta1' and 'Sta2'!"
          SHOW_HELP=1
          ;;
      esac
      ;;
    Infra) 
      case "$2" in
        Ap) echo "Start ns-3 as AP in Infrastructure mode with $3."
          $RUN_NS3_WIFI$3 Infra $2
          EXIT_CODE=$?
          ;;
        Sta) echo "Start ns-3 as STA in Infrastructure mode with $3."
          $RUN_NS3_WIFI$3 Infra $2
          EXIT_CODE=$?
          ;;
        *)
          echo "Error: No device chosen. Please choose between 'Ap' and 'Sta'!"
          SHOW_HELP=1
          ;;
      esac
      ;;
    LteWiFi) 
      case "$2" in
        BS) echo "Start ns-3 as BS in LteWifi example with $3."
          $RUN_NS3_LTE_WIFI_EXT$3 $2
          EXIT_CODE=$?
          ;;
        TS) echo "Start ns-3 as TS in LteWifi example with $3."
          $RUN_NS3_LTE_WIFI_EXT$3 $2
          EXIT_CODE=$?
          ;;
        BSTS) echo "Start ns-3 as BSTS in LteWifi example with $3."
          $RUN_NS3_LTE_WIFI_EXT$3 $2
          EXIT_CODE=$?
          ;;
        *)
          echo "Error: No device chosen. Please choose between 'BS' and 'TS'!"
          SHOW_HELP=1
          ;;
      esac
      ;;
    Lte) 
      case "$2" in
        BS) echo "Start ns-3 as BS in Lte example with $3."
          $RUN_NS3_LTE$3 $2
          EXIT_CODE=$?
          ;;
        TS) echo "Start ns-3 as TS in Lte example with $3."
          $RUN_NS3_LTE$3 $2
          EXIT_CODE=$?
          ;;
        BSTS) echo "Start ns-3 as BSTS in Lte example with $3."
          $RUN_NS3_LTE$3 $2
          EXIT_CODE=$?
          ;;
        *)
          echo "Error: No device chosen. Please choose between 'BS' and 'TS'!"
          SHOW_HELP=1
          ;;
      esac
      ;;        
    *) 
      echo "Error: Wrong simulation mode chosen. Please choose between 'Adhoc', 'Infra' or 'LteWiFi' or 'Lte'!"
      SHOW_HELP=1
      ;;
  esac
fi

if [ "$SHOW_HELP" == "1" ]
  then
  echo ""
  echo "Start application with following syntax:"
  echo ""
  echo "  ./ni_start.sh <mode> <device> <config file>"
  echo ""
  echo "                <mode>        : {Adhoc, Infra, LteWiFi, Lte}"
  echo "                <device>      : {Sta1, Sta2, Ap, Sta, BS, TS}"
  echo "                <config file> : see src/ni/scripts/configs/"
  echo ""
  echo ""
  echo "Examples for UDP local loopback using single device:"
  echo ""
  echo "Wifi Adhoc : ./ni_start.sh Adhoc Sta1 config_wifi_local_loopback"
  echo "             ./ni_start.sh Adhoc Sta2 config_wifi_local_loopback"
  echo "Wifi Infra : ./ni_start.sh Infra Ap config_wifi_local_loopback"
  echo "             ./ni_start.sh Infra Sta config_wifi_local_loopback"
  echo "LteWiFi    : ./ni_start.sh LteWiFi BS config_lwa_lwip_ext_local_loopback"
  echo "             ./ni_start.sh LteWiFi TS config_lwa_lwip_ext_local_loopback"
  echo "Lte        : ./ni_start.sh Lte BS config_lte_local_loopback"
  echo "             ./ni_start.sh Lte TS config_lte_local_loopback"
  echo ""
  echo ""
  echo "Examples for connection to Application Framework on SDR using two devices:"
  echo ""
  echo "Wifi Adhoc : ./ni_start.sh Adhoc Sta1 config_wifi_two_stations"
  echo "             ./ni_start.sh Adhoc Sta2 config_wifi_two_stations"
  echo "Wifi Infra : ./ni_start.sh Infra Ap config_wifi_two_stations"
  echo "             ./ni_start.sh Infra Sta config_wifi_two_stations"
  echo "LteWiFi    : ./ni_start.sh LteWiFi BS config_lwa_lwip_ext_two_stations"
  echo "             ./ni_start.sh LteWiFi TS config_lwa_lwip_ext_two_stations"
  echo "Lte        : ./ni_start.sh Lte BS config_lte_two_stations"
  echo "             ./ni_start.sh Lte TS config_lte_two_stations"
  echo ""
fi

exit $EXIT_CODE

