/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 * Modified by: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 * Extension of DALI Dual Connectivity Configuration and Split Bearer between LTE UEs
 */

#ifndef DALI_UE_PDCP_H
#define DALI_UE_PDCP_H

#include <ns3/traced-value.h>
#include <ns3/trace-source-accessor.h>

#include <ns3/object.h>

#include <ns3/dali-ue-dcx-sap.h>
#include <ns3/dali-ue-dcx.h>
#include <ns3/lte-pdcp-sap.h>
#include <ns3/lte-rlc-sap.h>
#include <ns3/lte-pdcp.h>

namespace ns3 {

/**
 * DALI UE PDCP entity. It has 2 interfaces to the 2 RLC layers,
 * the local and the remote one. The interface to the local is a
 * Rlc Sap, while the interface to the remote is offered by the 
 * DaliUeDcxSap.
 * Note: there is a single lcid (no problem in having the same
 * in the 2 UE), but 2 Rnti & Imsi.
 *
 * DALI Dual Connectivity Scenario allows to join the Split Bearer between Master and Secondary UEs.
 */
class DaliUePdcp : public LtePdcp
{
  friend class DaliUePdcpSpecificLteRlcSapUser;
  friend class LtePdcpSpecificLtePdcpSapProvider<DaliUePdcp>;
  friend class UeDcxPdcpSpecificProvider <DaliUeDcx>;
  friend class UeDcxPdcpSpecificUser <DaliUePdcp>;
public:
  DaliUePdcp ();
  virtual ~DaliUePdcp ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Set the RNTI of the UE in the SeNB
   *
   * \param rnti
   */
  void SetSenbRnti (uint16_t rnti);

  /**
   * Set the RNTI of the UE in the MeNB
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);

  /**
   * Set the ldid
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   *
   *
   * \param s the UeDcxPDCP Provider to the UE DCX interface
   */
  void SetUeDcxPdcpProvider (UeDcxPdcpProvider * s);

  /**
   *
   *
   * \return the UeDcxPDCP User, given to DCX to access PDCP Receive method
   */
  UeDcxPdcpUser* GetUeDcxPdcpUser ();

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
   * Set the param needed for DCX tunneling
   * \param the UeDataParams defined in RRC
   */
  void SetUeDataParams(DaliUeDcxSap::UeDataParams params);

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

protected:
  // Interface provided to upper RRC entity
  virtual void DoTransmitPdcpSdu (Ptr<Packet> p);

  LtePdcpSapUser* m_pdcpSapUser;
  LtePdcpSapProvider* m_pdcpSapProvider;

  // Interface provided to lower RLC entity
  virtual void DoReceivePdu (Ptr<Packet> p);

  LteRlcSapUser* m_rlcSapUser;
  LteRlcSapProvider* m_rlcSapProvider;

  uint16_t m_rnti;     //RNTI of the UE in the MeNB
  uint8_t m_lcid;
  uint16_t m_sEnbRnti; //RNTI of the UE in the SeNB

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

  // Interface provided to DaliUeDcx entity
  virtual void DoReceiveDcPdcpPdu(DaliUeDcxSap::UeDataParams params);

  UeDcxPdcpProvider* m_ueDcxPdcpProvider;
  UeDcxPdcpUser* m_ueDcxPdcpUser;

private:
  /**
   * State variables. See section 7.1 in TS 36.323
   */
  uint16_t m_txSequenceNumber;
  uint16_t m_rxSequenceNumber;

  // UeDataParams needed to forward data to sUE
  DaliUeDcxSap::UeDataParams m_ueDataParams;

  /**
   * Constants. See section 7.2 in TS 36.323
   */
  static const uint16_t m_maxPdcpSn = 4095;

  /**
   * Specific flags for DALI LTE Dual Connectivity
   */
  bool m_useDualConnectivity;
  bool m_sendOverSecondary; //Identify if Packet is sent over DaliUeDcx interface for RLC of sUE, otherwise local RLC would be used

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

#endif // DALI_UE_PDCP_H
