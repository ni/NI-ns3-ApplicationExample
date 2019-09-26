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
 
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <bitset>
#include <thread>
#include <string>
#include <ns3/simulator.h>
#include "ns3/ni-l1-l2-api-lte-message.h"
#include "ns3/ni-l1-l2-api-lte-handler.h"
#include "ns3/ni-lte-sdr-timing-sync.h"
#include "ns3/ni-utils.h"
#include "ns3/ni-logging.h"
#include "ni-pipe.h"
#include "ns3/ni-remote-control-engine.h"
#include "ni-pipe-transport.h"


namespace ns3 {
  NiPipeTransport::NiPipeTransport ()
  {
    m_context = "none";
    m_ns3Running = false;
    m_niApiTimingIndEndOkCallback = MakeNullCallback< bool, uint16_t, uint8_t, bool >();
    m_niApiDataEndOkCallback = MakeNullCallback< bool, uint8_t* >();
    m_niApiCellMeasurementEndOkCallback = MakeNullCallback< bool, PhyCellMeasInd >();
    ;
  }

  NiPipeTransport::NiPipeTransport (std::string context)
  {
    m_context = context;
    m_ns3Running = false;
    m_niApiTimingIndEndOkCallback = MakeNullCallback< bool, uint16_t, uint8_t, bool >();
    m_niApiDataEndOkCallback = MakeNullCallback< bool, uint8_t* >();
    m_niApiCellMeasurementEndOkCallback = MakeNullCallback< bool, PhyCellMeasInd >();
  }

