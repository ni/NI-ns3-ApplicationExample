#!/bin/bash

EXIT_CODE=0

# time out for each test run
TIMEOUT="120s 120s"

# test lists for a two node loopback scenario 
declare -a NODE1_START_CMD_LIST=(
                     "./waf --run ni-lte-wifi-lan-interworking"
                     "./waf --run ni-lte-simple"
#                     "./waf --run ni-remote-control-simple"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext_no_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_lan__no_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_lte__no_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_wifi__no_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_lte__lte_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_wifi__wifi_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_lte__lte_wifi_loopback"
                     "./ni_start.sh LteWiFi BSTS config_lwa_lwip_ext__ref_wifi__lte_wifi_loopback"
                     "./ni_start.sh Lte BSTS config_lte_no_loopback"
                     "./ni_start.sh LteDc BSTS config_lte_dc_no_loopback"
                     "./ni_start.sh LteDcDali BSTS config_lte_dc_dali_dl_no_loopback"
                     "./ni_start.sh LteDcDali BSTS config_lte_dc_dali_ul_no_loopback"
                     "./ni_start.sh LteDcDali BSTS config_lte_dc_dali_tcp_no_loopback"
                     )

NUMTESTS=${#NODE1_START_CMD_LIST[@]}

# go to ns-3 root folder
cd ../../../

# loop over test lists
for (( i=1; i<${NUMTESTS}+1; i++ ));
do
  echo ""
  echo "###############################################################################"
  echo "Test $i / ${NUMTESTS} :"
  echo "  ${NODE1_START_CMD_LIST[$i-1]}"
  echo "  executing..."
  # execute start command in background considerung a execution time out
  timeout -k $TIMEOUT ${NODE1_START_CMD_LIST[$i-1]} &> test_output_${i-1}_a.log & P1=$!
  sleep 1

  # wait for returning process and collect exit status
  wait -n
  EXIT_SCRIPT_1=$?
  if [ "$EXIT_SCRIPT_1" -eq 124 ]; then
    echo "  ...stopped on TIMEOUT!"
  fi

  
  echo "  ...finished with exit code 1: $EXIT_SCRIPT_1"

  # check exit status of both nodes
  if [ "$EXIT_SCRIPT_1" -ne 0 ]; then
    EXIT_CODE=1
    echo "  FAIL"
    echo ""
    echo "  ---- Displaying test output of file test_output_${i-1}_a.log ---------------------"
    echo ""
    cat test_output_${i-1}_a.log
    echo ""
    echo "  -----------------------------------------------------------------------------"
  else
    echo "  PASS"
  fi
done

echo ""

if [ "$EXIT_CODE" -eq 0 ]; then
  echo "###############################################################################"
  echo "ALL TESTS PASSED"
  echo "###############################################################################"
else
  echo "###############################################################################"
  echo "SOME TESTS FAILED"
  echo "###############################################################################"
fi

cd -
exit $EXIT_CODE

