/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 National Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Vincent Kotzsch <vincent.kotzsch@ni.com>
 *         Clemens Felber <clemens.felber@ni.com>
 */

#ifndef NI_PIPE_TRANSPORT_H_
#define NI_PIPE_TRANSPORT_H_

#include <cstdio>

#include <ns3/object.h>
#include <ns3/system-thread.h>
#include "ni-common-constants.h"
#include "ns3/ni-l1-l2-api-lte.h"

namespace ns3 {
  typedef Callback< bool, uint8_t* > NiPipeTransportDataEndOkCallback;
  typedef Callback< bool, PhyCellMeasInd > NiPipeTransportCellMeasurementEndOkCallback;


  // note - used as member of ns-3 object class - mainly used for callback functionality
  class NiPipeTransport : public Object
  {
  public:
    NiPipeTransport ();
    NiPipeTransport (std::string context);
    virtual
    ~NiPipeTransport ();


    int32_t Init(int timingIndThreadPriority, int rxThreadpriority);
    uint64_t GetTimingIndTime(void);
    bool GetTimingIndReceived(void);
    uint8_t GetTimingIndTti(void);
    uint16_t GetTimingIndSfn(void);
    int32_t DeInit(void);

    int32_t CreateAndSendDlTxConfigReqMsg(
        uint32_t sfn,
        uint32_t tti,
        uint32_t prbAlloc,
        uint32_t rnti,
        uint32_t mcs,
        uint32_t tbsSize
    );

    int32_t CreateAndSendDlTxPayloadReqMsg(
        uint32_t sfn,
        uint32_t tti,
        uint8_t* macPduPacket,
        uint32_t tbsSize
    );

    int32_t CreateAndSendUlTxPayloadReqMsg(
        uint32_t sfn,
        uint32_t tti,
        uint8_t* macPduPacket,
        uint32_t tbsSize
    );


    void SetNiApiDataEndOkCallback (NiPipeTransportDataEndOkCallback c);
    void SetNiApiCellMeasurementEndOkCallback (NiPipeTransportCellMeasurementEndOkCallback c);
    void SetNiApiDevType(uint8_t niApiDevType);

  private:
    // private function prototypes
    void ReceiveTimingInd(void);
    void ReceiveCnfAndRxInd(void);


    static const size_t m_maxPacketSize = NI_COMMON_CONST_MAX_PAYLOAD_SIZE; // bytes

    const struct timespec m_ts = {0, 10000L};  // 10us wait time within threads to CPU consumption

    // pipe names are defined by the LTE Application Framework
    // phy timing ind
    char* m_pipe_name_1 = (char*)"/tmp/api_transport_0_pipe";
    int32_t m_fd1 = -1;
    fd_set m_readFds1;
    int32_t m_fdMax1 = 0;
    uint8_t*  m_pBufU8Pipe1;
    uint32_t m_bufOffsetU8Pipe1 = 0;
    // phy tx cfg/payl
    char* m_pipe_name_2 = (char*)"/tmp/api_transport_1_pipe";
    int32_t m_fd2 = -1;
    uint8_t*  m_pBufU8Pipe2;
    uint32_t m_bufOffsetU8Pipe2 = 0;
    // phy rx ind
    char* m_pipe_name_3 = (char*)"/tmp/api_transport_2_pipe";
    int32_t m_fd3 = -1;
    fd_set m_readFds3;
    int32_t m_fdMax3 = 0;
    uint8_t*  m_pBufU8Pipe3;
    uint32_t m_bufOffsetU8Pipe3 = 0;

    uint64_t m_sysTimeUs = 1000; // 1 TTI = 1 ms
    uint64_t m_lastSysTimeUs = 0;
    uint8_t m_tti = 0;
    uint16_t m_sfn = 0;

    Ptr<SystemThread> m_timingIndThread;
    int m_timingIndThreadPriority = 0;
    bool m_timingIndThreadStop = false;
    bool m_timingIndReceived = false;

    Ptr<SystemThread> m_rxThread;
    NiPipeTransportDataEndOkCallback m_niApiDataEndOkCallback;
    NiPipeTransportCellMeasurementEndOkCallback m_niApiCellMeasurementEndOkCallback;
    int m_rxThreadpriority = 0;
    bool m_rxThreadStop = false;

    //TODO-NI: use typedef
    uint8_t m_niApiDevType = 0; // NIAPI_ENB  = 0, NIAPI_UE = 1, NIAPI_ALL = 2, NIAPI_NONE = 3,

    // global reference and PDU index counters
    uint64_t m_msgRefId = 0;
    uint64_t m_macPduIndex = 0;

    // for statistics
    uint64_t m_numPhyTimingInd                 = 0;
    uint64_t m_numPhyDlTxConfigReq             = 0;
    uint64_t m_numPhyDlTxPayloadReq            = 0;
    uint64_t m_numPhyUlTxPayloadReq            = 0;
    uint64_t m_numPhyCnf[CNF_NUM_STATUS_CODES] = {0};
    uint64_t m_numPhyDlschRxInd                = 0;
    uint64_t m_numPhyCellMeasInd               = 0;
    uint64_t m_numPhyUlschRxInd                = 0;

    std::string m_context; // context of this object e.g. "LTE"
  };

} //namespace ns3

#endif /* NI_PIPE_TRANSPORT_H_ */
