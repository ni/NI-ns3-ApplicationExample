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


#ifndef NI_LTE_PHY_INTERFACE_H_
#define NI_LTE_PHY_INTERFACE_H_


#include <ns3/object.h>
#include <ns3/packet-burst.h>

#include <ns3/lte-control-messages.h>
#include <ns3/ff-mac-common.h>
#include <ns3/lte-common.h>
#include <ns3/spectrum-value.h>
#include <ns3/lte-spectrum-phy.h>

#include "ns3/ni.h"

namespace ns3
{

  // device identifier
  typedef enum {
    NS3_ENB  = 0,
    NS3_UE = 1,
	NS3_NOAPI = 3,   //DALI: used to disallow NI API installation/operation in DALI fake nodes
  } Ns3LteDevType_t;

  // device identifier
  typedef enum {
    NIAPI_ENB  = 0,
    NIAPI_UE = 1,
    NIAPI_ALL = 2,
    NIAPI_NONE = 3,
  } NiApiDevType_t;

  // packet identifier
  typedef enum {
    NIAPI_DL_PACKET  = 0,
    NIAPI_UL_PACKET  = 1,
    NIAPI_UNDEF_PACKET = 2, //undefined packet type
  } NiApiPacketType_t ;

  typedef Callback< void, uint16_t, uint8_t, bool > NiPhyTimingIndEndOkCallback;
  typedef Callback< void, Ptr<Packet> > NiPhyRxDataEndOkCallback;
  typedef Callback< void, std::list<Ptr<LteControlMessage> > > NiPhyRxCtrlEndOkCallback;
  typedef Callback< void, const SpectrumValue& > NiPhyRxCqiReportCallback;
  typedef Callback< Ptr<LteSpectrumPhy> > NiPhySpectrumModelCallback;

  struct NiApiPacketHeader {
    uint8_t  niApiPacketType;
    uint32_t nrFrames;
    uint8_t  nrSubFrames;
    uint32_t cellId;
    uint16_t rnti;
    uint8_t  numCtrlMsg;
    uint8_t  numPaylMsg;
  };

  struct NiDlDciListElement_s
  {
    uint16_t  m_rnti;
    uint32_t  m_rbBitmap;
    uint8_t   m_rbShift;
    uint8_t   m_resAlloc;
    // NOTE: restriction to one transport block
    uint16_t  m_tbsSize;
    uint8_t   m_mcs;
    uint8_t   m_ndi;
    uint8_t   m_rv;
    uint8_t   m_cceIndex;
    uint8_t   m_aggrLevel;
    uint8_t   m_precodingInfo;
    enum Format_e
    {
      ONE, ONE_A, ONE_B, ONE_C, ONE_D, TWO, TWO_A, TWO_B
    } m_format;
    uint8_t   m_tpc;
    uint8_t   m_harqProcess;
    uint8_t   m_dai;
    enum VrbFormat_e
    {
      VRB_DISTRIBUTED,
      VRB_LOCALIZED
    } m_vrbFormat;
    bool      m_tbSwap;
    bool      m_spsRelease;
    bool      m_pdcchOrder;
    uint8_t   m_preambleIndex;
    uint8_t   m_prachMaskIndex;
    enum Ngap_e
    {
      GAP1, GAP2
    } m_nGap;
    uint8_t   m_tbsIdx;
    uint8_t   m_dlPowerOffset;
    uint8_t   m_pdcchPowerOffset;
  };

  struct NiMacCeListElement_s
  {
    uint16_t  m_rnti;
    enum MacCeType_e
    {
      BSR, PHR, CRNTI
    } m_macCeType;
    uint8_t   m_phr;
    uint8_t   m_crnti;
    // NOTE: always four - see LteUeMac::SendReportBufferStatus
    uint8_t   m_bufferStatus_0;
    uint8_t   m_bufferStatus_1;
    uint8_t   m_bufferStatus_2;
    uint8_t   m_bufferStatus_3;
  };

  class NiLtePhyInterface : public Object
  {

  public:

    NiLtePhyInterface ();

    NiLtePhyInterface (Ns3LteDevType_t ns3DevType);

    virtual ~NiLtePhyInterface ();

    virtual void DoDispose (void);
    virtual void DoInitialize (void);

    static TypeId GetTypeId (void);

    void SetNiPhyTimingIndEndOkCallback (NiPhyTimingIndEndOkCallback c);
    void SetNiPhyRxDataEndOkCallback (NiPhyRxDataEndOkCallback c);
    void SetNiPhyRxCtrlEndOkCallback (NiPhyRxCtrlEndOkCallback c);
    void SetNiPhyRxCqiReportCallback (NiPhyRxCqiReportCallback c);
    void SetNiPhySpectrumModelCallback (NiPhySpectrumModelCallback c);

    NiApiDevType_t GetNiApiDevType () const;
    Ns3LteDevType_t GetNs3DevType () const;
    bool GetNiApiEnable () const;
    bool GetNiApiLoopbackEnable () const;
    double GetNiChannelSinrValue () const;
    void SetNiChannelSinrValue (double chSinrDb);
    void UpdateNiChannelSinrValueFromRemoteControl(void);

    uint64_t NiSfnTtiCounterSync(uint32_t* m_nrFrames, uint32_t* m_nrSubFrames);

    bool NiStartTxCtrlDataFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint32_t m_nrFrames, uint32_t m_nrSubFrames);

