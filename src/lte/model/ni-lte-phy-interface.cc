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



#include <list>
#include <bitset>

#include "ns3/string.h"
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/log.h>

#include "ns3/packet.h"

#include "lte-radio-bearer-tag.h"
#include "lte-rlc-tag.h"
#include "lte-pdcp-tag.h"

#include "ni-lte-phy-interface.h"

#include "ns3/ni-api-rlc-tag-header.h"
#include "ns3/ni-api-pdcp-tag-header.h"
#include "ns3/ni-api-radio-bearer-header.h"
#include "ns3/ni-api-packet-tag-info-header.h"

#include "ns3/ni.h"
#include "ns3/ni-l1-l2-api-lte-tables.h"

#include "ns3/ni-lte-constants.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE ("NiLtePhyInterface");

  NS_OBJECT_ENSURE_REGISTERED (NiLtePhyInterface);

  TypeId
  NiLtePhyInterface::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NiLtePhyInterface")
      .SetParent<Object> ()
      .SetGroupName("Ni")
      .AddConstructor<NiLtePhyInterface> ()
      .AddAttribute ("niApiDevType",
                     "To set whether the simulation should be run as eNB or UE",
                     StringValue ("NIAPI_ENB"),
                     MakeStringAccessor (&NiLtePhyInterface::SetNiApiDevType),
                     MakeStringChecker ())
      .AddAttribute ("enableNiApiLoopback",
                     "Enable NI API loopback mode",
                     BooleanValue (false),
                     MakeBooleanAccessor (&NiLtePhyInterface::SetNiApiLoopbackEnable),
                     MakeBooleanChecker ())
      .AddAttribute ("niChSinrValueDb",
                     "Channel SINR value for emulation in dB",
                     DoubleValue (30.0),
                     MakeDoubleAccessor (&NiLtePhyInterface::SetNiChannelSinrValue),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("niCqiReportPeriodMs",
                     "Period for generated CQI reports in milliseconds",
                     UintegerValue (100),
                     MakeUintegerAccessor (&NiLtePhyInterface::m_niCqiReportPeriodMs),
                     MakeUintegerChecker<uint8_t> ())
      .AddAttribute ("enableNiApi",
                     "Enable NI API",
                     BooleanValue (false),
                     MakeBooleanAccessor (&NiLtePhyInterface::SetNiApiEnable),
                     MakeBooleanChecker ())

// Start of 5G Attributes
       //Sub-Carrier Spacing.
      .AddAttribute ("niDlScs",
                     "Desired subcarrier spacing for the downlink. 0=15kHz, 1=30kHz, 2=60kHz. 3=120kHz",
                     UintegerValue (NIAPI_SCS_15_KHZ),
                     MakeUintegerAccessor (&NiLtePhyInterface::SetNiDlSCSValue),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("niUlScs",
                     "Desired subcarrier spacing for the uplink. 0=15kHz, 1=30kHz, 2=60kHz. 3=120kHz",
                     UintegerValue (NIAPI_SCS_15_KHZ),
                     MakeUintegerAccessor (&NiLtePhyInterface::SetNiUlSCSValue),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("niApiLte5gEnabled",
                     "Enable the 5G API",
                     BooleanValue (false),
                     MakeBooleanAccessor (&NiLtePhyInterface::SetApiLte5gEnabled),
                     MakeBooleanChecker ())
      .AddAttribute ("nextScsSwitchTargetSfn",
                     "Desired SFN to make next subcarrier spacing switch",
                     UintegerValue (0),
                     MakeUintegerAccessor (&NiLtePhyInterface::SetScsSwitchTargetSfn),
                     MakeUintegerChecker<uint32_t> ())

//INTRODUCE THE SFN AS AN ATTRIBUTE TO GIVE THE USER THE ABILITY TO CHANGE IT IN THE ARGUMENTS.

// End of 5G Attributes

       ;
    return tid;
  }

  NiLtePhyInterface::NiLtePhyInterface ()
  {
    NS_LOG_FUNCTION (this);
    NS_FATAL_ERROR ("This constructor should not be called");
  }

  NiLtePhyInterface::NiLtePhyInterface (Ns3LteDevType_t ns3DevType)
  : m_ns3DevType(ns3DevType),
    m_niApiDevType(NIAPI_ENB),
    m_enableNiApi(0),
    m_enableNiApiLoopback(0),
    m_initializationDone(false),
    m_mibReceived(false),
    m_sfnSynced(false),
    m_mibSfn(0),
    m_sfn(0),
    m_tti(0),
    m_cellId(0),
    m_niApiCountTxControlDataPackets(0),
    m_niApiCountTxPayloadDataPackets(0),
    m_rbBitmap(0),
    m_rnti(0),
    m_mcs(0),
    m_tbsSize(0),
    m_firstPhyTimingInd(false),
    m_sfnSfOffset(0),
    m_lastTimingIndTimeUs(0),
    m_dlscs(NIAPI_SCS_15_KHZ), //DL SCS for 5G API
    m_ulscs(NIAPI_SCS_15_KHZ), //UL SCS for 5G API
    m_ApiLte5Genabled(false), //5G API enabled variable
    m_ScsSwitchTargetSfn(0),//SFN to do next SCS switch
    m_niLteSdrTimingSync(CreateObject <NiLteSdrTimingSync> ())
  {

  }

  NiLtePhyInterface::~NiLtePhyInterface ()
  {
    // TODO Auto-generated destructor stub
  }

  // automatically called by NS-3
  void
  NiLtePhyInterface::DoDispose (void)
  {
    if (m_enableNiApi && m_initializationDone
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
      {
        if (m_enableNiApiLoopback)
          {
            // de-initialize ni udp transport layer - used here only for debug purpose
            DeInitializeNiUdpTransport();
          }
        else
          {
            // de- initialize ni pipe transport layer
            DeInitializeNiPipeTransport();
            // initialize SDR timing synchronization
            DeInitializeNiLteSdrTimingSync();
          }
        m_enableNiApi = false;
        m_initializationDone = false;
      }
    else
    {
      m_enableNiApi = false;
      m_initializationDone = false;
    }
  }

  // automatically called by NS-3
  void
  NiLtePhyInterface::DoInitialize (void)
  {
    NI_LOG_DEBUG(this << "NiLtePhyInterface::DoInitialize with " << 
                        " m_enableNiApi:" << m_enableNiApi <<
                        ", m_ns3DevType:" << m_ns3DevType <<
                        ", m_niApiDevType:" << m_niApiDevType <<
                        ", m_enableNiApiLoopback:" << m_enableNiApiLoopback);

    if (m_enableNiApi && !m_initializationDone
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
      {
        if (m_enableNiApiLoopback)
          {
            // initialize ni udp transport layer - used here only for debug purpose
            InitializeNiUdpTransport();
          }
        else
          {
            // initialize SDR timing synchronization
            InitializeNiLteSdrTimingSync();
            // initialize ni pipe transport layer
            InitializeNiPipeTransport();
          }

        // in the case of an ue schedule first call for cqi report generation
        if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE))
          {
            Simulator::Schedule(Seconds(1), &NiLtePhyInterface::NiGenerateCqiReport, this);
          }
        m_initializationDone = true;
      }
  }

  void
  NiLtePhyInterface::SetNiPhyTimingIndEndOkCallback (NiPhyTimingIndEndOkCallback c)
  {
    m_niPhyTimingIndEndOkCallback = c;
  }

  void
  NiLtePhyInterface::SetNiPhyRxDataEndOkCallback (NiPhyRxDataEndOkCallback c)
  {
    m_niPhyRxDataEndOkCallback = c;
  }

  void
  NiLtePhyInterface::SetNiPhyRxCtrlEndOkCallback (NiPhyRxCtrlEndOkCallback c)
  {
    m_niPhyRxCtrlEndOkCallback = c;
  }

  void
  NiLtePhyInterface::SetNiPhyRxCqiReportCallback (NiPhyRxCqiReportCallback c)
  {
    m_niPhyRxCqiReportCallback = c;
  }

  void
  NiLtePhyInterface::SetNiPhySpectrumModelCallback (NiPhySpectrumModelCallback c)
  {
    m_niPhySpectrumModelCallback = c;
  }

  void
  NiLtePhyInterface::InitializeNiUdpTransport ()
  {
    NI_LOG_NONE (this << " " << __FILE__ << " " << __func__);

    // enable udp transport layer only if ni api is enabled
    if (m_enableNiApi && m_enableNiApiLoopback
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
    {
        // deriving thread priorities from ns3 main process
        int ns3Priority = NiUtils::GetThreadPrioriy(); // -60 htop value
        const int rxThreadPriority = ns3Priority - 2;  // -58 htop value
        // create udp transport object
        m_niUdpTransport = CreateObject <NiUdpTransport> ("LTE");
        // create udp tx/rx sockets for enb config
        if (((m_niApiDevType==NIAPI_ENB)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_ENB)){
            // set callback function for rx packets
            m_niUdpTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiLtePhyInterface::NiStartRxCtrlDataFrame, this));
            m_niUdpTransport->OpenUdpSocketTx(m_niUdpSta1RemoteIpAddrTx, m_niUdpSta1RemotePortTx);
            m_niUdpTransport->OpenUdpSocketRx(m_niUdpSta1LocalPortRx, rxThreadPriority);
        }
        // create udp tx/rx sockets for ue config
        else if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE)) {
            // set callback function for rx packets
            m_niUdpTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiLtePhyInterface::NiStartRxCtrlDataFrame, this));
            m_niUdpTransport->OpenUdpSocketTx(m_niUdpSta2RemoteIpAddrTx, m_niUdpSta2RemotePortTx);
            m_niUdpTransport->OpenUdpSocketRx(m_niUdpSta2LocalPortRx, rxThreadPriority);
        }
        else {
            // do not create udp tx/rx sockets - for virtual enb/ ue
        }
    }
    else {
        NI_LOG_FATAL("Init UDP Transport with wrong API mode!");
    }
  }

  void
  NiLtePhyInterface::DeInitializeNiUdpTransport ()
  {
    NI_LOG_NONE (this << " " << __FILE__ << " " << __func__);

    // de-init udp transport layer only if ni api was enabled
    if (m_enableNiApi && m_enableNiApiLoopback
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
    {
        if (((m_niApiDevType==NIAPI_ENB)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_ENB)){
            m_niUdpTransport->CloseUdpSocketTx();
            m_niUdpTransport->CloseUdpSocketRx();
            // remove call back
            m_niUdpTransport->SetNiApiDataEndOkCallback (MakeNullCallback< bool, uint8_t* >());
        }
        // close udp tx/rx sockets for ue config
        else if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE)) {
            m_niUdpTransport->CloseUdpSocketTx();
            m_niUdpTransport->CloseUdpSocketRx();
            // remove call back
            m_niUdpTransport->SetNiApiDataEndOkCallback (MakeNullCallback< bool, uint8_t* >());
        }
        else {
            // do not create udp tx/rx sockets - for virtual enb/ ue
        }
    }
    else {
        NI_LOG_FATAL("De-Init UDP Transport with wrong API mode!");
    }
  }

  void
  NiLtePhyInterface::InitializeNiPipeTransport ()
  {
    // enable pipe transport layer only if ni api is enabled
    if (m_enableNiApi && !m_enableNiApiLoopback
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
    {
        //  check device configuration
        if ( ((m_niApiDevType==NIAPI_ENB)&&(m_ns3DevType==NS3_ENB)) ||
            ((m_niApiDevType==NIAPI_UE)&&(m_ns3DevType==NS3_UE)) )
          {
            // deriving thread priorities from ns3 main process
            int ns3Priority = NiUtils::GetThreadPrioriy();
            const int niTransportPhyTimingIndPrio = ns3Priority; // -60 htop value
            const int niTransportPhyRxPrio = ns3Priority - 2;    // -58 htop value
            // create udp transport object
            m_niPipeTransport = CreateObject <NiPipeTransport> ("LTE");
            // init transport layer for LTE
            m_niPipeTransport->Init(niTransportPhyTimingIndPrio, niTransportPhyRxPrio);
            // set call back for timing indication
            m_niPipeTransport->SetNiApiTimingIndEndOkCallback(MakeCallback (&NiLtePhyInterface::NiStartTimingIndHandler, this));
            // set call back for Rx Control and Data Frames
            m_niPipeTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiLtePhyInterface::NiStartRxCtrlDataFrame, this));
            // set call back for Rx Cell Measurement Report
            m_niPipeTransport->SetNiApiCellMeasurementEndOkCallback (MakeCallback (&NiLtePhyInterface::NiStartRxCellMeasurementIndHandler, this));
            // set device type
            m_niPipeTransport->SetNiApiDevType(m_niApiDevType);
          }
    }
    else {
        NI_LOG_FATAL("Init PIPE Transport with wrong API mode!");
    }
  }

  void
  NiLtePhyInterface::DeInitializeNiPipeTransport ()
  {
    // de-init pipe transport layer only if ni api is enabled
    if (m_enableNiApi && !m_enableNiApiLoopback
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
    {
        //  check device configuration
        if ( ((m_niApiDevType==NIAPI_ENB)&&(m_ns3DevType==NS3_ENB)) ||
            ((m_niApiDevType==NIAPI_UE)&&(m_ns3DevType==NS3_UE)) )
          {
            // de-init transport layer for LTE
            m_niPipeTransport->DeInit();
            // remove call backs
            m_niPipeTransport->SetNiApiTimingIndEndOkCallback(MakeNullCallback< bool, uint16_t, uint8_t, bool >());
            m_niPipeTransport->SetNiApiDataEndOkCallback (MakeNullCallback< bool, uint8_t* >());
            m_niPipeTransport->SetNiApiCellMeasurementEndOkCallback (MakeNullCallback< bool, PhyCellMeasInd >());
          }
    }
    else {
        NI_LOG_FATAL("De-Init PIPE Transport with wrong API mode!");
    }
  }

  void
  NiLtePhyInterface::InitializeNiLteSdrTimingSync ()
  {
    // de-init SDR timing sync only if ni api is enabled
    if (m_enableNiApi && !m_enableNiApiLoopback
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
    {
        //  check device configuration
        if ( ((m_niApiDevType==NIAPI_ENB)&&(m_ns3DevType==NS3_ENB)) ||
            ((m_niApiDevType==NIAPI_UE)&&(m_ns3DevType==NS3_UE)) )
          {
            // align simulator against FPGA synchronized wall clock
            m_niLteSdrTimingSync->AttachWallClock();
          }
    }
    else {
        NI_LOG_FATAL("Init NiLteSdrTimingSync with wrong API mode!");
    }
  }

  void
  NiLtePhyInterface::DeInitializeNiLteSdrTimingSync ()
  {
    // enable pipe transport layer only if ni api is enabled
    if (m_enableNiApi && !m_enableNiApiLoopback
    		&& m_ns3DevType != NS3_NOAPI) ////DALI: fake nodes should not proceed with Ni Api enabled
    {
        //  check device configuration
        if ( ((m_niApiDevType==NIAPI_ENB)&&(m_ns3DevType==NS3_ENB)) ||
            ((m_niApiDevType==NIAPI_UE)&&(m_ns3DevType==NS3_UE)) )
          {
            // TODO-NI: detach from wall clock
            //m_niLteSdrTimingSync->DetachWallClock();
          }
    }
    else {
        NI_LOG_FATAL("Init NiLteSdrTimingSync with wrong API mode!");
    }
  }

  void
  NiLtePhyInterface::SetNiApiDevType (std::string type)
  {
    NI_LOG_DEBUG(this << " - Set NI LTE API Mode to " << type);

    if (type == "NIAPI_ENB")
      m_niApiDevType = NIAPI_ENB;
    else if (type == "NIAPI_UE")
      m_niApiDevType = NIAPI_UE;
    else if (type == "NIAPI_ALL")
      m_niApiDevType = NIAPI_ALL;
    else if (type == "NIAPI_NONE")
      m_niApiDevType = NIAPI_NONE;
    else
      NI_LOG_FATAL("\nNiLtePhyInterface::m_niApiDevType: Unrecognizable option for running the program in NI API device mode");



    return;
  }

  NiApiDevType_t
  NiLtePhyInterface::GetNiApiDevType () const
  {
    return m_niApiDevType;
  }

  Ns3LteDevType_t
  NiLtePhyInterface::GetNs3DevType () const
  {
    return m_ns3DevType;
  }

  void
  NiLtePhyInterface::SetNiApiEnable (bool enable)
  {
    NI_LOG_DEBUG(this << " - Set NI LTE API Mode to " << enable);

      m_enableNiApi = enable;
      this->DoInitialize();

  }

  bool
  NiLtePhyInterface::GetNiApiEnable () const
  {
    return m_enableNiApi;
  }

  void
  NiLtePhyInterface::SetNiApiLoopbackEnable (bool enable)
  {
    NI_LOG_DEBUG(this << " - Set NI LTE API Loopback Mode to " << enable);

    m_enableNiApiLoopback = enable;

  }

  bool
  NiLtePhyInterface::GetNiApiLoopbackEnable () const
  {
    return m_enableNiApiLoopback;
  }

  void
  NiLtePhyInterface::SetNiChannelSinrValue (double chSinrDb)
  {
    NI_LOG_DEBUG(this << " - Set Channel SINR value to " << chSinrDb << "db");

    m_chSinrDb  = chSinrDb;

    m_chSinrLin = pow(10, m_chSinrDb/10);
  }

  double
  NiLtePhyInterface::GetNiChannelSinrValue () const
  {
    return m_chSinrDb;
  }


  // *************************** 5G Set/Get Functions.

   void
   NiLtePhyInterface::SetNiDlSCSValue (NiApiScs_t scs)
   {
     NI_LOG_DEBUG(this << " - Set SCS value to " << scs << "kHz");

     m_dlscs  = scs;
   }

   NiApiScs_t
   NiLtePhyInterface::GetNiDlSCS () const
   {
     return m_dlscs;
   }

   void
    NiLtePhyInterface::SetNiUlSCSValue (NiApiScs_t scs)
    {
      NI_LOG_DEBUG(this << " - Set SCS value to " << scs << "kHz");

      m_ulscs  = scs;
    }

    NiApiScs_t
    NiLtePhyInterface::GetNiUlSCS () const
    {
      return m_ulscs;
    }


    void
     NiLtePhyInterface::SetApiLte5gEnabled (bool fiveGenabled)
     {
       NI_LOG_DEBUG(this << " - Set 5G framework to " << fiveGenabled << "kHz");

       m_ApiLte5Genabled = fiveGenabled;
     }

     bool
     NiLtePhyInterface::GetApiLte5gEnabled () const
     {
       return m_ApiLte5Genabled;
     }
     void
	 NiLtePhyInterface::SetScsSwitchTargetSfn(uint32_t ScsSwitchTargetSfn)
     {
       NI_LOG_DEBUG(this << " - Set SFN to switch Subcarrier Spacing to " << ScsSwitchTargetSfn);

       m_ScsSwitchTargetSfn = ScsSwitchTargetSfn;
     }

     double
     NiLtePhyInterface::GetScsSwitchTargetSfn() const
     {
       return m_ScsSwitchTargetSfn;
     }
     // *************************** End of 5G Set/Get Functions.
  uint64_t
  NiLtePhyInterface::NiSfnTtiCounterSync (uint32_t* m_nrFrames, uint32_t* m_nrSubFrames)
  {
    uint64_t offset = 0;

    // TODO-NI: sync also with lte afw counter via phy tmg ind, should be constant offset
    if ((m_mibReceived)&&(!m_sfnSynced)){
        // in ul the ue ns-3 counter are synced when receiving a mib
        // synchronize local sfn and tti counters
        m_sfn = m_mibSfn;
        m_tti = 1; // MIB is received in the TTI 1 (last subframe)
        m_tti++;   // Now the next subframe is started -> TTI 2
        if (!m_enableNiApiLoopback)
          {
            // add delay fot OTA transmission
            m_tti += 2; // 1 TTI for Preparation at eNB, 1 TTI for Transmission over PHY
          }
        // calculate offset in number of subrames
        int64_t signedOffset = (int64_t)((*m_nrFrames) * 10 + (*m_nrSubFrames)) - (int64_t)(m_sfn * 10 + m_tti);
        offset = (uint64_t) llabs(signedOffset);
        NI_LOG_DEBUG(this << " - SfnTtiCounterSync: MIB received - synchronizing sfn: "
                          << *m_nrFrames << "->" << m_sfn << " and tti: "  << *m_nrSubFrames << "->" << (uint16_t) m_tti
                          << ", offset: " << signedOffset);
        // synchronize ns-3 sfn and tti counters
        *m_nrFrames    = m_sfn;
        *m_nrSubFrames = m_tti;
        // update state variables
        m_mibReceived  = false;
        m_sfnSynced    = true;
    } else {
        // synchronize local sfn and tti counters
        m_sfn = *m_nrFrames;
        m_tti = *m_nrSubFrames;
    }
    return offset;
  }

  void
  NiLtePhyInterface::NiGenerateCqiReport ()
  {

    // here only wideband SINR/CQI is used
    // TODO-NI: add subband SINR/CQI if needed

    NI_LOG_DEBUG(this << " - NI CQI Report generated with SINR " << m_chSinrDb << " dB");

    // convert sinr value ns-3 SpectrumValue format
    Ptr<LteSpectrumPhy>      spectrumPhy   = m_niPhySpectrumModelCallback();
    Ptr<const SpectrumModel> spectrumModel = spectrumPhy->GetRxSpectrumModel();
    Ptr<SpectrumValue>       spectrumSinr  = Create<SpectrumValue>(spectrumModel);

    // add sinr value to SpectrumValue vector
    (*spectrumSinr) += m_chSinrLin;

    // call function to create cqi report
    m_niPhyRxCqiReportCallback(*spectrumSinr);

    // schedule next function call
    Simulator::Schedule(MilliSeconds(m_niCqiReportPeriodMs), &NiLtePhyInterface::NiGenerateCqiReport, this);
  }

  bool
  NiLtePhyInterface::NiStartDlTxApiSend (uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset)
  {
    // sanity checks
    if (*payloadDataBufOffset > m_tbsSize) {
        NI_LOG_FATAL (this << " - DL: payloadDataBufOffset(" << *payloadDataBufOffset << ") > m_tbsSize(" << m_tbsSize << ") "
                      << m_mcs << ", " << m_rbBitmap);
    }

    // chose api transport layer
    if (m_enableNiApiLoopback){ // send message over udp loopback channel to rx station
        m_niUdpTransport->SendToUdpSocketTx(payloadDataBuffer, m_tbsSize);
    } else { // send message over pipe connection to lte app framework
    //Enables/disables the 5G message in the DL TX message chain
    if(m_ApiLte5Genabled){
    // create & send an ni api 5G downlink tx control request message
        if (m_niPipeTransport->CreateAndSendFiveGDlTxConfigReqMsg(m_sfn, m_tti, m_dlscs) < 0){
            NI_LOG_FATAL (this << " - DL: Could not send 5G DL Config Request Message");
        }
    }
    // create & send an ni api downlink tx control request message
        if (m_niPipeTransport->CreateAndSendDlTxConfigReqMsg(m_sfn, m_tti, m_rbBitmap, m_rnti, m_mcs, m_tbsSize) < 0){
            NI_LOG_FATAL (this << " - DL: Could not send DL Config Request Message");
        }
        // create & send an ni api downlink tx payload request message
        if (m_niPipeTransport->CreateAndSendDlTxPayloadReqMsg(m_sfn, m_tti, payloadDataBuffer, m_tbsSize) < 0){
            NI_LOG_FATAL (this << " - DL: Could not send DL Payload Request Message");
        }
    }
    // call rx function directly - useful for debugging
    //NiStartRxCtrlDataFrame((uint8_t*)&payloadDataBuffer);

  }

//******************** 5G Message setting and sending functions.
//Note: The function related to DL TX was added directly on NiLtePhyInterface::NiStartDlTxApiSend
// as the API defined the sequence of messages as such, and it streamlined the implementation.
bool
   NiLtePhyInterface::NiStartDlRxApiSend ()
   {
  // create & send an ni api 5G downlink rx control request message
     if (m_niPipeTransport->CreateAndSendFiveGDlRxConfigReqMsg(m_sfn, m_tti, m_dlscs) < 0){
         NI_LOG_FATAL (this << " - DL: Could not send 5G DL Rx Config Request Message");
     }
   }

bool
   NiLtePhyInterface::NiStartUlTxApiSend (uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset)
   {

      if (*payloadDataBufOffset > m_tbsSize) {
          NI_LOG_FATAL (this << " - DL: payloadDataBufOffset > m_tbsSize");
      }
      // create & send an ni api 5G uplink tx control request message
      if (m_niPipeTransport->CreateAndSendFiveGUlTxConfigReqMsg(m_sfn, m_tti, m_ulscs) < 0){
          NI_LOG_FATAL (this << " - UL: Could not send 5G UL Tx Config Request Message");
      }
      if (m_niPipeTransport->CreateAndSendUlTxPayloadReqMsg(m_sfn, m_tti, payloadDataBuffer, m_tbsSize) < 0){
          NI_LOG_FATAL ("NiLtePhyInterface::NiStartTxCtrlDataFrame: Could not send UL Payload Req Message");
     }
   }

bool
   NiLtePhyInterface::NiStartUlRxApiSend ()
   {
  // create & send an ni api 5G uplink rx control request message
     if (m_niPipeTransport->CreateAndSendFiveGUlRxConfigReqMsg(m_sfn, m_tti, m_ulscs) < 0){
         NI_LOG_FATAL (this << " - UL: Could not send 5G UL Rx Config Request Message");
     }
   }
//******************** End of 5G Message setting and sending functions.

  bool
  NiLtePhyInterface::NiStartTxCtrlDataFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint32_t m_nrFrames, uint32_t m_nrSubFrames)
  {
    NI_LOG_TRACE("[Trace#20],NiStartTxCtrlDataFrameStart," << ( NiUtils::GetSysTime()-g_logTraceStartSubframeTime));

    if ((m_nrFrames!=m_sfn)||(m_nrSubFrames!=m_tti)){
        NI_LOG_DEBUG (this << " - SFN/TTI Counters out of sync");
    }

    // packet buffer: TODO-NI: create packet buffer in transport layer object and get pointer to it
    uint8_t  payloadDataBuffer[9500];
    // pointer to current pdu buffer offset
    uint32_t payloadDataBufOffset = 0;
    // counter of control messages
    uint32_t controlMessageCnt    = 0;
    uint32_t paylMessageCnt       = 0;

    uint32_t payloadDataBufOffsetTmp  = 0;
    uint32_t controlMessageCntTmp     = 0;

    // switch between enb and ue
    if (((m_niApiDevType==NIAPI_ENB)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_ENB)){
        // downlink transmitter

        // create api packet header
        struct NiApiPacketHeader niApiPacketHeader;
        niApiPacketHeader.niApiPacketType = NIAPI_DL_PACKET;

        // include this as first element in pdu message
        std::memcpy(payloadDataBuffer, (uint8_t*)&niApiPacketHeader, sizeof(niApiPacketHeader));
        payloadDataBufOffset += sizeof(niApiPacketHeader);

        // check if control messages available
        if (ctrlMsgList.size () > 0){
            NI_LOG_DEBUG(this << " - DL: Ctrl data available for TTI #" << (uint16_t) m_tti << " and SFN #" << m_sfn);

            // create rnti/ue map for counting ue specific messages
            std::map <uint16_t, uint16_t> rntiMap;

            // downlink broadcast control message processing (MIB, SIB, RAR) & search for rnti specific control messages
            NiStartTxDlCtrlFrameBc (packetBurst, ctrlMsgList, (uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset, controlMessageCnt, rntiMap);

            // save buffer pointer
            payloadDataBufOffsetTmp  = payloadDataBufOffset;
            controlMessageCntTmp     = controlMessageCnt;

            // check if rnti specific data were included
            if (rntiMap.empty()){
                // if not then use default dci
                m_rbBitmap = 255; // 6 RBGs --> For GFDM change to 255 = 8PRBs
                m_rnti     = 1;
                m_mcs      = 5;
                GetTbs(m_mcs, m_rbBitmap, &m_tbsSize);

                NI_LOG_DEBUG (this << " - DL: No UE data, use default DCI /"
                              << " buffer offset=" << payloadDataBufOffsetTmp
                              << " number of messages=" << controlMessageCnt
                              << " rbBitmap=" << (std::bitset<32>) m_rbBitmap
                              << " rnti=" << m_rnti
                              << " mcs=" << (uint16_t) m_mcs
                              << " tbsSize=" << (uint16_t) m_tbsSize << " bytes");

                // update api packet header infos
                niApiPacketHeader.nrFrames        = m_sfn;
                niApiPacketHeader.nrSubFrames     = m_tti;
                niApiPacketHeader.cellId          = m_cellId;
                niApiPacketHeader.rnti            = m_rnti;
                niApiPacketHeader.numCtrlMsg      = controlMessageCnt;
                niApiPacketHeader.numPaylMsg      = 0;
                // include packet header as first element in payload buffer
                std::memcpy(payloadDataBuffer, (uint8_t*)&niApiPacketHeader, sizeof(niApiPacketHeader));

                // send mac pdu with broadcast information to all ue's
                NiStartDlTxApiSend ((uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset);

            } else {
                // loop over all ue's to find corresponding control and payload messages
                uint16_t curRnti, numRntiPkts;
                for (std::map<uint16_t, uint16_t>::iterator itRntiMap=rntiMap.begin(); itRntiMap!=rntiMap.end(); ++itRntiMap)
                  {
                    curRnti     = itRntiMap->first;
                    numRntiPkts = itRntiMap->second;

                    NI_LOG_DEBUG (this << " - DL: UE data - " << numRntiPkts
                                  << " control messages available for rnti #" << curRnti
                                  << " / buffer offset=" << payloadDataBufOffsetTmp);

                    // reset payload buffer offset
                    payloadDataBufOffset  = payloadDataBufOffsetTmp;
                    // reset control message cnt
                    controlMessageCnt     = controlMessageCntTmp;

                    // downlink rnti specific control message processing (DL DCI, UL DCI)
                    // note: in the dl the payload data packet processing is done when receiving a dl dci message
                    NiStartTxDlCtrlFrameUc (packetBurst, ctrlMsgList, (uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset, controlMessageCnt, curRnti);

                    // update api packet header infos
                    niApiPacketHeader.nrFrames        = m_sfn;
                    niApiPacketHeader.nrSubFrames     = m_tti;
                    niApiPacketHeader.cellId          = m_cellId;
                    niApiPacketHeader.rnti            = m_rnti;
                    niApiPacketHeader.numCtrlMsg      = controlMessageCnt;
                    niApiPacketHeader.numPaylMsg      = paylMessageCnt;
                    // include packet header as first element in payload buffer
                    std::memcpy(payloadDataBuffer, (uint8_t*)&niApiPacketHeader, sizeof(niApiPacketHeader));

                    // send mac pdu via ni api to specific ue
                    NiStartDlTxApiSend ((uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset);

                  } // end rnti/ue loop
            }
        } else {
            // in case of using ni lte afw send default message every tti
            //if (!m_enableNiApiLoopback){

            //}
        } // end if ctrl msg list

    } else if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE)){
        // uplink transmitter

        // create api packet header
        struct NiApiPacketHeader niApiPacketHeader;
        niApiPacketHeader.niApiPacketType = NIAPI_UL_PACKET;

        // include this as first element in pdu message
        std::memcpy(payloadDataBuffer, (uint8_t*)&niApiPacketHeader, sizeof(niApiPacketHeader));
        payloadDataBufOffset += sizeof(niApiPacketHeader);

        // check if control messages available
        if (ctrlMsgList.size () > 0){
            NI_LOG_DEBUG(this << " - UL: Ctrl data available for TTI #" << (uint16_t) m_tti << " and SFN #" << m_sfn);

            // uplink control message processing
            NiStartTxUlCtrlFrame (packetBurst, ctrlMsgList, (uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset, controlMessageCnt);

        } // end if ctrl msg list

        // check ul packet burst that can come asynchronously to control messages
        if (packetBurst){
            NI_LOG_DEBUG(this << " - UL: Payload data available for TTI #" << (uint16_t) m_tti << " and SFN #" << m_sfn);

            // store payload packets from packet burst that belong to rnti in pdu
            NiStartTxDataFrame (packetBurst, (uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset, 0);

            paylMessageCnt = 1;
        }

        // do only send messages via api if control or payload is included
        if ((controlMessageCnt>0)||(paylMessageCnt>0)){

            // update api packet header infos
            niApiPacketHeader.numCtrlMsg  = controlMessageCnt;
            niApiPacketHeader.numPaylMsg  = paylMessageCnt;
            niApiPacketHeader.nrFrames    = m_sfn;
            niApiPacketHeader.nrSubFrames = m_tti;
            niApiPacketHeader.cellId      = m_cellId;
            niApiPacketHeader.rnti        = m_rnti;
            // include packet header this as first element in payload buffer
            std::memcpy(payloadDataBuffer, (uint8_t*)&niApiPacketHeader, sizeof(niApiPacketHeader));

            // chose api transport layer
            if (m_enableNiApiLoopback){ // send message over udp loopback channel to rx station
                m_niUdpTransport->SendToUdpSocketTx((uint8_t*)&payloadDataBuffer, payloadDataBufOffset);
            }
            else { // send message over pipe connection to lte app framework
              // create & send an ni api uplink tx payload request message
                //NiStartUlTxApiSend ((uint8_t*)&payloadDataBuffer, (uint32_t*)&payloadDataBufOffset);

            //enables/disables the 5G message in the UL TX chain.
            if(m_ApiLte5Genabled){
              if (m_niPipeTransport->CreateAndSendFiveGUlTxConfigReqMsg(m_sfn, m_tti, m_ulscs) < 0){
                 NI_LOG_FATAL (this << " - UL: Could not send 5G  UL Tx Config Request Message");
               }
            }
              if (m_niPipeTransport->CreateAndSendUlTxPayloadReqMsg(m_sfn, m_tti, (uint8_t*)&payloadDataBuffer, payloadDataBufOffset) < 0){
                NI_LOG_FATAL ("NiLtePhyInterface::NiStartTxCtrlDataFrame: Could not send UL Payload Req Message");
               }
            }
        }
        // call rx function directly - useful for debugging
        //NiStartRxCtrlDataFrame((uint8_t*)&payloadDataBuffer);

    } else {
        //NI_LOG_FATAL ("NiLtePhyInterface::NiStartTxCtrlDataFrame: NI device mode " << m_niApiDevType << " with NS3 device mode " << m_ns3DevType << " not allowed in DL");
        // do nothing
    }

    NI_LOG_TRACE(" [Trace#21],NiStartTxCtrlDataFrameEnd," << ( NiUtils::GetSysTime()-g_logTraceStartSubframeTime));

  } // end NiStartTxCtrlDataFrame function

  bool
  NiLtePhyInterface::NiStartRxCtrlDataFrame (uint8_t* payloadDataBuffer)
  {

    // pointer to current payload buffer offset
    uint32_t payloadDataBufOffset = 0;

    // create control message list and clear queue
    std::list<Ptr<LteControlMessage> > ctrlMsgList;
    ctrlMsgList.clear();
    // create payload packet burst
    Ptr<PacketBurst> packetBurst = Create <PacketBurst> ();

    // switch between enb and ue
    if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE)){
        // downlink receiver

        // create empty dl packet header
        struct NiApiPacketHeader niApiPacketHeader;
        // extract dl packet header
        std::memcpy((uint8_t*)&niApiPacketHeader, payloadDataBuffer, sizeof(niApiPacketHeader));
        payloadDataBufOffset += sizeof(niApiPacketHeader);

        NI_LOG_DEBUG (this << " - DL: Received MAC PDU with /"
                      << " control=" << (uint16_t) niApiPacketHeader.numCtrlMsg
                      << " payload=" << (uint16_t) niApiPacketHeader.numPaylMsg
                      << " SFN=" << niApiPacketHeader.nrFrames << " (" << m_sfn << ")"
                      << " TTI=" << (uint16_t) niApiPacketHeader.nrSubFrames <<  "(" << (uint16_t) m_tti << ")"
                      << " cellId=" << niApiPacketHeader.cellId
                      << " RNTI=" << niApiPacketHeader.rnti);

        for (uint32_t rxCtrlPacketCnt = 0; rxCtrlPacketCnt < niApiPacketHeader.numCtrlMsg ; rxCtrlPacketCnt  ++){
            // process all received downlink control messages
            // note: when receiving a dl dci message the payload is processed directly afterwards
            NiStartRxDlCtrlFrame (packetBurst, ctrlMsgList, payloadDataBuffer, (uint32_t*)&payloadDataBufOffset);
        }

    } else if (((m_niApiDevType==NIAPI_ENB)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_ENB)){
        // uplink receiver

        // create empty dl packet header
        struct NiApiPacketHeader niApiPacketHeader;
        // extract dl packet header
        std::memcpy((uint8_t*)&niApiPacketHeader, payloadDataBuffer, sizeof(niApiPacketHeader));
        payloadDataBufOffset += sizeof(niApiPacketHeader);

        NI_LOG_DEBUG (this << " - UL: Received MAC PDU with /"
                      << " control=" << (uint16_t) niApiPacketHeader.numCtrlMsg
                      << " payload=" << (uint16_t) niApiPacketHeader.numPaylMsg
                      << " SFN=" << niApiPacketHeader.nrFrames << " (" << m_sfn << ")"
                      << " TTI=" << (uint16_t) niApiPacketHeader.nrSubFrames <<  "(" << (uint16_t) m_tti << ")"
                      << " cellId=" << niApiPacketHeader.cellId
                      << " RNTI=" << niApiPacketHeader.rnti);

        // process all received control messages
        for (uint32_t rxCtrlPacketCnt = 0; rxCtrlPacketCnt < niApiPacketHeader.numCtrlMsg ; rxCtrlPacketCnt  ++){
            // process all received uplink control messages
            NiStartRxUlCtrlFrame (packetBurst, ctrlMsgList, payloadDataBuffer, (uint32_t*)&payloadDataBufOffset);
        }

        if (niApiPacketHeader.numPaylMsg > 0 ) {
            // extract uplink payload data packets from MAC PDU and store in a packet burst
            NiStartRxDataFrame (packetBurst, payloadDataBuffer, (uint32_t*)&payloadDataBufOffset);
        }

    } else {
        NI_LOG_ERROR ("NiLtePhyInterface::NiStartRxCtrlDataFrame UL: Device mode " << m_niApiDevType << " not allowed in DL with ns3DevType: " << m_ns3DevType);
        // do nothing
    }

    // check if packets available
    if (packetBurst) {
        for (std::list<Ptr<Packet> >::const_iterator itPacketBurst = packetBurst->Begin (); itPacketBurst != packetBurst->End (); ++itPacketBurst)
          {
            // call ns-3 PhyPduReceived function in lte end/ue phy for further payload packet processing
            m_niPhyRxDataEndOkCallback (*itPacketBurst);
          }
    }
    // check if control messages available
    if (ctrlMsgList.size () > 0){
        // call ns-3 ReceiveLteControlMessageList function in lte end/ue phy for further control message processing
        m_niPhyRxCtrlEndOkCallback (ctrlMsgList);
    }

  } // end NiStartRxCtrlDataFrame function

  bool
  NiLtePhyInterface::NiStartRxCellMeasurementIndHandler (PhyCellMeasInd phyCellMeasInd)
  {
    // update channel SINR value in case manual channel SINR setting via remote control engine is disabled
    if (g_RemoteControlEngine.GetPdb()->getParameterManualLteUeChannelSinrEnable() == false)
      {
        // convert fixed point to double
        double widebandSinr = NiUtils::ConvertFxpI8_6_2ToDouble(phyCellMeasInd.cellMeasReportBody.widebandSinr);
        // update channel SINR value
        SetNiChannelSinrValue(widebandSinr);
      }
    return true;
  }

  // call back function triggerd by PHY_TIMING_IND and provides SFN/TTI
  bool
  NiLtePhyInterface::NiStartTimingIndHandler (
                                uint16_t sfn,
                                uint8_t tti,
                                bool firstRun
                               )
  {
    NI_LOG_DEBUG (this << ", " << sfn << ", " << std::to_string(tti));
    m_firstPhyTimingInd = firstRun;
    m_niPhyTimingIndEndOkCallback(sfn, tti, firstRun);
    return true;
  }

  bool
  NiLtePhyInterface::NiStartTxDlCtrlFrameBc (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t &controlMessageCnt, std::map <uint16_t, uint16_t> &rntiMap)
  {
    // parse the control messages
    std::list<Ptr<LteControlMessage> >::iterator itCtrlMsg;

    // first collect all broadcast messages which are not rnti specific - these will be sent to all ue's
    itCtrlMsg = ctrlMsgList.begin ();
    while (itCtrlMsg != ctrlMsgList.end ())
      {
        Ptr<LteControlMessage> msg = (*itCtrlMsg);
        LteControlMessage::MessageType m_messageType = msg->GetMessageType ();

        switch (m_messageType) {
          case LteControlMessage::MIB:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              // extract mib control message
              Ptr<MibLteControlMessage> mib = DynamicCast<MibLteControlMessage> (msg);
              LteRrcSap::MasterInformationBlock mibElem = mib->GetMib();
              // store mivb in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&mibElem, sizeof(mibElem));
              *payloadDataBufOffset += sizeof(mibElem);

              NI_LOG_DEBUG (this << " - DL: MIB Message sent (Size=" << sizeof(mibElem) << " bytes) /"
                            " SFN=" << (uint16_t) mibElem.systemFrameNumber <<
                            " DL-BW=" << (uint16_t) mibElem.dlBandwidth);
              break;
            }
          case LteControlMessage::SIB1:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              // extract sib1 control message
              Ptr<Sib1LteControlMessage> sib1 = DynamicCast<Sib1LteControlMessage> (msg);
              LteRrcSap::SystemInformationBlockType1 sib1Elem = sib1->GetSib1();
              // extract cell id
              m_cellId = sib1Elem.cellAccessRelatedInfo.cellIdentity;
              // store sib1 in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&sib1Elem, sizeof(sib1Elem));
              *payloadDataBufOffset += sizeof(sib1Elem);

              NI_LOG_DEBUG (this << " - DL: SIB1 Message sent (Size=" << sizeof(sib1Elem) << " bytes) /"
                            " Cell ID=" << (uint16_t) sib1Elem.cellAccessRelatedInfo.cellIdentity);
              break;
            }
          case LteControlMessage::RAR:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              // Elements defined in BuildRarListElement_s in ff-mac-common.h
              Ptr<RarLteControlMessage> rar = DynamicCast<RarLteControlMessage> (msg);

              // serialize rar message
              SerializeRarMessage (rar, payloadDataBuffer, payloadDataBufOffset);

              NI_LOG_DEBUG (this << " - DL: RAR Message sent (Size=" << sizeof(rar) << " bytes) /"
                            " raRNTI=" << (uint16_t) rar->GetRaRnti ());

              break;
            }
          case LteControlMessage::UL_DCI:
            {
              // extract the required ul dci data
              Ptr<UlDciLteControlMessage> uldci = DynamicCast<UlDciLteControlMessage> (msg);
              // insert rnti in map
              if (rntiMap.find(uldci->GetDci().m_rnti)==rntiMap.end())
                {
                  rntiMap[uldci->GetDci().m_rnti] = 1;
                }
              else
                {
                  rntiMap[uldci->GetDci().m_rnti]++;
                }
              break;
            }
          case LteControlMessage::DL_DCI:
            {
              // extract the required dl dci data
              Ptr<DlDciLteControlMessage> dldci = DynamicCast<DlDciLteControlMessage> (msg);
              // insert rnti in map
              if (rntiMap.find(dldci->GetDci().m_rnti)==rntiMap.end())
                {
                  rntiMap[dldci->GetDci().m_rnti] = 1;
                }
              else
                {
                  rntiMap[dldci->GetDci().m_rnti]++;
                }
              break;
            }
          default:
            NI_LOG_FATAL (this << " - DL: Message type not recognized");
        }
        itCtrlMsg++;
      } // end while loop

  }

  bool
  NiLtePhyInterface::NiStartTxDlCtrlFrameUc (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t &controlMessageCnt, uint16_t curRnti)
  {
    // parse the control messages
    std::list<Ptr<LteControlMessage> >::iterator itCtrlMsg;

    // collect all messages which are rnti specific - these will be sent only to specific ue
    itCtrlMsg = ctrlMsgList.begin ();
    while (itCtrlMsg != ctrlMsgList.end ())
      {
        Ptr<LteControlMessage> msg = (*itCtrlMsg);
        LteControlMessage::MessageType m_messageType = msg->GetMessageType ();

        switch (m_messageType) {
          case LteControlMessage::MIB:
            {
              break;
            }
          case LteControlMessage::SIB1:
            {
              break;
            }
          case LteControlMessage::RAR:
            {
              break;
            }
          case LteControlMessage::UL_DCI:
            {
              // extract the required ul dci data
              Ptr<UlDciLteControlMessage> uldci = DynamicCast<UlDciLteControlMessage> (msg);

              if (uldci->GetDci().m_rnti == curRnti){
                  // store message type in pdu
                  std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
                  *payloadDataBufOffset += sizeof(m_messageType);
                  controlMessageCnt++;

                  // get ul dci element
                  UlDciListElement_s ulDciElem = uldci->GetDci ();
                  // store ul dci element in pdu
                  std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&ulDciElem, sizeof(ulDciElem));
                  *payloadDataBufOffset += sizeof(ulDciElem);

                  NI_LOG_DEBUG (this << " - DL: UL DCI Message sent (Size=" << sizeof(ulDciElem) << " bytes) /"
                                << " rnti=" << ulDciElem.m_rnti);
              }

              break;
            }
          case LteControlMessage::DL_DCI:
            {
              // elements defined in DlDciListElement_s in ff-mac-common.h
              Ptr<DlDciLteControlMessage> dldci = DynamicCast<DlDciLteControlMessage> (msg);

              if (dldci->GetDci().m_rnti == curRnti){
                  // store message type in pdu
                  std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
                  *payloadDataBufOffset += sizeof(m_messageType);
                  controlMessageCnt++;

                  // get dl dci element
                  DlDciListElement_s dlDciElem = dldci->GetDci ();
                  // extract required parameters for ni api tx config req message
                  m_rbBitmap = dlDciElem.m_rbBitmap;
                  m_rnti     = dlDciElem.m_rnti;
                  m_mcs      = dlDciElem.m_mcs.at (0);
                  m_tbsSize  = dlDciElem.m_tbsSize.at (0);
                  // convert ns-3 dl dci struct into ni dl dci struct due to use of std::vector
                  NiDlDciListElement_s dlDciElemNi;
                  ConvertToNiDlDciListElement(dlDciElem, &dlDciElemNi);
                  // store dl dci element in pdu
                  std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&dlDciElemNi, sizeof(dlDciElemNi));
                  *payloadDataBufOffset += sizeof(dlDciElemNi);

                  NI_LOG_DEBUG (this << " - DL:"
                                << " Create DL DCI data (" << sizeof(dlDciElemNi) << " bytes) /"
                                << " rbBitmap=" << (std::bitset<32>) m_rbBitmap
                                << " rnti=" << m_rnti
                                << " mcs=" << (uint16_t) m_mcs
                                << " tbsSize=" << (uint16_t) m_tbsSize << " bytes"
                                << " buffer offset=" << *payloadDataBufOffset);

                  // store dl payload packets from packet burst that belong to rnti in pdu
                  NiStartTxDataFrame (packetBurst, payloadDataBuffer, payloadDataBufOffset, m_rnti);

              } else {
                  // create default dci
              }

              break;
            }
          default:
            NI_LOG_FATAL (this << " - DL: Message type not recognized");
        }
        itCtrlMsg++;
      } // end while loop

  }

  bool
  NiLtePhyInterface::NiStartRxDlCtrlFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > &ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset)
  {

    LteControlMessage::MessageType m_messageType;
    // extract message type
    std::memcpy((uint8_t*)&m_messageType, payloadDataBuffer+*payloadDataBufOffset, sizeof(m_messageType));
    *payloadDataBufOffset += sizeof(m_messageType);

    switch (m_messageType){
      case LteControlMessage::MIB:
        {
          // extract mib control element
          LteRrcSap::MasterInformationBlock mibElem;

          std::memcpy(&mibElem, payloadDataBuffer+*payloadDataBufOffset, sizeof(mibElem));
          *payloadDataBufOffset += sizeof(mibElem);

          // recreate the mib control message
          Ptr<MibLteControlMessage> mib = Create<MibLteControlMessage> ();
          mib->SetMib(mibElem);
          Ptr<LteControlMessage> msg = mib;
          ctrlMsgList.push_back( msg );

          // update sfn and tti counters
          if (!m_mibReceived){
              m_mibSfn        = mibElem.systemFrameNumber;
              //m_tti         = 1;
              m_mibReceived = true;
          }

          NI_LOG_DEBUG (this << " - DL: MIB Message received (Size=" << sizeof(mibElem) << " bytes) /"
                        " SFN=" << (uint16_t) mibElem.systemFrameNumber <<
                        " DL-BW=" << (uint16_t) mibElem.dlBandwidth <<
                        " SFN updated=" << m_mibReceived);

          break;
        }
      case LteControlMessage::SIB1:
        {
          // extract sib1 control element
          LteRrcSap::SystemInformationBlockType1 sib1Elem;

          std::memcpy(&sib1Elem, payloadDataBuffer+*payloadDataBufOffset, sizeof(sib1Elem));
          *payloadDataBufOffset += sizeof(sib1Elem);

          // recreate the sib1 control message
          Ptr<Sib1LteControlMessage> sib1 = Create<Sib1LteControlMessage> ();
          sib1->SetSib1(sib1Elem);
          Ptr<LteControlMessage> msg = sib1;
          ctrlMsgList.push_back( msg );

          NI_LOG_DEBUG (this << " - DL: SIB1 Message received (Size=" << sizeof(sib1Elem) << " bytes) /"
                        " Cell ID=" << (uint16_t) sib1Elem.cellAccessRelatedInfo.cellIdentity);

          break;
        }
      case LteControlMessage::RAR:
        {
          // recreate the rar control message
          Ptr<RarLteControlMessage> rar = Create<RarLteControlMessage> ();
          // deserialize rar message and set temporarily the m_rnti - needed for ul packets
          m_rnti = DeserializeRarMessage (rar, payloadDataBuffer, payloadDataBufOffset);
          // put rar message into queue
          Ptr<LteControlMessage> msg = rar;
          ctrlMsgList.push_back( msg );

          NI_LOG_DEBUG (this << " - DL: RAR Message received (Size=" << sizeof(rar) << " bytes) /"
                        " raRNTI=" << (uint16_t) rar->GetRaRnti ());

          break;
        }
      case LteControlMessage::UL_DCI:
        {
          // extract ul dci element
          UlDciListElement_s   ulDciElem;

          std::memcpy(&ulDciElem, payloadDataBuffer+*payloadDataBufOffset, sizeof(ulDciElem));
          *payloadDataBufOffset += sizeof(ulDciElem);

          // recreate the ul dci control message
          Ptr<UlDciLteControlMessage> uldci = Create<UlDciLteControlMessage> ();
          uldci->SetDci(ulDciElem);
          Ptr<LteControlMessage> msg = uldci;
          ctrlMsgList.push_back( msg );

          NI_LOG_DEBUG (this << " - DL: UL DCI data received (" << sizeof(ulDciElem) << " bytes)");

          break;
        }
      case LteControlMessage::DL_DCI:
        {
          // extract dl dci element
          DlDciListElement_s   dlDciElem;
          NiDlDciListElement_s dlDciElemNi;

          std::memcpy(&dlDciElemNi, payloadDataBuffer+*payloadDataBufOffset, sizeof(dlDciElemNi));
          *payloadDataBufOffset += sizeof(dlDciElemNi);

          // convert ni dl dci structure to ns-3 dl dci structure due to use of std::vector
          ConvertFromNiDlDciListElement(dlDciElemNi, &dlDciElem);

          // recreate the dl dci control message
          Ptr<DlDciLteControlMessage> dldci = Create<DlDciLteControlMessage> ();
          dldci->SetDci(dlDciElem);
          Ptr<LteControlMessage> msg = dldci;
          ctrlMsgList.push_back( msg );

          uint32_t m_numPackets;
          std::memcpy(&m_numPackets, payloadDataBuffer+*payloadDataBufOffset, sizeof(m_numPackets));

          NI_LOG_DEBUG (this << " - DL:"
                        << " DL DCI data received (" << sizeof(dlDciElemNi) << " bytes)"
                        << " rbBitmap=" << (std::bitset<32>) dldci->GetDci ().m_rbBitmap
                        << " rnti=" << dldci->GetDci ().m_rnti
                        << " mcs=" << (uint16_t) dldci->GetDci ().m_mcs.at (0)
                        << " tbsSize=" << (uint16_t) dldci->GetDci ().m_tbsSize.at (0) << " bytes"
                        << " buffer offset=" << *payloadDataBufOffset
                        << " packetNum=" << m_numPackets);


          // extract payload data packets from MAC PDU and store in a burst
          NiStartRxDataFrame (packetBurst, payloadDataBuffer, payloadDataBufOffset);

          break;
        }
      default:
        NI_LOG_FATAL (this << " - DL: Message type " << m_messageType << " not recognized");
    } // end switch

  }

  bool
  NiLtePhyInterface::NiStartTxUlCtrlFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t &controlMessageCnt)
  {
    // parse the control messages
    std::list<Ptr<LteControlMessage> >::iterator itCtrlMsg;

    // first collect all broadcast messages which are not rnti specific - these will be sent to all ue's
    itCtrlMsg = ctrlMsgList.begin ();
    while (itCtrlMsg != ctrlMsgList.end ())
      {
        Ptr<LteControlMessage> msg = (*itCtrlMsg);
        LteControlMessage::MessageType m_messageType = msg->GetMessageType ();

        switch (m_messageType) {
          case LteControlMessage::DL_CQI:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              Ptr<DlCqiLteControlMessage> dlcqi = DynamicCast<DlCqiLteControlMessage> (msg);

              // serialize dl cqi message
              SerializeDlCqiMessage (dlcqi, payloadDataBuffer, payloadDataBufOffset);

              NI_LOG_DEBUG (this << " - UL: DL_CQI Message sent (Size=" << sizeof(dlcqi) << " bytes)");
              break;
            }
          case LteControlMessage::BSR:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              Ptr<BsrLteControlMessage> bsr = DynamicCast<BsrLteControlMessage> (msg);
              // get bsr element
              MacCeListElement_s bsrElem = bsr->GetBsr ();

              // convert ns-3 dl dci struct into ni dl dci struct due to use of std::vector
              NiMacCeListElement_s bsrElemNi;
              ConvertToNiMacCeListElement(bsrElem, &bsrElemNi);
              // store dl dci element in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&bsrElemNi, sizeof(bsrElemNi));
              *payloadDataBufOffset += sizeof(bsrElemNi);

              NI_LOG_DEBUG (this << " - UL: BSR Message sent (Size=" << sizeof(bsrElemNi) << " bytes) /"
                            " m_rnti=" << (uint16_t)bsrElemNi.m_rnti  <<
                            " m_phr=" << (uint16_t)bsrElemNi.m_phr <<
                            " m_macCeType=" << (uint16_t)bsrElemNi.m_macCeType <<
                            " m_crnti=" << (uint16_t)bsrElemNi.m_crnti <<
                            " m_bufferStatus_0=" << (uint16_t)bsrElemNi.m_bufferStatus_0);
              break;
            }
          case LteControlMessage::DL_HARQ:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              Ptr<DlHarqFeedbackLteControlMessage> dlharq = DynamicCast<DlHarqFeedbackLteControlMessage> (msg);

              // NOTE: currently no DL HARQ messages are supported in NI API as disabled in main file
              // will be generated in UE in LteUePhy::ReceiveLteDlHarqFeedback called by LteSpectrumPhy::EndRxData
              // and evaluated in LteEnbMac::DoDlInfoListElementHarqFeeback to provide the info to scheduler

              NI_LOG_FATAL (this << " - UL: DL HARQ Message sent (Size=" << sizeof(dlharq) << " bytes) - NOT SUPPORTED BY NI API");

              break;
            }
          case LteControlMessage::RACH_PREAMBLE:
            {
              // store message type in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_messageType, sizeof(m_messageType));
              *payloadDataBufOffset += sizeof(m_messageType);
              controlMessageCnt++;

              Ptr<RachPreambleLteControlMessage> rach = DynamicCast<RachPreambleLteControlMessage> (msg);

              uint32_t m_rapId = rach->GetRapId ();

              // store RACH rapID in pdu
              std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_rapId, sizeof(m_rapId));
              *payloadDataBufOffset += sizeof(m_rapId);

              NI_LOG_DEBUG (this << " - UL: RACH_PREAMBLE Message sent (Size=" << sizeof(rach) << " bytes) /"
                            " m_rapId=" << m_rapId);

              break;
            }
          default:
            NI_LOG_FATAL (this << " - UL: Message type not recognized");
        }
        itCtrlMsg++;
      } // end while loop

  }

  bool
  NiLtePhyInterface::NiStartRxUlCtrlFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > &ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset)
  {
    LteControlMessage::MessageType m_messageType;
    // extract message type
    std::memcpy((uint8_t*)&m_messageType, payloadDataBuffer+*payloadDataBufOffset, sizeof(m_messageType));
    *payloadDataBufOffset += sizeof(m_messageType);

    switch (m_messageType){
      case LteControlMessage::DL_CQI:
        {
          Ptr<DlCqiLteControlMessage> dlcqi = Create<DlCqiLteControlMessage> ();

          // deserialize dl cqi message
          DeserializeDlCqiMessage (dlcqi, payloadDataBuffer, payloadDataBufOffset);

          Ptr<LteControlMessage> msg = dlcqi;
          ctrlMsgList.push_back( msg );

          NI_LOG_DEBUG (this << " - UL: DL_CQI Message received (Size=" << sizeof(dlcqi) << " bytes)");
          break;
        }
      case LteControlMessage::BSR:
        {
          // extract bsr element
          MacCeListElement_s bsrElem;
          NiMacCeListElement_s bsrElemNi;

          std::memcpy(&bsrElemNi, payloadDataBuffer+*payloadDataBufOffset, sizeof(bsrElemNi));
          *payloadDataBufOffset += sizeof(bsrElemNi);

          // convert ni dl dci structure to ns-3 dl dci structure due to use of std::vector
          ConvertFromNiMacCeListElement(bsrElemNi, &bsrElem);

          Ptr<BsrLteControlMessage> bsr = Create<BsrLteControlMessage> ();
          bsr->SetBsr(bsrElem);
          Ptr<LteControlMessage> msg = bsr;
          ctrlMsgList.push_back( msg );

          NI_LOG_DEBUG (this << " - UL: BSR Message received (Size=" << sizeof(bsrElemNi) << " bytes)"
                        " m_rnti=" << (uint16_t)bsrElemNi.m_rnti  <<
                        " m_phr=" << (uint16_t)bsrElemNi.m_phr <<
                        " m_macCeType=" << (uint16_t)bsrElemNi.m_macCeType <<
                        " m_crnti=" << (uint16_t)bsrElemNi.m_crnti <<
                        " m_bufferStatus_0=" << (uint16_t)bsrElemNi.m_bufferStatus_0);

          break;
        }
      case LteControlMessage::DL_HARQ:
        {
          Ptr<DlHarqFeedbackLteControlMessage> dlharq = Create<DlHarqFeedbackLteControlMessage> ();

          // NOTE: currently no DL HARQ messages are supported in NI API as disabled in main file

          //Ptr<LteControlMessage> msg = dlharq;
          //ctrlMsgList.push_back( msg );

          NI_LOG_FATAL (this << " - UL: DL HARQ Message received (Size=" << sizeof(dlharq) << " bytes) - NOT SUPPORTED BY NI API");

          break;
        }
      case LteControlMessage::RACH_PREAMBLE:
        {
          uint32_t m_rapId;

          std::memcpy(&m_rapId, payloadDataBuffer+*payloadDataBufOffset, sizeof(m_rapId));
          *payloadDataBufOffset += sizeof(m_rapId);

          Ptr<RachPreambleLteControlMessage> rach = Create<RachPreambleLteControlMessage> ();
          rach->SetRapId (m_rapId);

          Ptr<LteControlMessage> msg = rach;
          ctrlMsgList.push_back( rach );

          NI_LOG_DEBUG (this << " - UL: RACH_PREAMBLE Message received (Size=" << sizeof(rach) << " bytes) /"
                        " m_rapId=" << m_rapId);

          break;
        }
      default:
        NI_LOG_ERROR (this << " - UL: Message type " << m_messageType << " not recognized");
    } // end switch

  }

  bool
  NiLtePhyInterface::NiStartTxDataFrame (Ptr<PacketBurst> packetBurst, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t curRnti)
  {
    // check if payload data packets are available
    if (packetBurst) {

        LteRadioBearerTag mLteBearerTag;
        Ptr<Packet> packetTmp;

        // create auxiliary rnti specific packet burst
        Ptr<PacketBurst> packetBurstTmp = Create<PacketBurst> ();

        // collect all packets which belong to one rnti
        std::list<Ptr<Packet> >::const_iterator itPacketBurst = packetBurst->Begin ();
        while (itPacketBurst != packetBurst->End ())
          {
            // get next packet from burst
            packetTmp = (*itPacketBurst)->Copy();
            // get rnti from bearer tag
            packetTmp->RemovePacketTag (mLteBearerTag);
            // check if packet belong to requested rnti - if requested rnti is zero transmit all (used for uplink)
            if ((curRnti == mLteBearerTag.GetRnti ())||(curRnti == 0))
              {
                // add packets to rnti specific packet burst
                packetBurstTmp->AddPacket ((*itPacketBurst)->Copy());
              }
            itPacketBurst++;

            //NI_LOG_DEBUG("NiStartTxDataFrame: rnti=" << mLteBearerTag.GetRnti () << " curRnti=" << curRnti);
          }

        // include number of packets in pdu for restoring at the receiver
        uint32_t m_numPackets = packetBurstTmp->GetNPackets();

        std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_numPackets, sizeof(m_numPackets));
        *payloadDataBufOffset += sizeof(m_numPackets);

        // serialize the packet burst of the current rnti
        for (std::list<Ptr<Packet> >::const_iterator itPacketBurstTmp = packetBurstTmp->Begin (); itPacketBurstTmp != packetBurstTmp->End (); ++itPacketBurstTmp)
          {
            // get next packet
            Ptr<Packet> m_currentPacket = (*itPacketBurstTmp);

            NI_LOG_DEBUG(this << " - Serialize packet of size " << m_currentPacket->GetSerializedSize () << " bytes - Payload buffer cnt: " << *payloadDataBufOffset);

            // Append all packet and byte tags as header objects to the packet,
            // because these are NOT serialized
            // keep track of the tag types and the order with a list
            // LteRadioBearerTag == 1, RlcTag == 2, PDCP Tag == 3
            std::vector<uint8_t> tagTypeList;

            // Add the LteRadioBearerTag to the packet
            LteRadioBearerTag radioBearTag;
            m_currentPacket->RemovePacketTag (radioBearTag);
            LteRadioBearerHeader radioBearerHead (radioBearTag.GetRnti (),\
                                                  radioBearTag.GetLcid (),\
                                                  radioBearTag.GetLayer ());
            m_currentPacket->AddHeader (radioBearerHead);
            tagTypeList.push_back (1);
            uint16_t nTags = 1;

            // Add RlcTag, PdcpTag to the packet
            RlcTag rlcTag;
            TypeId rlcTid = rlcTag.GetInstanceTypeId ();
            PdcpTag pdcpTag;
            TypeId pdcpTid = pdcpTag.GetInstanceTypeId ();
            ByteTagIterator itByteTag = m_currentPacket->GetByteTagIterator ();
            while (itByteTag.HasNext ())
              {
                ByteTagIterator::Item item = itByteTag.Next ();
                if (rlcTid == item.GetTypeId ())
                  {
                    item.GetTag (rlcTag);
                    RlcTagHeader rlcTagHead (rlcTag.GetSenderTimestamp ());
                    m_currentPacket->AddHeader (rlcTagHead);
                    nTags++;
                    tagTypeList.push_back (2);
                  }
                if (pdcpTid == item.GetTypeId ())
                  {
                    item.GetTag (pdcpTag);
                    PdcpTagHeader pdcpTagHead (pdcpTag.GetSenderTimestamp ());
                    m_currentPacket->AddHeader (pdcpTagHead);
                    nTags++;
                    tagTypeList.push_back (3);
                  }
              }
            //Add the tag info to the packet
            LtePacketTagInfoHeader numTags (nTags, tagTypeList);
            m_currentPacket->AddHeader (numTags);

            // include packet size for restoring at the receiver
            uint32_t m_PacketSize = m_currentPacket->GetSerializedSize();

            std::memcpy(payloadDataBuffer+*payloadDataBufOffset, (uint8_t*)&m_PacketSize, sizeof(m_PacketSize));
            *payloadDataBufOffset += sizeof(m_PacketSize);

            // include payload packet in pdu
            m_currentPacket->Serialize (payloadDataBuffer+*payloadDataBufOffset, m_PacketSize);
            *payloadDataBufOffset += m_PacketSize;
          }

        m_niApiCountTxPayloadDataPackets++;

        NI_LOG_DEBUG (this << " - Created MAC PDU #" << m_niApiCountTxPayloadDataPackets
                              << " including " << m_numPackets << " packets"
                              << " with total size " << *payloadDataBufOffset << " bytes");

        return 0;

    } else {
        NI_LOG_DEBUG (this << " -  No payload data for current rnti");
        return 0;
    }
  }

  bool
  NiLtePhyInterface::NiStartRxDataFrame (Ptr<PacketBurst> packetBurst, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset)
  {
    //
    LtePacketTagInfoHeader tagTypeListRx;
    std::list<RlcTag> rlcTagList;
    std::list<PdcpTag> pdcpTagList;
    LteRadioBearerHeader radioBearerHeadRx;
    RlcTagHeader rlcTagHeadRx;
    PdcpTagHeader pdcpTagHeadRx;

    // extract number of packets
    uint32_t m_numPackets;
    std::memcpy((uint8_t*)&m_numPackets, payloadDataBuffer+*payloadDataBufOffset, sizeof(m_numPackets));
    *payloadDataBufOffset += sizeof(m_numPackets);

    NI_LOG_DEBUG (this << " - Received MAC PDU including " << m_numPackets << " packets - buffer offset=" << *payloadDataBufOffset);

    for (uint32_t idxPacket = 0; idxPacket < m_numPackets; idxPacket++)
      {
        rlcTagList.clear ();
        pdcpTagList.clear ();

        // extract packet size
        uint32_t m_PacketSize;
        std::memcpy((uint8_t*)&m_PacketSize, payloadDataBuffer+*payloadDataBufOffset, sizeof(m_PacketSize));
        *payloadDataBufOffset += sizeof(m_PacketSize);

        // extract payload packet
        Ptr<Packet> packet = Create<Packet> ((uint8_t const*)(payloadDataBuffer+*payloadDataBufOffset), m_PacketSize, true);
        *payloadDataBufOffset += m_PacketSize;

        packet->RemoveHeader (tagTypeListRx);

        // extract header object containing original packet and byte tags in
        // the opposite order
        for (int16_t i=tagTypeListRx.GetNumOfTagHeaders()-1; i>=0; i--)
          {
            switch (tagTypeListRx.GetHeaderTypeList ()[i]) {
              case 1: //Packet Tag
                {
                  packet->RemoveHeader (radioBearerHeadRx);
                  LteRadioBearerTag tagRx (radioBearerHeadRx.GetRnti (), radioBearerHeadRx.GetLcid (), radioBearerHeadRx.GetLayer ());
                  packet->AddPacketTag (tagRx);
                  break;
                }
              case 2: //RLC Tag
                {
                  packet->RemoveHeader (rlcTagHeadRx);
                  RlcTag rlcTag (rlcTagHeadRx.GetSenderTimestamp ());
                  rlcTagList.push_front (rlcTag);
                  break;
                }
              case 3: //PDCP Tag
                {
                  packet->RemoveHeader (pdcpTagHeadRx);
                  PdcpTag pdcpTag (pdcpTagHeadRx.GetSenderTimestamp ());
                  pdcpTagList.push_front (pdcpTag);
                  break;
                }
            }
          }

        // Add the original Bytes Tags back to the packet
        for (uint16_t i = 0; i<tagTypeListRx.GetNumOfTagHeaders(); i++)
          {
            switch (tagTypeListRx.GetHeaderTypeList ()[i]) {
              case 2: //RLC Tag
                {
                  packet->AddByteTag (rlcTagList.front ());
                  rlcTagList.pop_front ();
                  break;
                }
              case 3: //PDCP Tag
                {
                  packet->AddByteTag (pdcpTagList.front ());
                  pdcpTagList.pop_front ();
                  break;
                }
            }
          }

        NI_LOG_DEBUG(this << " - Add packet #" << idxPacket << " of size " << packet->GetSerializedSize () << " bytes to packet burst - buffer offset=" << *payloadDataBufOffset);

        // add packet to temp burst
        packetBurst->AddPacket(packet);
      }
  }

  void
  NiLtePhyInterface::ConvertToNiDlDciListElement (DlDciListElement_s dlDciElem, NiDlDciListElement_s* dlDciElemNi)
  {
    dlDciElemNi->m_rnti             = dlDciElem.m_rnti;
    dlDciElemNi->m_rbBitmap         = dlDciElem.m_rbBitmap;
    dlDciElemNi->m_rbShift          = dlDciElem.m_rbShift;
    dlDciElemNi->m_resAlloc         = dlDciElem.m_resAlloc;
    // NOTE: restriction to one transport block
    dlDciElemNi->m_tbsSize          = dlDciElem.m_tbsSize.at(0);
    dlDciElemNi->m_mcs              = dlDciElem.m_mcs.at(0);
    dlDciElemNi->m_ndi              = dlDciElem.m_ndi.at(0);
    dlDciElemNi->m_rv               = dlDciElem.m_rv.at(0);
    dlDciElemNi->m_cceIndex         = dlDciElem.m_cceIndex;
    dlDciElemNi->m_aggrLevel        = dlDciElem.m_aggrLevel;
    dlDciElemNi->m_precodingInfo    = dlDciElem.m_precodingInfo;
    dlDciElemNi->m_format           = (NiDlDciListElement_s::Format_e) dlDciElem.m_format;
    dlDciElemNi->m_tpc              = dlDciElem.m_tpc;
    dlDciElemNi->m_harqProcess      = dlDciElem.m_harqProcess;
    dlDciElemNi->m_dai              = dlDciElem.m_dai;
    dlDciElemNi->m_vrbFormat        = (NiDlDciListElement_s::VrbFormat_e) dlDciElem.m_vrbFormat;
    dlDciElemNi->m_tbSwap           = dlDciElem.m_tbSwap;
    dlDciElemNi->m_spsRelease       = dlDciElem.m_spsRelease;
    dlDciElemNi->m_pdcchOrder       = dlDciElem.m_pdcchOrder;
    dlDciElemNi->m_preambleIndex    = dlDciElem.m_preambleIndex;
    dlDciElemNi->m_prachMaskIndex   = dlDciElem.m_prachMaskIndex;
    dlDciElemNi->m_nGap             = (NiDlDciListElement_s::Ngap_e) dlDciElem.m_nGap;
    dlDciElemNi->m_tbsIdx           = dlDciElem.m_tbsIdx;
    dlDciElemNi->m_dlPowerOffset    = dlDciElem.m_dlPowerOffset;
    dlDciElemNi->m_pdcchPowerOffset = dlDciElem.m_pdcchPowerOffset;
  }

  void
  NiLtePhyInterface::ConvertFromNiDlDciListElement (NiDlDciListElement_s dlDciElemNi, DlDciListElement_s* dlDciElem)
  {
    dlDciElem->m_rnti             = dlDciElemNi.m_rnti;
    dlDciElem->m_rbBitmap         = dlDciElemNi.m_rbBitmap;
    dlDciElem->m_rbShift          = dlDciElemNi.m_rbShift;
    dlDciElem->m_resAlloc         = dlDciElemNi.m_resAlloc;
    dlDciElem->m_tbsSize.push_back(dlDciElemNi.m_tbsSize);
    dlDciElem->m_mcs.push_back(dlDciElemNi.m_mcs);
    dlDciElem->m_ndi.push_back(dlDciElemNi.m_ndi);
    dlDciElem->m_rv.push_back(dlDciElemNi.m_rv);
    dlDciElem->m_cceIndex         = dlDciElemNi.m_cceIndex;
    dlDciElem->m_aggrLevel        = dlDciElemNi.m_aggrLevel;
    dlDciElem->m_precodingInfo    = dlDciElemNi.m_precodingInfo;
    dlDciElem->m_format           = (DlDciListElement_s::Format_e) dlDciElemNi.m_format;
    dlDciElem->m_tpc              = dlDciElemNi.m_tpc;
    dlDciElem->m_harqProcess      = dlDciElemNi.m_harqProcess;
    dlDciElem->m_dai              = dlDciElemNi.m_dai;
    dlDciElem->m_vrbFormat        = (DlDciListElement_s::VrbFormat_e) dlDciElemNi.m_vrbFormat;
    dlDciElem->m_tbSwap           = dlDciElemNi.m_tbSwap;
    dlDciElem->m_spsRelease       = dlDciElemNi.m_spsRelease;
    dlDciElem->m_pdcchOrder       = dlDciElemNi.m_pdcchOrder;
    dlDciElem->m_preambleIndex    = dlDciElemNi.m_preambleIndex;
    dlDciElem->m_prachMaskIndex   = dlDciElemNi.m_prachMaskIndex;
    dlDciElem->m_nGap             = (DlDciListElement_s::Ngap_e) dlDciElemNi.m_nGap;
    dlDciElem->m_tbsIdx           = dlDciElemNi.m_tbsIdx;
    dlDciElem->m_dlPowerOffset    = dlDciElemNi.m_dlPowerOffset;
    dlDciElem->m_pdcchPowerOffset = dlDciElemNi.m_pdcchPowerOffset;
  }

  void
  NiLtePhyInterface::ConvertToNiMacCeListElement (MacCeListElement_s BsrElem, NiMacCeListElement_s* BsrElemNi)
  {
    BsrElemNi->m_rnti            = BsrElem.m_rnti;
    BsrElemNi->m_macCeType       = (NiMacCeListElement_s::MacCeType_e) BsrElem.m_macCeType;
    BsrElemNi->m_phr             = BsrElem.m_macCeValue.m_phr;
    BsrElemNi->m_crnti           = BsrElem.m_macCeValue.m_crnti;
    BsrElemNi->m_bufferStatus_0  = BsrElem.m_macCeValue.m_bufferStatus.at(0);
    BsrElemNi->m_bufferStatus_1  = BsrElem.m_macCeValue.m_bufferStatus.at(1);
    BsrElemNi->m_bufferStatus_2  = BsrElem.m_macCeValue.m_bufferStatus.at(2);
    BsrElemNi->m_bufferStatus_3  = BsrElem.m_macCeValue.m_bufferStatus.at(3);
  }

  void
  NiLtePhyInterface::ConvertFromNiMacCeListElement (NiMacCeListElement_s BsrElemNi, MacCeListElement_s* BsrElem)
  {
    BsrElem->m_rnti               = BsrElemNi.m_rnti;
    BsrElem->m_macCeType          = (MacCeListElement_s::MacCeType_e) BsrElemNi.m_macCeType;
    BsrElem->m_macCeValue.m_phr   = BsrElemNi.m_phr;
    BsrElem->m_macCeValue.m_crnti = BsrElemNi.m_crnti;
    BsrElem->m_macCeValue.m_bufferStatus.push_back(BsrElemNi.m_bufferStatus_0);
    BsrElem->m_macCeValue.m_bufferStatus.push_back(BsrElemNi.m_bufferStatus_1);
    BsrElem->m_macCeValue.m_bufferStatus.push_back(BsrElemNi.m_bufferStatus_2);
    BsrElem->m_macCeValue.m_bufferStatus.push_back(BsrElemNi.m_bufferStatus_3);
  }

  void
  NiLtePhyInterface::SerializeRarMessage (Ptr<RarLteControlMessage> rar , uint8_t* buffer, uint32_t* bufferOffset)
  {
    // serialize rar message
    uint8_t rarMsgCnt=0;
    for (std::list<RarLteControlMessage::Rar>::const_iterator it = rar->RarListBegin (); it != rar->RarListEnd ();++it){
        rarMsgCnt++;
    }
    // store rar rnti in pdu
    uint16_t raRnti = rar->GetRaRnti ();
    std::memcpy(buffer+*bufferOffset, (uint8_t*)&raRnti, sizeof(raRnti));
    *bufferOffset += sizeof(raRnti);
    // store number of messages in pdu
    std::memcpy(buffer+*bufferOffset, (uint8_t*)&rarMsgCnt, sizeof(rarMsgCnt));
    *bufferOffset += sizeof(rarMsgCnt);
    for (std::list<RarLteControlMessage::Rar>::const_iterator it = rar->RarListBegin (); it != rar->RarListEnd ();++it){
        uint8_t rapId                    = it->rapId;
        BuildRarListElement_s rarPayload = it->rarPayload;
        uint16_t m_rnti                  = rarPayload.m_rnti;
        UlGrant_s m_grant                = rarPayload.m_grant;
        // store rapId in pdu
        std::memcpy(buffer+*bufferOffset, (uint8_t*)&rapId, sizeof(rapId));
        *bufferOffset += sizeof(rapId);
        // store m_rnti in pdu
        std::memcpy(buffer+*bufferOffset, (uint8_t*)&m_rnti, sizeof(m_rnti));
        *bufferOffset += sizeof(m_rnti);
        // store m_rnti in pdu
        std::memcpy(buffer+*bufferOffset, (uint8_t*)&m_grant, sizeof(m_grant));
        *bufferOffset += sizeof(m_grant);
        // note that the dl dci element that is part of the rar is not used and transferred here

        /*NI_LOG_DEBUG("Serialize RAR Message: "
            << " raRnti=" << (uint16_t)raRnti
            << " rarMsgCnt=" << (uint16_t)rarMsgCnt
            << " rapId=" << (uint16_t)rapId
            << " m_rnti=" << (uint16_t)m_rnti);*/
    }
  }

  uint16_t
  NiLtePhyInterface::DeserializeRarMessage (Ptr<RarLteControlMessage> rarMsg , uint8_t* buffer, uint32_t* bufferOffset)
  {
    // extract and set rar control element
    uint16_t raRnti;
    std::memcpy(&raRnti, buffer+*bufferOffset, sizeof(raRnti));
    *bufferOffset += sizeof(raRnti);
    rarMsg->SetRaRnti (raRnti);
    // extract number of messages
    uint8_t rarMsgCnt=0;
    std::memcpy(&rarMsgCnt, buffer+*bufferOffset, sizeof(rarMsgCnt));
    *bufferOffset += sizeof(rarMsgCnt);

    // extract rar messages
    uint8_t   rapId;
    uint16_t  m_rnti;
    UlGrant_s m_grant;
    for (uint32_t cntMessage = 0; cntMessage < rarMsgCnt ; cntMessage  ++){

        std::memcpy(&rapId, buffer+*bufferOffset, sizeof(rapId));
        *bufferOffset += sizeof(rapId);

        std::memcpy(&m_rnti, buffer+*bufferOffset, sizeof(m_rnti));
        *bufferOffset += sizeof(m_rnti);

        std::memcpy(&m_grant, buffer+*bufferOffset, sizeof(m_grant));
        *bufferOffset += sizeof(m_grant);

        // create new rar list element
        BuildRarListElement_s rarListElem;
        rarListElem.m_rnti  = m_rnti;
        rarListElem.m_grant = m_grant;
        RarLteControlMessage::Rar rar;
        rar.rapId      = rapId;
        rar.rarPayload = rarListElem;
        rarMsg->AddRar (rar);

        /*NI_LOG_DEBUG("Deserialized RAR Message: "
            << "raRnti=" << (uint16_t)raRnti
            << " / rarMsgCnt=" << (uint16_t)rarMsgCnt
            << " / rapId=" << (uint16_t)rapId << " / m_rnti="
            << (uint16_t)m_rnti);*/
    }

    return m_rnti;
  }

  void
  NiLtePhyInterface::SerializeDlCqiMessage (Ptr<DlCqiLteControlMessage> dlCqiMsg , uint8_t* buffer, uint32_t* bufferOffset)
  {
    CqiListElement_s dlCqiElem = dlCqiMsg->GetDlCqi ();

    // serialize m_rnti
    std::memcpy(buffer+*bufferOffset, (uint16_t*)&dlCqiElem.m_rnti, sizeof(dlCqiElem.m_rnti));
    *bufferOffset += sizeof(dlCqiElem.m_rnti);
    // serialize m_ri
    std::memcpy(buffer+*bufferOffset, (uint8_t*)&dlCqiElem.m_ri, sizeof(dlCqiElem.m_ri));
    *bufferOffset += sizeof(dlCqiElem.m_ri);
    // serialize m_cqiType
    std::memcpy(buffer+*bufferOffset, &dlCqiElem.m_cqiType, sizeof(dlCqiElem.m_cqiType));
    *bufferOffset += sizeof(dlCqiElem.m_cqiType);
    // serialize number of wbCqi elements
    uint8_t wbCqiNum = dlCqiElem.m_wbCqi.size();
    std::memcpy(buffer+*bufferOffset, (uint8_t*)&wbCqiNum, sizeof(wbCqiNum));
    *bufferOffset += sizeof(wbCqiNum);
    if (wbCqiNum>0){
        // serialize m_wbCqi -> simplification: only one layer supported
        std::memcpy(buffer+*bufferOffset, (uint8_t*)&dlCqiElem.m_wbCqi.at(0), sizeof(dlCqiElem.m_wbCqi.at(0)));
        *bufferOffset += sizeof(dlCqiElem.m_wbCqi.at(0));
    }
    // serialize number of sbCqi elements
    // NOTE: the following elements are not transmitted as there are not used:
    // m_wbPmi, m_ueSelected, m_bwPart, m_sbList, m_sbPmi
    uint8_t sbCqiNum = dlCqiElem.m_sbMeasResult.m_higherLayerSelected.size();
    std::memcpy(buffer+*bufferOffset, (uint8_t*)&sbCqiNum, sizeof(sbCqiNum));
    *bufferOffset += sizeof(sbCqiNum);
    if (sbCqiNum>0){
        // serialize m_higherLayerSelected vector
        for (uint32_t cntCqi = 0; cntCqi < sbCqiNum ; cntCqi  ++){
            // serialize m_sbCqi -> simplification: only one layer supported
            uint8_t sbCqi = dlCqiElem.m_sbMeasResult.m_higherLayerSelected.at(cntCqi).m_sbCqi.at(0);
            std::memcpy(buffer+*bufferOffset, (uint8_t*)&sbCqi, sizeof(sbCqi));
            *bufferOffset += sizeof(sbCqi);
        }
    }

    /*NI_LOG_DEBUG("Serialize Dl CQI Message: "
        << " m_rnti=" << (uint16_t)dlCqiElem.m_rnti
        << " sbCqiNum=" << (uint16_t)sbCqiNum
        << " wbCqiNum=" << (uint16_t)wbCqiNum);*/
  }

  void
  NiLtePhyInterface::DeserializeDlCqiMessage (Ptr<DlCqiLteControlMessage> dlCqiMsg , uint8_t* buffer, uint32_t* bufferOffset)
  {
    CqiListElement_s dlCqiElem;

    // deserialize m_rnti
    std::memcpy(&dlCqiElem.m_rnti, buffer+*bufferOffset, sizeof(dlCqiElem.m_rnti));
    *bufferOffset += sizeof(dlCqiElem.m_rnti);
    // serialize m_ri
    std::memcpy(&dlCqiElem.m_ri, buffer+*bufferOffset, sizeof(dlCqiElem.m_ri));
    *bufferOffset += sizeof(dlCqiElem.m_ri);
    // serialize m_cqiType
    std::memcpy(&dlCqiElem.m_cqiType, buffer+*bufferOffset, sizeof(dlCqiElem.m_cqiType));
    *bufferOffset += sizeof(dlCqiElem.m_cqiType);
    // deserialize number of wbCqi elements
    uint8_t wbCqiNum;
    std::memcpy((uint8_t*)&wbCqiNum, buffer+*bufferOffset, sizeof(wbCqiNum));
    *bufferOffset += sizeof(wbCqiNum);
    if (wbCqiNum>0){
        // serialize m_wbCqi -> simplification: only one layer supported
        uint8_t wbCqi;
        std::memcpy(&wbCqi,buffer+*bufferOffset, sizeof(wbCqi));
        *bufferOffset += sizeof(wbCqi);
        dlCqiElem.m_wbCqi.push_back(wbCqi);
    }
    // serialize number of cqi elements
    uint8_t sbCqiNum;
    std::memcpy((uint8_t*)&sbCqiNum, buffer+*bufferOffset, sizeof(sbCqiNum));
    *bufferOffset += sizeof(sbCqiNum);
    if (sbCqiNum>0){
        // deserialize m_higherLayerSelected vector
        for (uint32_t cntCqi = 0; cntCqi < sbCqiNum ; cntCqi  ++){
            HigherLayerSelected_s hlCqi;
            // deserialize m_sbCqi -> simplification: only one layer supported
            uint8_t sbCqi;
            std::memcpy(buffer+*bufferOffset, (uint8_t*)&sbCqi, sizeof(sbCqi));
            *bufferOffset += sizeof(sbCqi);
            hlCqi.m_sbCqi.push_back(sbCqi);
            dlCqiElem.m_sbMeasResult.m_higherLayerSelected.push_back(hlCqi);
        }
    }

    dlCqiMsg->SetDlCqi (dlCqiElem);

    /*NI_LOG_DEBUG("Deserialize Dl CQI Message: "
            << " m_rnti=" << (uint16_t)dlCqiElem.m_rnti
            << " sbCqiNum=" << (uint16_t)sbCqiNum
            << " wbCqiNum=" << (uint16_t)wbCqiNum);*/
  }

  void
  NiLtePhyInterface::NiStartSubframe (
                                      uint32_t nrFrames,       // current system frame number (SFN)
                                      uint32_t nrSubFrames,    // current subframe number (TTI)
                                      int64_t ns3TtiTimingUs,  // NS3 TTI duration in micro seconds
                                      uint64_t ueToEnbSfoffset // inital subframe offset between UE and eNB determined by MIB reception, for eNB this value is always 0
                                     )
  {
    const int defaultTtiDuration = NI_LTE_CONST_TTI_DURATION_US;
    const int errorTtiDuration   = 666; // defaultTtiTiming*2/3
    const int warningTtiDuration = 333; // defaultTtiTiming*1/3

  NI_LOG_DEBUG("NiLtePhyInterface::NiStartSubframe, " << nrFrames << "." << nrSubFrames);

    if (defaultTtiDuration != (uint32_t) ns3TtiTimingUs)
      {
        NI_LOG_FATAL("NS3 TTI duration (" << ns3TtiTimingUs <<") not equal to: " << defaultTtiDuration << "(us)");
      }

    // tracing used for performance measurements
    g_logTraceStartSubframeTime = NiUtils::GetSysTime();
    NI_LOG_TRACE("[Trace#0],StartSubFrameStart," << (NiUtils::GetSysTime()-g_logTraceStartSubframeTime));

    // get actual system time
    const uint64_t systemTimeUs = NiUtils::GetSysTime();

    // wait for PhyTimingInd to ensure PHY is sync with simulator
    const uint64_t timingIndTimeUs = WaitForPhyTimingInd();

    // calculate difference to PHY timing indication
    //  diff positive -> PHY timing before NS3 Simulator
    //  diff negative -> NS3 timing before PHY timing
    int64_t diffUs = (int64_t)systemTimeUs - (int64_t)timingIndTimeUs;
    const int64_t absDiffUs = llabs(diffUs);

    // error handling in case of high timing diff
    if (m_firstPhyTimingInd)
      {
        // ignore errors on first run
        NI_LOG_DEBUG ("first iteration");
      }
    else
      {
        if (absDiffUs > warningTtiDuration)
          {
            NI_LOG_WARN("PHY timing extended TTI*1/3 limit with " + std::to_string(diffUs) + "us");
          }
        else if (absDiffUs > errorTtiDuration)
          {
            NI_LOG_ERROR("PHY timing extended TTI*2/3 limit with " + std::to_string(diffUs) + "us");
          }
        else if (absDiffUs > defaultTtiDuration) // FATAL ERROR if above 1ms
          {
            NI_LOG_FATAL("PHY timing extended TTI limit with " + std::to_string(diffUs) + "us");
            diffUs %= defaultTtiDuration;
          }
      }

    // calculate timing difference between simulator start subframe and PHY timing indication
    m_niLteSdrTimingSync->CalcPhyTimeDiff(diffUs);

    // get NI FPGA PHY timing based on PHY timing indication
    const uint16_t timingIndSfn = m_niPipeTransport->GetTimingIndSfn();
    const uint8_t timingIndTti = m_niPipeTransport->GetTimingIndTti();

    // calc and track the TTI/SFN offset (m_sfnSfOffset) between PHY and Simulator timing
    m_niLteSdrTimingSync->ReCalcSfnSfOffset(m_firstPhyTimingInd, nrFrames, nrSubFrames,
                                            timingIndSfn, timingIndTti,
                                            ueToEnbSfoffset, m_sfnSfOffset);

    NI_LOG_TRACE("[Trace#1],SFN,TTI,diffNs3ToPhyTimingInd,timingIndTimeUs," <<
                 "systemTimeUs,simTimeUs,g_alignmentOffset,g_globalTimingdiffNano," <<
                 timingIndSfn << "," << std::to_string(timingIndTti) << "," << diffUs << "," << timingIndTimeUs << "," <<
                 systemTimeUs << "," << Simulator::Now().GetMicroSeconds() << "," << m_niLteSdrTimingSync->GetAlignmentOffset() << "," << m_niLteSdrTimingSync->GetGlobalTimingdiffNano());

    // store value for evaluation in next iteration
    m_lastTimingIndTimeUs = timingIndTimeUs;

    // Handle 5G API Rx configuration
    if(m_ApiLte5Genabled){
        if (m_firstPhyTimingInd){ 
          // Condition to send initial Rx message for UE.
          if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE)){
              NiStartDlRxApiSend();
          }
          //Condition to only initial RX configuration for eNB.
          else if (((m_niApiDevType==NIAPI_ENB)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_ENB)){
              NiStartUlRxApiSend();
          }
        }
      //Change value of the SCS in mid operation. Can add more to make more changes.
      //NI_LTE_5G_CONST_MAX_NUM_SCS_CFG
      // m_ScsSwitchTargetSfn
      if (nrFrames==m_ScsSwitchTargetSfn && nrSubFrames==1){
        NiApiScs_t oldDlScs = m_dlscs;
        NiApiScs_t oldUlScs = m_ulscs;
        m_dlscs = (NiApiScs_t)(((uint32_t) m_dlscs+1) % NI_LTE_5G_CONST_MAX_NUM_SCS_CFG);
        m_ulscs = (NiApiScs_t)(((uint32_t) m_ulscs+1) % NI_LTE_5G_CONST_MAX_NUM_SCS_CFG);
        NI_LOG_CONSOLE_INFO ("5G SCS Configuration for DL changed from " << oldDlScs << " to " << m_dlscs);
        NI_LOG_CONSOLE_INFO ("5G SCS Configuration for UL changed from " << oldUlScs << " to " << m_ulscs);
        if (((m_niApiDevType==NIAPI_UE)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_UE)){
          NiStartDlRxApiSend();
        }
        else if (((m_niApiDevType==NIAPI_ENB)||(m_niApiDevType==NIAPI_ALL))&&(m_ns3DevType==NS3_ENB)){
          NiStartUlRxApiSend();
        }
      }
      //Additional cases need to be implemented here as additional if cases.
    }
    //End of SCS Changing functions.

    UpdateNiChannelSinrValueFromRemoteControl();
  }

  uint64_t
  NiLtePhyInterface::WaitForPhyTimingInd()
  {
    uint64_t timingIndTimeUs = 0;
    const struct timespec ts = {0, 1000L}; // 1us
    uint32_t numWaitIterations = 0;
    uint64_t systemTimeUs,waitTimeUs;
    bool firstTimingInd = false;


    if (m_niPipeTransport->GetTimingIndReceived() == false)
      {
        NI_LOG_CONSOLE_INFO ("    Waiting for first LTE PHY timing indication...");
        NI_LOG_DEBUG ("Waiting for first timing indication ...");
        firstTimingInd = true;
      }

    systemTimeUs = NiUtils::GetSysTime();

    // in case timing indication was not received, wait until reception
    while ((m_niPipeTransport->GetTimingIndReceived() == false) ||
           (m_niPipeTransport->GetTimingIndTime() == m_lastTimingIndTimeUs))
      {
        nanosleep( &ts, NULL );
        numWaitIterations++;
      }
    timingIndTimeUs = m_niPipeTransport->GetTimingIndTime();
    waitTimeUs = NiUtils::GetSysTime() - systemTimeUs;
    if (numWaitIterations > 0)
      {
        NI_LOG_DEBUG ("Waited " + std::to_string(numWaitIterations) + " iterations for timing indication, waitTimeUs: " + std::to_string(waitTimeUs));
      }
    if (firstTimingInd)
      {
        NI_LOG_CONSOLE_INFO ("    ...done!" << std::endl << "[~] NS-3 Simulator executing...");
      }
    return timingIndTimeUs;
  }

  void
  NiLtePhyInterface::UpdateNiChannelSinrValueFromRemoteControl(void)
  {
    // update channel SINR value only in case manual channel SINR setting via remote control engine is enabled
    // in oder to not interfer with PHY_CELL_MEASUREMENT_IND
    if (g_RemoteControlEngine.GetPdb()->getParameterManualLteUeChannelSinrEnable() == true)
      {
        //read out values from remote control database and overwrite local sinr value
        if (GetNiChannelSinrValue() != g_RemoteControlEngine.GetPdb()->getParameterLteUeChannelSinr())
          {
            double sinr = g_RemoteControlEngine.GetPdb()->getParameterLteUeChannelSinr();
            SetNiChannelSinrValue(sinr);
            NI_LOG_CONSOLE_DEBUG("NI.RC:LteUeChannelSinr value changed! SINR value is : " << sinr);
          }
      }
  }
} /* namespace ns3 */