  NiPipeTransport::~NiPipeTransport ()
  {
    // TODO Auto-generated destructor stub
  }


// public functions
int32_t
NiPipeTransport::Init(int timingIndThreadPriority, int rxThreadpriority)
{
  //---------------------------------------------------------
  // initialize named pipes
  //---------------------------------------------------------

  NI_LOG_CONSOLE_DEBUG( "NI.TRANSPORT: Preparing named pipes" );

  // Assumption: The LV counterpart has created the FIFOs and will also clean them up at the end
  NiPipe::OpenPipeForTx(m_pipe_name_2, &m_fd2);
  NiPipe::OpenPipeForRx(m_pipe_name_1, &m_fd1, &m_readFds1, &m_fdMax1);
  NiPipe::OpenPipeForRx(m_pipe_name_3, &m_fd3, &m_readFds3, &m_fdMax3);

  //---------------------------------------------------------
  // prepare variables
  //---------------------------------------------------------

  NI_LOG_NONE( "Preparing variables" );

  // buffer for NIAPI messages
  m_pBufU8Pipe1 = (uint8_t*)malloc(m_maxPacketSize*sizeof(uint8_t));
  if (m_pBufU8Pipe1 == NULL)
    {
      NI_LOG_FATAL("NiPipeTransport::Init: malloc for pipe1 failed")
    }
  memset(m_pBufU8Pipe1, 0, m_maxPacketSize);

  m_pBufU8Pipe2 = (uint8_t*)malloc(m_maxPacketSize*sizeof(uint8_t));
  if (m_pBufU8Pipe2 == NULL)
    {
      NI_LOG_FATAL("NiPipeTransport::Init: malloc for pipe2 failed")
    }
  memset(m_pBufU8Pipe2, 0, m_maxPacketSize);

  m_pBufU8Pipe3 = (uint8_t*)malloc(m_maxPacketSize*sizeof(uint8_t));
  if (m_pBufU8Pipe3 == NULL)
    {
      NI_LOG_FATAL("NiPipeTransport::Init: malloc for pipe3 failed")
    }
  memset(m_pBufU8Pipe3, 0, m_maxPacketSize);

  m_sysTimeUs = 1000; // 1 TTI = 1 ms
  m_lastSysTimeUs = 0;

  // spawn thread for PHY timint indication
  m_timingIndReceived = false;
  m_timingIndThreadStop = false;
  m_timingIndThreadPriority = timingIndThreadPriority;
  m_timingIndThread = Create<SystemThread> (MakeCallback (&NiPipeTransport::ReceiveTimingInd, this));
  m_timingIndThread->Start();

  // spawn thread for Rx reception (RX indication, TX confirmation)
  m_rxThreadStop = false;
  m_rxThreadpriority = rxThreadpriority;
  m_rxThread = Create<SystemThread> (MakeCallback (&NiPipeTransport::ReceiveCnfAndRxInd, this));
  m_rxThread->Start();

  return 0;
}

uint64_t NiPipeTransport::GetTimingIndTime(void)
{
  return m_sysTimeUs;
}

bool NiPipeTransport::GetTimingIndReceived(void)
{
  return m_timingIndReceived;
}

uint8_t NiPipeTransport::GetTimingIndTti(void)
{
  return m_tti;
}

uint16_t NiPipeTransport::GetTimingIndSfn(void)
{
  return m_sfn;
}

int32_t
NiPipeTransport::DeInit(void)
{
  m_timingIndThreadStop = true;
  NI_LOG_DEBUG( "wait for timingIndThread");
  m_timingIndThread->Join();
  NI_LOG_DEBUG( "timingIndThread finished");

  m_rxThreadStop = true;
  NI_LOG_DEBUG( "wait for rxThread");
  m_rxThread->Join();
  NI_LOG_DEBUG( "rxThread finished");

  NI_LOG_CONSOLE_INFO("\n-------- NI L1-L2 API Statistics --------");
  NI_LOG_CONSOLE_INFO("PhyTimingInd       = " << m_numPhyTimingInd);
  NI_LOG_CONSOLE_INFO("PhyDlTxConfigReq   = " << m_numPhyDlTxConfigReq);
  NI_LOG_CONSOLE_INFO("PhyDlTxPayloadReq  = " << m_numPhyDlTxPayloadReq);
  NI_LOG_CONSOLE_INFO("PhyUlTxPayloadReq  = " << m_numPhyUlTxPayloadReq);
  for (uint32_t i; i < CNF_NUM_STATUS_CODES; i++)
    {
      if (m_numPhyCnf[i] > 0)
        {
          NI_LOG_CONSOLE_INFO("PhyCnf (Status " << i << ") = " << m_numPhyCnf[i]);
        }
    }
  NI_LOG_CONSOLE_INFO("PhyDlschRxInd      = " << m_numPhyDlschRxInd);
  NI_LOG_CONSOLE_INFO("PhyCellMeasInd     = " << m_numPhyCellMeasInd);
  NI_LOG_CONSOLE_INFO("PhyUlschRxInd      = " << m_numPhyUlschRxInd);
  NI_LOG_CONSOLE_INFO("-----------------------------------------\n");

  free( m_pBufU8Pipe1 );
  free( m_pBufU8Pipe2 );
  free( m_pBufU8Pipe3 );

  return 0;
}


// private functions
void
NiPipeTransport::ReceiveTimingInd(void)
{
  // set thread priority
  NiUtils::SetThreadPrioriy(m_timingIndThreadPriority);
  NiUtils::AddThreadInfo (pthread_self(), (m_context + " NiPipeTransport PhyTimingInd thread"));
  NI_LOG_DEBUG("NI.PIPE.TRANSPORT: Pipe PhyTimingInd thread with id:" <<  pthread_self() << " started");

  // thread loop
  while(!m_timingIndThreadStop)
    {
      NI_LOG_NONE ("timingIndThread Start");
    int32_t  nread  = 0;

    // NIAPI related parameters
    uint32_t msgType    = 0;
    uint32_t bodyLength = 0;

    // NIAPI messages
    PhyTimingInd phyTimingInd;

    uint32_t iteration = 0;
    // poll pipe for timing indication
    while ( true )
      {
        bool done = false;
        //----------------------------------------------------------------
        // read timing indication (polling mode)
        //----------------------------------------------------------------

        nread = NiPipe::PipeRead(&m_fd1, &m_readFds1, &m_fdMax1, m_pBufU8Pipe1, 16); // TTI indication = 16 bytes

        if (nread > 0)
          {
            // check im ns-3 is running
            m_ns3Running = (Simulator::Now().GetMicroSeconds() > 0) || !(Simulator::GetContext () == Simulator::NO_CONTEXT);

            NI_LOG_NONE ("received " << nread << "bytes");
            m_bufOffsetU8Pipe1 = 0;
            GetMsgType( &msgType, m_pBufU8Pipe1, &m_bufOffsetU8Pipe1 );

            switch (msgType)
            {
              // Where timing trigger is received from L1, send config and payload.
              case (PHY_TIMING_IND):
                  bool firstRun, callbackValid;
                  firstRun = false;
                  if (!m_timingIndReceived)
                    {
                      m_timingIndReceived = true;
                      firstRun = true;
                    }
                  m_numPhyTimingInd++;
                  m_lastSysTimeUs = m_sysTimeUs;
                  m_sysTimeUs = NiUtils::GetSysTime();
                  DeserializePhyTimingInd( &phyTimingInd, m_pBufU8Pipe1, &m_bufOffsetU8Pipe1 );
                  m_tti = (uint8_t) phyTimingInd.subMsgHdr.tti;
                  m_sfn = (uint16_t) phyTimingInd.subMsgHdr.sfn;
                  NI_LOG_DEBUG ("TimingInd received after: " + std::to_string(m_sysTimeUs-m_lastSysTimeUs) +
                                " with SFN: " + std::to_string(m_sfn) + " TTI: " +
                                std::to_string(m_tti));
                  callbackValid = (m_niApiTimingIndEndOkCallback != MakeNullCallback< bool, uint16_t, uint8_t, bool >());
                  if (callbackValid && m_ns3Running)
                    {
                      // provide timing ind information to ni-phy-interface
                      m_niApiTimingIndEndOkCallback(m_sfn, m_tti, firstRun);
                    }
                  done = true;
                  break;
              default:
                  NI_LOG_FATAL("Received UNKNOWN message. pipe=" << m_pipe_name_1 << " msgType=" << msgType);
                  done = true;
                break;
            }
          }
        else
          {
            iteration++;
            if (iteration > 1000) {
                NI_LOG_NONE ("1000 x nothing received");
                iteration = 0;
            }
          }
        if (done || m_timingIndThreadStop) break;
        nanosleep( &m_ts, NULL );   // wait a little bit and then start over again
      }
//    // remote control implementation example
//    g_RemoteControlEngine.GetPdb()->setParameterNumPhyTimingInd(m_numPhyTimingInd); // Makes PhyTimingInd read accesible thorgh Remote Control
//    if (m_numPhyTimingInd % 1000 == 0
//      && g_RemoteControlEngine.GetPdb()->getParameterLogPhyTimingInd() ) //Remote Controlled logging enable through second boolean statement
//      {
//        NI_LOG_CONSOLE_INFO(std::to_string(m_numPhyTimingInd) + " PhyTimingInd Received");
//      }

    }
}

void
NiPipeTransport::ReceiveCnfAndRxInd(void)
{
  // set thread priority
  NiUtils::SetThreadPrioriy(m_rxThreadpriority);
  NiUtils::AddThreadInfo (pthread_self(), (m_context + " NiPipeTransport RxIndCnf thread"));
  NI_LOG_DEBUG("NI.PIPE.TRANSPORT: Pipe RxIndCnf thread with id:" <<  pthread_self() << " started");

  // thread loop
  while(!m_rxThreadStop)
    {
    NI_LOG_NONE ("rxThread Start");
    int32_t  nread  = 0;

    // NIAPI related parameters
    uint32_t msgType    = 0;
    uint32_t bodyLength = 0;

    // NIAPI messages
    PhyCnf phyCnf;
    PhyDlschRxInd phyDlschRxInd;
    PhyCellMeasInd phyCellMeasInd;
    PhyUlschRxInd phyUlschRxInd;

    std::string str;

    uint64_t iteration = 0;
    // poll pipe for rx indication or tx confirmation
    while ( true )
      {
        bool done = false;
        //----------------------------------------------------------------
        // read DL RX payload and DL TX confirmation
        //----------------------------------------------------------------

        nread = NiPipe::PipeRead( &m_fd3, &m_readFds3, &m_fdMax3, m_pBufU8Pipe3, 8 ); // genMsgHdr = 8 bytes

        if (nread > 0)
          {
            NI_LOG_NONE ("received " << nread << "bytes");
            // Extract message type and body length (variable)
            m_bufOffsetU8Pipe3 = 0;
            GetMsgType( &msgType, m_pBufU8Pipe3, &m_bufOffsetU8Pipe3 );
            GetBodyLength( &bodyLength, m_pBufU8Pipe3, &m_bufOffsetU8Pipe3 );

            // Read rest of the message
            if ( bodyLength > NiPipe::PipeRead(&m_fd3, &m_readFds3, &m_fdMax3, m_pBufU8Pipe3+8, bodyLength ) )
              {
                NI_LOG_FATAL( "ERROR: Could not read requested amount of bytes. "
                              "pipe=" << m_pipe_name_3 << ", num_requested=" << bodyLength << ", num_received=" << nread );
              }

            const int numBytes = 32;
            char buffer [numBytes*2+1];

            switch (msgType)
            {
              case (PHY_ULSCH_RX_IND):
                m_numPhyUlschRxInd++;
                DeserializePhyUlschRxInd( &phyUlschRxInd, m_pBufU8Pipe3, &m_bufOffsetU8Pipe3 );

                for(int j = 0; j < numBytes; j++)
                  {
                    sprintf(&buffer[2*j], "%02X", phyUlschRxInd.ulschMacPduRxBody.macPdu[j]);
                  }
                NI_LOG_DEBUG ("UlschRxInd received with" <<
                              " SFN: " << phyUlschRxInd.subMsgHdr.sfn <<
                              " TTI: " << phyUlschRxInd.subMsgHdr.tti <<
                              " CRC: " << phyUlschRxInd.ulschMacPduRxBody.crcResult <<
                              " RNTI: " << phyUlschRxInd.ulschMacPduRxBody.rnti <<
                              " PDU Size: " << phyUlschRxInd.ulschMacPduRxBody.macPduSize <<
                              " PDU[]: " << buffer);

                if (m_niApiDevType == 0) // eNB
                  {
                    if (phyUlschRxInd.ulschMacPduRxBody.crcResult == 1)
                      {
                        // call function for rx packet processing
                        const uint32_t fpgaPayloadLengthHeaderSize = 4; // added by FPGA
                        if (phyUlschRxInd.ulschMacPduRxBody.macPduSize >= fpgaPayloadLengthHeaderSize)
                          {
                            uint32_t fpgaPayloadLength = 0;
                            for (int i = 0; i < fpgaPayloadLengthHeaderSize; ++i) {
                                fpgaPayloadLength |= (phyUlschRxInd.ulschMacPduRxBody.macPdu[i] << i*8);
                            }
                            NI_LOG_DEBUG("fpgaPayloadLength: " << fpgaPayloadLength);
                            const bool callbackValid = (m_niApiDataEndOkCallback != MakeNullCallback< bool, uint8_t* >());
                            if (callbackValid && m_ns3Running)
                              {
                                // TODO-NI: handover payload length to upper layers?
                                m_niApiDataEndOkCallback((uint8_t*)&phyUlschRxInd.ulschMacPduRxBody.macPdu[fpgaPayloadLengthHeaderSize]);
                              }
                          }
                        else
                          {
                            NI_LOG_FATAL("PHY_ULSCH_RX_IND corrupt");
                          }
                      }
                  }
                else
                  {
                    NI_LOG_FATAL("PHY_ULSCH_RX_IND received and device type is not eNB");
                  }
                done = true;
                break;
              case (PHY_DLSCH_RX_IND):
                m_numPhyDlschRxInd++;
                DeserializePhyDlschRxInd( &phyDlschRxInd, m_pBufU8Pipe3, &m_bufOffsetU8Pipe3 );

                for(int j = 0; j < numBytes; j++)
                  {
                    sprintf(&buffer[2*j], "%02X", phyDlschRxInd.dlschMacPduRxBody.macPdu[j]);
                  }
                NI_LOG_DEBUG ("DlschRxInd received with" <<
                              " SFN: " << phyDlschRxInd.subMsgHdr.sfn <<
                              " TTI: " << phyDlschRxInd.subMsgHdr.tti <<
                              " CRC: " << phyDlschRxInd.dlschMacPduRxBody.crcResult <<
                              " RNTI: " << phyDlschRxInd.dlschMacPduRxBody.rnti <<
                              " PDU Size: " << phyDlschRxInd.dlschMacPduRxBody.macPduSize <<
                              " PDU[]: " << buffer);

                if (m_niApiDevType == 1) // UE
                  {
                    if (phyDlschRxInd.dlschMacPduRxBody.crcResult == 1)
                      {
                        const bool callbackValid = (m_niApiDataEndOkCallback != MakeNullCallback< bool, uint8_t* >());
                        if (callbackValid && m_ns3Running)
                          {
                            NI_LOG_NONE("Start NS3 Rx processing");
                            // call function for rx packet processing
                            m_niApiDataEndOkCallback((uint8_t*)&phyDlschRxInd.dlschMacPduRxBody.macPdu);
                            NI_LOG_NONE("Finished NS3 Rx processing");
                          }
                      }
                  }
                else
                  {
                    NI_LOG_FATAL("PHY_DLSCH_RX_IND received and device type is not UE");
                  }
                done = true;
                break;
              case (PHY_CELL_MEASUREMENT_IND):
                {
                  m_numPhyCellMeasInd++;
                  DeserializePhyCellMeasurementInd( &phyCellMeasInd, m_pBufU8Pipe3, &m_bufOffsetU8Pipe3 );
                  if (phyCellMeasInd.cellMeasReportBody.numSubbandSinr > MAX_NUM_SUBBAND_SINR)
                    {
                      NI_LOG_FATAL("PHY_CELL_MEASUREMENT_IND numSubbandSinr > MAX_NUM_SUBBAND_SINR (" << phyCellMeasInd.cellMeasReportBody.numSubbandSinr << ")");
                    }
                  std::stringstream strSbSinr;
                  for(int j = 0; j < phyCellMeasInd.cellMeasReportBody.numSubbandSinr; j++)
                    {
                      strSbSinr << std::to_string(NiUtils::ConvertFxpI8_6_2ToDouble(phyCellMeasInd.cellMeasReportBody.subbandSinr[j])) << " ";
                    }
                  NI_LOG_DEBUG ("CellMeasInd received with" <<
                                " SFN: " << phyCellMeasInd.subMsgHdr.sfn <<
                                " TTI: " << phyCellMeasInd.subMsgHdr.tti <<
                                " CELL ID: " << phyCellMeasInd.cellMeasReportBody.cellId <<
                                " WBSINR: " << NiUtils::ConvertFxpI8_6_2ToDouble(phyCellMeasInd.cellMeasReportBody.widebandSinr) <<
                                " Num SBSINR: " << phyCellMeasInd.cellMeasReportBody.numSubbandSinr <<
                                " SBSINR[]: " << strSbSinr.str());
                  if (m_niApiDevType == 1) // UE
                    {
                      const bool callbackValid = (m_niApiCellMeasurementEndOkCallback != MakeNullCallback< bool, PhyCellMeasInd >());
                      if (callbackValid && m_ns3Running)
                        {
                          m_niApiCellMeasurementEndOkCallback(phyCellMeasInd);
                        }
                    }
                  done = true;
                  break;
                }
              case (PHY_CNF):
                DeserializePhyCnf( &phyCnf, m_pBufU8Pipe3, &m_bufOffsetU8Pipe3 );
                str = " for srcMsgType:" + std::to_string((uint16_t)phyCnf.cnfBody.srcMsgType) +
                      " received in SFN: " +
                       std::to_string((uint16_t) phyCnf.subMsgHdr.sfn) + " TTI: " +
                       std::to_string((uint8_t) phyCnf.subMsgHdr.tti);
                m_numPhyCnf[phyCnf.cnfBody.cnfStatus]++;
                switch (phyCnf.cnfBody.cnfStatus)
                {
                  case CNF_SUCCESS:
                    NI_LOG_DEBUG("CNF_SUCCESS" << str);
                    break;
                  case CNF_UNKNOWN_MESSAGE:
                    NI_LOG_WARN("CNF_UNKNOWN_MESSAGE" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_UNKNOWN_MESSAGE" << str);
                    break;
                  case CNF_MESSAGE_NOT_SUPPORTED:
                    NI_LOG_WARN("CNF_MESSAGE_NOT_SUPPORTED" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_MESSAGE_NOT_SUPPORTED" << str);
                    break;
                  case CNF_UNKNOWN_PARAMETER_SET:
                    NI_LOG_WARN("CNF_UNKNOWN_PARAMETER_SET" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_UNKNOWN_PARAMETER_SET" << str);
                    break;
                  case CNF_MISSING_PARAMETER_SET:
                    NI_LOG_WARN("CNF_MISSING_PARAMETER_SET" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_MISSING_PARAMETER_SET" << str);
                    break;
                  case CNF_PARAMETER_SET_REPETITION:
                    NI_LOG_WARN("CNF_PARAMETER_SET_REPETITION" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_PARAMETER_SET_REPETITION" << str);
                    break;
                  case CNF_RANGE_VIOLATION:
                    NI_LOG_WARN("CNF_RANGE_VIOLATION" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_RANGE_VIOLATION" << str);
                    break;
                  case CNF_STATE_VIOLATION:
                    NI_LOG_WARN("CNF_STATE_VIOLATION" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_STATE_VIOLATION" << str);
                    break;
                  case CNF_TIMEOUT:
                    NI_LOG_WARN("CNF_TIMEOUT" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_TIMEOUT" << str);
                    break;
                  case CNF_CONFIG_PAYLOAD_MISMATCH:
                    NI_LOG_WARN("CNF_CONFIG_PAYLOAD_MISMATCH" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_CONFIG_PAYLOAD_MISMATCH" << str);
                    break;
                  case CNF_LENGTH_MISMATCH:
                    NI_LOG_WARN("CNF_LENGTH_MISMATCH" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_LENGTH_MISMATCH" << str);
                    break;
                  case CNF_INPUT_BUFFER_FULL:
                    NI_LOG_WARN("CNF_INPUT_BUFFER_FULL" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_INPUT_BUFFER_FULL" << str);
                    break;
                  case CNF_INTERNAL_ERROR:
                    NI_LOG_WARN("CNF_INTERNAL_ERROR" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_INTERNAL_ERROR" << str);
                    break;
                  case CNF_INSTANCE_ID_MISMATCH:
                    NI_LOG_WARN("CNF_INSTANCE_ID_MISMATCH" << str);
                    NI_LOG_CONSOLE_DEBUG("CNF_INSTANCE_ID_MISMATCH" << str);
                    break;
                  default:
                    NI_LOG_FATAL("Received UNKNOWN confirm. pipe=" << m_pipe_name_3 << " cnfStatus=" << phyCnf.cnfBody.cnfStatus);
                    break;
                }
                done = true;
                break;
              default:
                NI_LOG_FATAL("Received UNKNOWN message. pipe=" << m_pipe_name_3 << " msgType=" << msgType);
                done = true;
                break;
            }
          }
        else {
            iteration++;
            if (iteration > 1000) {
                NI_LOG_NONE ("1000 x nothing received");
                iteration = 0;
            }
        }
        if (done || m_rxThreadStop) break;
        nanosleep( &m_ts, NULL );   // wait a little bit and then start over again
      }
    }
}

//======================================================================================
int32_t NiPipeTransport::CreateAndSendDlTxConfigReqMsg(
    uint32_t sfn,
    uint32_t tti,
    uint32_t prbAlloc,
    uint32_t rnti,
    uint32_t mcs,
    uint32_t tbsSize
)
{
  NI_LOG_DEBUG ("Create DL Config REQ Message with" <<
                " sfn: " << sfn <<
                ", tti: " << tti <<
                ", prbAlloc: " << (std::bitset<32>) prbAlloc <<
                ", rnti: " << rnti <<
                ", mcs: " << mcs <<
                ", tbsSize: " << tbsSize);

  int32_t  nwrite = 0;

  // create dl config request data structure and initialize
  PhyDlTxConfigReq phyDlTxConfigReq;
  InitializePhyDlTxConfigReq(  &phyDlTxConfigReq );
  // update general and sub message headers
  phyDlTxConfigReq.genMsgHdr.refId                      = m_msgRefId++;
  phyDlTxConfigReq.subMsgHdr.cnfMode                    = 1;    // request confirmations from PHY
  // TODO-NI: re-calculate NS-3 timing to PHY timing
  phyDlTxConfigReq.subMsgHdr.sfn                        = GetTimingIndSfn(); //TODO-NI: replace by caller Sfn
  phyDlTxConfigReq.subMsgHdr.tti                        = GetTimingIndTti(); //TODO-NI: replace by caller Tti
  m_dlTxConfigSfn = phyDlTxConfigReq.subMsgHdr.sfn;
  m_dlTxConfigTti = phyDlTxConfigReq.subMsgHdr.tti;
  // update dlsch config
  phyDlTxConfigReq.dlschTxConfigBody.macPduIndex        = ++m_macPduIndex;
  phyDlTxConfigReq.dlschTxConfigBody.rnti               = rnti;
  phyDlTxConfigReq.dlschTxConfigBody.prbAllocation      = prbAlloc;
  phyDlTxConfigReq.dlschTxConfigBody.mcs                = mcs;
  // update dci config
  phyDlTxConfigReq.dciTxConfigDlGrantBody.rnti          = rnti;
  phyDlTxConfigReq.dciTxConfigDlGrantBody.cceOffset     = 0;
  phyDlTxConfigReq.dciTxConfigDlGrantBody.prbAllocation = prbAlloc;
  phyDlTxConfigReq.dciTxConfigDlGrantBody.mcs           = mcs;
  phyDlTxConfigReq.dciTxConfigDlGrantBody.tpc           = 0;

  // serialize dl config request message
  m_bufOffsetU8Pipe2 = 0;


  SerializePhyDlTxConfigReq( &phyDlTxConfigReq, m_pBufU8Pipe2, &m_bufOffsetU8Pipe2 );

  // send tx config request message to lte application framework l1-l2 api
  nwrite = NiPipe::PipeWrite( &m_fd2, m_pBufU8Pipe2, (uint16_t)m_bufOffsetU8Pipe2 );
  m_numPhyDlTxConfigReq++;

  return nwrite;
}

int32_t NiPipeTransport::CreateAndSendDlTxPayloadReqMsg(
    uint32_t sfn,
    uint32_t tti,
    uint8_t* macPduPacket,
    uint32_t tbsSize
)
{
  int32_t  nwrite     = 0;

  const int numBytes = 32;
  char buffer [numBytes*2+1];
  for(int j = 0; j < numBytes; j++)
    {
      sprintf(&buffer[2*j], "%02X", macPduPacket[j]);
    }

  NI_LOG_DEBUG ("Create DL Payload REQ Message with"
                " sfn: " << sfn <<
                ", tti: " << tti <<
                ", macPduPacket[]: " << buffer <<
                ", tbsSize: " << tbsSize);

  // create dl payload request data structure and initialize
  PhyDlTxPayloadReq phyDlTxPayloadReq;
  InitializePhyDlTxPayloadReq( &phyDlTxPayloadReq );
  // update general and sub message headers
  phyDlTxPayloadReq.genMsgHdr.refId                   = m_msgRefId++;
  phyDlTxPayloadReq.genMsgHdr.bodyLength              = 17+tbsSize;
  phyDlTxPayloadReq.subMsgHdr.cnfMode                 = 1;    // request confirmations from PHY
  phyDlTxPayloadReq.subMsgHdr.sfn                     = m_dlTxConfigSfn; //TODO-NI: replace by caller Sfn
  phyDlTxPayloadReq.subMsgHdr.tti                     = m_dlTxConfigTti; //TODO-NI: replace by caller Tti
  // update dlsch mac pdu fields
  phyDlTxPayloadReq.dlschMacPduTxHdr.parSetBodyLength = 4+tbsSize;
  phyDlTxPayloadReq.dlschMacPduTxBody.macPduIndex     = m_macPduIndex;
  phyDlTxPayloadReq.dlschMacPduTxBody.macPduSize      = tbsSize;

  // store mac pdu into buffer
  // TODO-NI: remove this memcpy by handing over the mempointer to the upper layer
  memcpy(phyDlTxPayloadReq.dlschMacPduTxBody.macPdu, macPduPacket, tbsSize);
  m_numPhyDlTxPayloadReq++;

  // serialize dl payload request message
  m_bufOffsetU8Pipe2 = 0;


  SerializePhyDlTxPayloadReq( &phyDlTxPayloadReq, m_pBufU8Pipe2, &m_bufOffsetU8Pipe2 );

  // send tx payload request message to lte application framework l1-l2 api
  nwrite = NiPipe::PipeWrite(&m_fd2, m_pBufU8Pipe2, (uint16_t)m_bufOffsetU8Pipe2);

  return nwrite;
}

int32_t NiPipeTransport::CreateAndSendUlTxPayloadReqMsg(
    uint32_t sfn,
    uint32_t tti,
    uint8_t* macPduPacket,
    uint32_t tbsSize
)
{
  int32_t  nwrite     = 0;

  const int numBytes = 32;
  char buffer [numBytes*2+1];
  for(int j = 0; j < numBytes; j++)
    {
      sprintf(&buffer[2*j], "%02X", macPduPacket[j]);
    }

  NI_LOG_DEBUG ("Create UL Payload REQ Message with"
                " sfn: " << sfn <<
                ", tti: " << tti <<
                ", macPduPacket[]: " << buffer <<
                ", tbsSize: " << tbsSize);

  // create ul payload request data structure and initialize
  PhyUlTxPayloadReq phyUlTxPayloadReq;
  InitializePhyUlTxPayloadReq( &phyUlTxPayloadReq );
  // update general and sub message headers
  phyUlTxPayloadReq.genMsgHdr.refId                   = m_msgRefId++;
  phyUlTxPayloadReq.genMsgHdr.bodyLength              = 17+tbsSize;
  phyUlTxPayloadReq.subMsgHdr.cnfMode                 = 1;    // request confirmations from PHY
  phyUlTxPayloadReq.subMsgHdr.sfn                     = GetTimingIndSfn(); //TODO-NI: replace by caller Sfn
  phyUlTxPayloadReq.subMsgHdr.tti                     = GetTimingIndTti(); //TODO-NI: replace by caller Tti
  // update dlsch mac pdu fields
  phyUlTxPayloadReq.ulschMacPduTxHdr.parSetBodyLength = 4+tbsSize;
  phyUlTxPayloadReq.ulschMacPduTxBody.macPduIndex     = m_macPduIndex;
  phyUlTxPayloadReq.ulschMacPduTxBody.macPduSize      = tbsSize;

  // store mac pdu into buffer
  // TODO-NI: remove this memcpy by handing over the mempointer to the upper layer
  memcpy(phyUlTxPayloadReq.ulschMacPduTxBody.macPdu, macPduPacket, tbsSize);
  m_numPhyUlTxPayloadReq++;

  // serialize dl payload request message
  m_bufOffsetU8Pipe2 = 0;


  SerializePhyUlTxPayloadReq( &phyUlTxPayloadReq, m_pBufU8Pipe2, &m_bufOffsetU8Pipe2 );

  // send tx payload request message to lte application framework l1-l2 api
  nwrite = NiPipe::PipeWrite(&m_fd2, m_pBufU8Pipe2, (uint16_t)m_bufOffsetU8Pipe2);

  return nwrite;
}

void NiPipeTransport::SetNiApiTimingIndEndOkCallback (NiPipeTransportTimingIndEndOkCallback c)
  {
    m_niApiTimingIndEndOkCallback = c;
  }

void NiPipeTransport::SetNiApiDataEndOkCallback (NiPipeTransportDataEndOkCallback c)
  {
    m_niApiDataEndOkCallback = c;
  }

void NiPipeTransport::SetNiApiCellMeasurementEndOkCallback (NiPipeTransportCellMeasurementEndOkCallback c)
  {
    m_niApiCellMeasurementEndOkCallback = c;
  }

void NiPipeTransport::SetNiApiDevType(uint8_t niApiDevType)
  {
    m_niApiDevType = niApiDevType;
  }

} //namespace ns3
