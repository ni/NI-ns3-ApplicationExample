/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
 * Copyright (c) 2019, Universitat Politecnica de Catalunya
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 * Extension to DC devices by Michele Polese <michele.polese@gmail.com>
 *
 * Modified by: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 * Extension of DALI Dual Connectivity Configuration and Split Bearer between LTE eNBs
 */

#ifndef DALI_ENB_PDCP_H
#define DALI_ENB_PDCP_H

#include <ns3/traced-value.h>
#include <ns3/trace-source-accessor.h>

#include <ns3/object.h>

#include <ns3/epc-x2-sap.h>
#include <ns3/epc-x2.h>
#include <ns3/lte-pdcp-sap.h>
#include <ns3/lte-rlc-sap.h>
#include <ns3/lte-pdcp.h>

namespace ns3 {

/**
 * DALI eNB PDCP entity. It has 2 interfaces to the 2 RLC layers,
 * the local and the remote one. The interface to the local is a
 * Rlc Sap, while the interface to the remote is offered by the 
 * EpcX2Sap.
 * Note: there is a single IMSI and lcid (no problem in having the same
 * in the 2 eNBs), but 2 rnti.
 *
 * DALI Dual Connectivity Scenario allows Split Bearer between Master and Secondary eNBs.
 */
class DaliEnbPdcp : public LtePdcp
{
  friend class DaliEnbPdcpSpecificLteRlcSapUser;
  friend class LtePdcpSpecificLtePdcpSapProvider<DaliEnbPdcp>;
  friend class EpcX2PdcpSpecificProvider <EpcX2>;
  friend class EpcX2PdcpSpecificUser <DaliEnbPdcp>;
public:
  DaliEnbPdcp ();
  virtual ~DaliEnbPdcp ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Set the RNTI of the UE in the Master eNB
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);

  /**
   * Set the RNTI of the UE in the Secondary eNB
   *
   * \param rnti
   */
  void SetSenbRnti (uint16_t rnti);

  /**
   * Set the lcid
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   *
   *
   * \param s the EpcX2PDCP Provider to the Epc X2 interface
   */
  void SetEpcX2PdcpProvider (EpcX2PdcpProvider * s);

  /**
   *
   *
   * \return the EpcX2PDCP User, given to X2 to access PDCP Receive method
   */
  EpcX2PdcpUser* GetEpcX2PdcpUser ();

  /**
   *
   *
   * \param s the PDCP SAP user to be used by this LTE_PDCP
   */
  void SetLtePdcpSapUser (LtePdcpSapUser * s);

  /**
   *
   *
   * \return the PDCP SAP Provider interface offered to the RRC by this LTE_PDCP
   */
  LtePdcpSapProvider* GetLtePdcpSapProvider ();

  /**
   *
   *
   * \param s the RLC SAP Provider to be used by this LTE_PDCP
   */
  void SetLteRlcSapProvider (LteRlcSapProvider * s);

  /**
   *
   *
   * \return the RLC SAP User interface offered to the RLC by this LTE_PDCP
   */
  LteRlcSapUser* GetLteRlcSapUser ();

  static const uint16_t MAX_PDCP_SN = 4096;

  /**
   * Status variables of the PDCP
   * 
   */
  struct Status
  {
    uint16_t txSn; ///< TX sequence number
    uint16_t rxSn; ///< RX sequence number
  };

  /** 
   * 
   * \return the current status of the PDCP
   */
  Status GetStatus ();

  /**
   * Set the status of the PDCP
   * 
   * \param s 
   */
  void SetStatus (Status s);

  /**
   * Set the param needed for X2 tunneling
   * \param the UeDataParams defined in RRC
   */
  void SetUeDataParams(EpcX2Sap::UeDataParams params);

  /**
   * TracedCallback for PDU transmission event.
   *
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] size Packet size.
   */
  typedef void (* PduTxTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t size);

  /**
   * TracedCallback signature for PDU receive event.
   *
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] size Packet size.
   * \param [in] delay Delay since packet sent, in ns..
   */
  typedef void (* PduRxTracedCallback)
    (const uint16_t rnti, const uint8_t lcid,
     const uint32_t size, const uint64_t delay);

  /**
   * Set DALI LTE Dual Connectivity
   */
  void UseDualConnectivity(bool useDualConnectivity);