    void NiStartSubframe (uint32_t nrFrames, uint32_t nrSubFrames, int64_t ns3TtiTimingUs, uint64_t ueToEnbSfoffset);

  private:

    void SetNs3DevType (std::string type);
    void SetNiApiDevType (std::string type);
    void SetNiApiEnable (bool enable);
    void SetNiApiLoopbackEnable (bool enable);

    void InitializeNiUdpTransport();
    void DeInitializeNiUdpTransport();
    void InitializeNiPipeTransport();
    void DeInitializeNiPipeTransport();
    void InitializeNiLteSdrTimingSync();
    void DeInitializeNiLteSdrTimingSync();

    void NiGenerateCqiReport ();

    bool NiStartTxDlCtrlFrameBc (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t &controlMessageCnt, std::map <uint16_t, uint16_t> &rntiMap);
    bool NiStartTxDlCtrlFrameUc (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t &controlMessageCnt, uint16_t curRnti);
    bool NiStartTxUlCtrlFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t &controlMessageCnt);
    bool NiStartTxDataFrame (Ptr<PacketBurst> packetBurst, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset, uint32_t curRnti);
    bool NiStartTxApiSend (uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset);

    bool NiStartRxCtrlDataFrame (uint8_t* payloadDataBuffer);
    bool NiStartRxCellMeasurementIndHandler (PhyCellMeasInd phyCellMeasInd);
    bool NiStartTimingIndHandler (uint16_t sfn, uint8_t tti, bool firstRun);
    bool NiStartRxDlCtrlFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > &ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset);
    bool NiStartRxUlCtrlFrame (Ptr<PacketBurst> packetBurst, std::list<Ptr<LteControlMessage> > &ctrlMsgList, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset);
    bool NiStartRxDataFrame (Ptr<PacketBurst> packetBurst, uint8_t* payloadDataBuffer, uint32_t* payloadDataBufOffset);

    void ConvertToNiDlDciListElement (DlDciListElement_s dlDciElem, NiDlDciListElement_s* dlDciElemNi);
    void ConvertFromNiDlDciListElement (NiDlDciListElement_s dlDciElemNi, DlDciListElement_s* dlDciElem);
    void ConvertToNiMacCeListElement (MacCeListElement_s BsrElem, NiMacCeListElement_s* BsrElemNi);
    void ConvertFromNiMacCeListElement (NiMacCeListElement_s BsrElemNi, MacCeListElement_s* BsrElem);
    void SerializeRarMessage (Ptr<RarLteControlMessage> rar , uint8_t* buffer, uint32_t* bufferOffset);
    uint16_t DeserializeRarMessage (Ptr<RarLteControlMessage> rar , uint8_t* buffer, uint32_t* bufferOffset);
    void SerializeDlCqiMessage (Ptr<DlCqiLteControlMessage> dlCqiMsg , uint8_t* buffer, uint32_t* bufferOffset);
    void DeserializeDlCqiMessage (Ptr<DlCqiLteControlMessage> dlCqiMsg , uint8_t* buffer, uint32_t* bufferOffset);

    uint64_t WaitForPhyTimingInd (void);
    void NiSetTti (uint64_t tti_us);

    NiPhyTimingIndEndOkCallback m_niPhyTimingIndEndOkCallback;
    NiPhyRxDataEndOkCallback m_niPhyRxDataEndOkCallback;
    NiPhyRxCtrlEndOkCallback m_niPhyRxCtrlEndOkCallback;
    NiPhyRxCqiReportCallback m_niPhyRxCqiReportCallback;
    NiPhySpectrumModelCallback m_niPhySpectrumModelCallback;

    bool m_enableNiApi;
    bool m_enableNiApiLoopback;

    bool m_initializationDone;

    NiApiDevType_t m_niApiDevType;
    Ns3LteDevType_t m_ns3DevType;

    bool     m_sfnSynced;
    bool     m_mibReceived;
    uint32_t m_mibSfn;
    uint32_t m_sfn;
    uint8_t  m_tti;
    uint32_t m_cellId;
    uint32_t m_rbBitmap;
    uint16_t m_rnti;
    uint32_t m_mcs;
    uint32_t m_tbsSize;
    bool     m_firstPhyTimingInd;

    double   m_chSinrDb;
    double   m_chSinrLin;
    uint32_t m_niCqiReportPeriodMs;

    uint32_t m_niApiCountTxControlDataPackets;
    uint32_t m_niApiCountTxPayloadDataPackets;

    Ptr<NiUdpTransport> m_niUdpTransport;
    Ptr<NiPipeTransport> m_niPipeTransport;
    Ptr<NiLteSdrTimingSync> m_niLteSdrTimingSync;

    // default ip address / port configuration for station #1
    std::string m_niUdpSta1RemoteIpAddrTx="127.0.0.1";
    std::string m_niUdpSta1RemotePortTx="12802";
    std::string m_niUdpSta1LocalPortRx="12801";
    // default ip address / port configuration for station #2
    std::string m_niUdpSta2RemoteIpAddrTx="127.0.0.1";
    std::string m_niUdpSta2RemotePortTx="12801";
    std::string m_niUdpSta2LocalPortRx="12802";

    int64_t m_sfnSfOffset;
    uint64_t m_lastTimingIndTimeUs;
  };

} /* namespace ns3 */


#endif /* NI_LTE_PHY_INTERFACE_H_ */

