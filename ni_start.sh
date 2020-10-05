#!/bin/bash

RUN_NS3_LTE="./src/ni/scripts/run_ns3_lte.sh src/ni/scripts/configs/"
RUN_NS3_LTE_DC="./src/ni/scripts/run_ns3_lte_dc.sh src/ni/scripts/configs/"
RUN_NS3_LTE_DC_DALI="./src/ni/scripts/run_ns3_lte_dc_dali.sh src/ni/scripts/configs/"
RUN_NS3_WIFI="./src/ni/scripts/run_ns3_wifi.sh src/ni/scripts/configs/"
RUN_NS3_LTE_WIFI_EXT="./src/ni/scripts/run_ns3_lte_wifi_ext.sh src/ni/scripts/configs/"
RUN_NS3_5G_LTE_WIFI_INTERWORKING="./src/ni/scripts/run_ns3_5g_lte_wifi_interworking.sh src/ni/scripts/configs/"

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
    LteDc) 
      case "$2" in
        BSTS) echo "Start ns-3 as BSTS in Lte example with $3."
          $RUN_NS3_LTE_DC$3 $2
          EXIT_CODE=$?
          ;;
        MBSTS) echo "Start ns-3 as MBSTS in Lte example with $3."
          $RUN_NS3_LTE_DC$3 $2
          EXIT_CODE=$?
          ;;
        SBSTS) echo "Start ns-3 as SBSTS in Lte example with $3."
          $RUN_NS3_LTE_DC$3 $2
          EXIT_CODE=$?
          ;;
        *)
          echo "Error: No device chosen. Please choose between 'BS' and 'TS'!"
          SHOW_HELP=1
          ;;
      esac
      ;;
    LteDcDali) 
      case "$2" in
        BSTS) echo "Start ns-3 as BSTS in Lte example with $3."
          $RUN_NS3_LTE_DC_DALI$3 $2
          EXIT_CODE=$?
          ;;
        MBSTS) echo "Start ns-3 as MBSTS in Lte example with $3."
          $RUN_NS3_LTE_DC_DALI$3 $2
          EXIT_CODE=$?
          ;;
        SBSTS) echo "Start ns-3 as SBSTS in Lte example with $3."
          $RUN_NS3_LTE_DC_DALI$3 $2
          EXIT_CODE=$?
          ;;
        *)
          echo "Error: No device chosen. Please choose between 'BS' and 'TS'!"
          SHOW_HELP=1
          ;;
      esac
      ;;
    LteWifi5g) 
      case "$2" in
        BSTS) echo "Start ns-3 as BSTS in Lte example with $3."
          $RUN_NS3_5G_LTE_WIFI_INTERWORKING$3 $2
          EXIT_CODE=$?
          ;;
        MBSTS) echo "Start ns-3 as MBSTS in Lte example with $3."
          $RUN_NS3_5G_LTE_WIFI_INTERWORKING$3 $2
          EXIT_CODE=$?
          ;;
        SBSTS) echo "Start ns-3 as SBSTS in Lte example with $3."
          $RUN_NS3_5G_LTE_WIFI_INTERWORKING$3 $2
          EXIT_CODE=$?
          ;;
        *)
          echo "Error: No device chosen. Please choose between 'BS' and 'TS'!"
          SHOW_HELP=1
          ;;
      esac
      ;;
    *) 
      echo "Error: Wrong simulation mode chosen. Please choose between 'Adhoc', 'Infra', 'LteWifi5g', 'LteWiFi' or 'Lte'!"
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
  echo "                <mode>        : {Adhoc, Infra, LteWiFi, Lte, LteDc, LteDcDali}"
  echo "                <device>      : {Sta1, Sta2, Ap, Sta, BS, TS, BSTS}"
  echo "                <config file> : see src/ni/scripts/configs/"
  echo ""
  echo ""
  echo "Examples for PHY simulation using single device:"
  echo ""
  echo "LteWiFi    : ./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext_no_loopback"
  echo "Lte        : ./ni_start.sh Lte BSTS config_lte_no_loopback"
  echo "LteDc      : ./ni_start.sh LteDc BSTS config_lte_dc_no_loopback"
  echo "LteDcDali  : ./ni_start.sh LteDcDali BSTS config_lte_dc_dali_dl_no_loopback"
  echo "             ./ni_start.sh LteDcDali BSTS config_lte_dc_dali_ul_no_loopback"
  echo "             ./ni_start.sh LteDcDali BSTS config_lte_dc_dali_tcp_no_loopback"
  echo "LteWifi5g  : ./ni_start.sh LteWifi5g BSTS config_5g_lte_wifi_interworking_no_loopback"
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