  /**
   * Return true if this PDCP is configured to use DALI LTE Dual Connectivity
   */
  bool GetUseDualConnectivity() const;

  /**
   * Set DALI In-sequence delivery of packets
   */
  void UseInSequenceDelivery(bool useInSequenceDelivery);

  /**
   * Return true if this PDCP is configured to use DALI In-sequence delivery of packets
   */
  bool GetUseInSequenceDelivery() const;

  /**
     * Transmit packet over LWA/LWIP
     */
  void TransmitOverLwaLwip(LteRlcSapProvider::TransmitPdcpPduParameters params, Ptr<Packet> p);

protected:
  // Interface provided to upper RRC entity
  virtual void DoTransmitPdcpSdu (Ptr<Packet> p);

  LtePdcpSapUser* m_pdcpSapUser;
  LtePdcpSapProvider* m_pdcpSapProvider;

  // Interface provided to lower RLC entity
  virtual void DoReceivePdu (Ptr<Packet> p);

  LteRlcSapUser* m_rlcSapUser;
  LteRlcSapProvider* m_rlcSapProvider;

  uint16_t m_rnti;
  uint8_t m_lcid;
  uint16_t m_sEnbRnti;

  /**
   * Used to inform of a PDU delivery to the RLC SAP provider.
   * The parameters are RNTI, LCID and bytes delivered
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception from the RLC SAP user.
   * The parameters are RNTI, LCID, bytes delivered and delivery delay in nanoseconds. 
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

  // Interface provided to EpcX2 entity
  virtual void DoReceiveDcPdcpPdu(EpcX2Sap::UeDataParams params);

  EpcX2PdcpProvider* m_epcX2PdcpProvider;
  EpcX2PdcpUser* m_epcX2PdcpUser;

  // lwa/lwip changes
  TracedCallback<Ptr<const Packet> >m_pdcptxtrace ;

    uint32_t pdcp_decisionlwa;
    uint32_t pdcp_decisionlwip;
    uint32_t pdcp_decisionDc;
    uint32_t m_packetCounter = 0;
    uint32_t m_dcLwaLwipSwitch = 0;  // switch between DC and LWA/LWIP when both of them are enabled 0->use DC, 1->use LWA/LWIP

private:
  /**
   * State variables. See section 7.1 in TS 36.323
   */
  uint16_t m_txSequenceNumber;
  uint16_t m_rxSequenceNumber;

  // UeDataParams needed to forward data to Secondary
  EpcX2Sap::UeDataParams m_ueDataParams;

  /**
   * Constants. See section 7.2 in TS 36.323
   */
  static const uint16_t m_maxPdcpSn = 4095;

  /**
   * Specific flags for DALI LTE Dual Connectivity
   */
  bool m_useDualConnectivity;
  bool m_sendOverSecondary; //Identify if Packet is sent over EpcX2 interface for RLC of SeNB, otherwise local RLC would be used

  /**
   * Member variables required for DALI In-sequence delivery of packets
   *
   * The `UseInSequenceDelivery` flag: Set in true if DALI In-sequence delivery of packets is going to be used
   * The `PdcpPduMap` attribute: List of PdcpPdu packets by Sequence Number (buffer), which have been received out of sequence
   * The `SequenceLost` flag: Set in true when there are out of sequence packets stored and waiting to be delivered
   * The `MaxPdcpRW` constant: Define the maximum Reordering Window in between a packet can be stored without triggering a buffer flush
   */
  bool m_useInSequenceDelivery;
  std::map<uint16_t, Ptr<Packet> > m_pdcpPduMap;
  bool m_sequenceLost;
  static const uint16_t m_maxPdcpRW = 2047; //Reordering Window

  /**
   * Send package to the RRC upper layer
   */
  void DoReceivePduSend (Ptr<Packet> p);

  /**
   * Increment and control of the Sequence Number
   */
  void RxSequenceNumberNext();

  /**
   * Executes the routine for the in-order delivery of available packages in buffer (iteratively) up to the last "consecutive" Sequence Number (without gaps).
   * During delivering iteratively, routine starts with the next expected Sequence Number and continues until the the next expected is not in the Buffer
   */
  void DoUnloadReceiveBuffer();

};


} // namespace ns3

#endif // DALI_ENB_PDCP_H
