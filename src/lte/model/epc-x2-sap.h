/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          MC Dual Connectivity functionalities
 * Modified by: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 *          Dual Connectivity functionalities configured for DALI
 */

#ifndef EPC_X2_SAP_H
#define EPC_X2_SAP_H

#include "ns3/packet.h"
#include "ns3/eps-bearer.h"
#include "ns3/ipv4-address.h"
#include <ns3/lte-rrc-sap.h>
#include <ns3/lte-enb-cmac-sap.h>

#include <bitset>

namespace ns3 {


class Node;

/**
 * The X2 SAP defines the service between the X2 entity and the RRC entity.
 * 
 * The X2 SAP follows the specification 3GPP TS 36.423: "X2 application protocol (X2AP)"
 * 
 * The service primitives corresponds to the X2AP procedures and messages and 
 * the service parameters corresponds to the Information Elements
 * 
 * Note: Any reference in this file refers to the 3GPP TS 36.423 specification
 */

/**
 * \brief Common structures for EpcX2SapProvider and EpcX2SapUser
 */
class EpcX2Sap
{
public:
  virtual ~EpcX2Sap ();
  
  /**
   * E-RABs to be setup item as
   * it is used in the HANDOVER REQUEST message.
   * See section 9.1.1.1 for further info about the parameters
   */
  struct ErabToBeSetupItem
  {
    uint16_t    erabId;
    EpsBearer   erabLevelQosParameters;
    bool        dlForwarding;
    Ipv4Address transportLayerAddress;
    uint32_t    gtpTeid;

    ErabToBeSetupItem ();
  };

  /**
   * E-RABs admitted item as
   * it is used in the HANDOVER REQUEST ACKNOWLEDGE message.
   * See section 9.1.1.2 for further info about the parameters
   */
  struct ErabAdmittedItem
  {
    uint16_t    erabId;
    uint32_t    ulGtpTeid;
    uint32_t    dlGtpTeid;
  };

  /**
   * E-RABs not admitted item as
   * it is used in the HANDOVER REQUEST ACKNOWLEDGE message.
   * See section 9.1.1.2 for further info about the parameters
   */
  struct ErabNotAdmittedItem
  {
    uint16_t    erabId;
    uint16_t    cause;
  };

  /**
   * E-RABs subject to status transfer item as
   * it is used in the SN STATUS TRANSFER message.
   * See section 9.1.1.4 for further info about the parameters
   */
  static const uint16_t m_maxPdcpSn = 4096;
  struct ErabsSubjectToStatusTransferItem
  {
    uint16_t            erabId;
    std::bitset<m_maxPdcpSn> receiveStatusOfUlPdcpSdus;
    uint16_t            ulPdcpSn;
    uint32_t            ulHfn;
    uint16_t            dlPdcpSn;
    uint32_t            dlHfn;
  };

  /**
   * UL Interference OverloadIndication as
   * it is used in the LOAD INFORMATION message.
   * See section 9.2.17 for further info about the values
   */
  enum UlInterferenceOverloadIndicationItem
  {
    HighInterference,
    MediumInterference,
    LowInterference
  };

  /**
   * UL High Interference Information as
   * it is used in the LOAD INFORMATION message.
   * See section 9.1.2.1 for further info about the parameters
   */
  struct UlHighInterferenceInformationItem
  {
    uint16_t    targetCellId;
    std::vector <bool> ulHighInterferenceIndicationList;
  };

  /**
   * Relative Narrowband Tx Power (RNTP) as
   * it is used in the LOAD INFORMATION message.
   * See section 9.2.19 for further info about the parameters
   * 
   * Note: You can use INT16_MIN value for -infinite value
   *       in the rntpThreshold field
   */
  struct RelativeNarrowbandTxBand
  {
    std::vector <bool> rntpPerPrbList;
    int16_t     rntpThreshold;
    uint16_t    antennaPorts;
    uint16_t    pB;
    uint16_t    pdcchInterferenceImpact;
  };

  /**
   * Cell Information Item as
   * it is used in the LOAD INFORMATION message.
   * See section 9.1.2.1 for further info about the parameters
   */
  struct CellInformationItem
  {
    uint16_t    sourceCellId;
    std::vector <UlInterferenceOverloadIndicationItem> ulInterferenceOverloadIndicationList;
    std::vector <UlHighInterferenceInformationItem> ulHighInterferenceInformationList;
    RelativeNarrowbandTxBand relativeNarrowbandTxBand;
  };

  /**
   * Load Indicator as
   * it is used in the RESOURCE STATUS UPDATE message.
   * See section 9.2.36 for further info about the value
   */
  enum LoadIndicator
  {
    LowLoad,
    MediumLoad,
    HighLoad,
    Overload
  };

  /**
   * Composite Available Capacity as
   * it is used in the RESOURCE STATUS UPDATE message.
   * See section 9.2.45 for further info about the parameters
   */
  struct CompositeAvailCapacity
  {
    uint16_t    cellCapacityClassValue;
    uint16_t    capacityValue;
  };

  /**
   * Cell Measurement Result Item as
   * it is used in the RESOURCE STATUS UPDATE message.
   * See section 9.1.2.14 for further info about the parameters
   */
  struct CellMeasurementResultItem
  {
    uint16_t        sourceCellId;

    LoadIndicator   dlHardwareLoadIndicator;
    LoadIndicator   ulHardwareLoadIndicator;

    LoadIndicator   dlS1TnlLoadIndicator;
    LoadIndicator   ulS1TnlLoadIndicator;

    uint16_t        dlGbrPrbUsage;
    uint16_t        ulGbrPrbUsage;
    uint16_t        dlNonGbrPrbUsage;
    uint16_t        ulNonGbrPrbUsage;
    uint16_t        dlTotalPrbUsage;
    uint16_t        ulTotalPrbUsage;

    CompositeAvailCapacity  dlCompositeAvailableCapacity;
    CompositeAvailCapacity  ulCompositeAvailableCapacity;
  };


  enum IdCause
  {
    HandoverDesirableForRadioReason,
    TimeCriticalHandover
  };

  /**
   * \brief Parameters of the RlcSetupRequest to handle DALI dual connectivity, based on MC implementation
   *
   * Forward UE params during the DC setup
   */
  struct RlcSetupRequest
  {
    uint16_t    sourceCellId;
    uint16_t    targetCellId;
    uint32_t    gtpTeid;
    uint16_t    secondaryRnti;
    uint16_t    masterRnti;
    uint8_t     drbid;
    LteEnbCmacSapProvider::LcInfo lcinfo;
    LteRrcSap::RlcConfig rlcConfig;
    LteRrcSap::LogicalChannelConfig logicalChannelConfig;
    TypeId      rlcType;
  };
  
  /**
   * \brief Parameters of the HANDOVER REQUEST message.
   *
   * See section 9.1.1.1 for further info about the parameters
   */
  struct HandoverRequestParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            cause;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    uint32_t            mmeUeS1apId;
    uint64_t            ueAggregateMaxBitRateDownlink;
    uint64_t            ueAggregateMaxBitRateUplink;
    std::vector <ErabToBeSetupItem> bearers;
    Ptr<Packet>         rrcContext;
  };

  /**
   * \brief Parameters of the HANDOVER REQUEST ACKNOWLEDGE message.
   *
   * See section 9.1.1.2 for further info about the parameters
   */
  struct HandoverRequestAckParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            newEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    std::vector <ErabAdmittedItem> admittedBearers;
    std::vector <ErabNotAdmittedItem> notAdmittedBearers;
    Ptr<Packet>         rrcContext;
  };

  /**
   * \brief Parameters of the HANDOVER PREPARATION FAILURE message.
   *
   * See section 9.1.1.3 for further info about the parameters
   */
  struct HandoverPreparationFailureParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    uint16_t            cause;
    uint16_t            criticalityDiagnostics;
  };

  /**
   * \brief Parameters of the SN STATUS TRANSFER message.
   *
   * See section 9.1.1.4 for further info about the parameters
   */
  struct SnStatusTransferParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            newEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    std::vector <ErabsSubjectToStatusTransferItem> erabsSubjectToStatusTransferList;
  };

  /**
   * \brief Parameters of the UE CONTEXT RELEASE message.
   *
   * See section 9.1.1.5 for further info about the parameters
   */
  struct UeContextReleaseParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            newEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
  };

  /**
   * \brief Parameters of the LOAD INFORMATION message.
   *
   * See section 9.1.2.1 for further info about the parameters
   */
  struct LoadInformationParams
  {
    uint16_t            targetCellId;
    std::vector <CellInformationItem> cellInformationList;
  };

  /**
   * \brief Parameters of the RESOURCE STATUS UPDATE message.
   *
   * See section 9.1.2.14 for further info about the parameters
   */
  struct ResourceStatusUpdateParams
  {
    uint16_t            targetCellId;
    uint16_t            enb1MeasurementId;
    uint16_t            enb2MeasurementId;
    std::vector <CellMeasurementResultItem> cellMeasurementResultList;
  };

  /**
   * \brief Parameters of the UE DATA primitive
   *
   * Forward UE data during the handover procedure from source eNB (sourceCellId)
   * to target eNB (targetCellId) using a GTP-U tunnel (gtpTeid)
   */
  struct UeDataParams
  {
    uint16_t    sourceCellId;
    uint16_t    targetCellId;
    uint32_t    gtpTeid;
    Ptr<Packet> ueData;
  };

};

/**
 * DALI DC primitives, based on MC implementation. Part of X2 entity, called by PDCP.
 */
class EpcX2PdcpProvider : public EpcX2Sap
{
public:
  virtual ~EpcX2PdcpProvider ();

  /*
   * Service primitives
   */
  // X2 sends a Pdcp PDU in downlink to the SeNB for transmission to the sUE
  virtual void SendDcPdcpPdu (UeDataParams params) = 0;
};


/**
 * DALI DC primitives, based on MC implementation. Part of PDCP entity, called by X2
 */
class EpcX2PdcpUser : public EpcX2Sap
{
public:
  virtual ~EpcX2PdcpUser ();

  /*
   * Service primitives
   */
  // Receive a PDCP PDU in uplink from the SeNB for transmission to CN
  virtual void ReceiveDcPdcpPdu (UeDataParams params) = 0;
};


/**
 * DALI DC primitives, based on MC implementation. Part of X2 entity, called by RLC
 */
class EpcX2RlcProvider : public EpcX2Sap
{
public:
  virtual ~EpcX2RlcProvider ();

  /*
   * Service primitives
   */
  // Receive a PDCP SDU from RLC for uplink transmission to PDCP in MeNB
  virtual void ReceiveDcPdcpSdu (UeDataParams params) = 0;
};


/**
 * DALI DC primitives, based on MC implementation. Part of RLC entity, called by X2
 */
class EpcX2RlcUser : public EpcX2Sap
{
public:
  virtual ~EpcX2RlcUser ();

  /*
   * Service primitives
   */
  // X2 sends a PDCP SDU to RLC for downlink transmission to the UE
  virtual void SendDcPdcpSdu (UeDataParams params) = 0;
};

/**
 * These service primitives of this part of the X2 SAP
 * are provided by the X2 entity and issued by RRC entity
 */
class EpcX2SapProvider : public EpcX2Sap
{
public:
  virtual ~EpcX2SapProvider ();

  /**
   * Service primitives
   */

  virtual void SendHandoverRequest (HandoverRequestParams params) = 0;

  virtual void SendHandoverRequestAck (HandoverRequestAckParams params) = 0;

  virtual void SendHandoverPreparationFailure (HandoverPreparationFailureParams params) = 0;

  virtual void SendSnStatusTransfer (SnStatusTransferParams params) = 0;

  virtual void SendUeContextRelease (UeContextReleaseParams params) = 0;

  virtual void SendLoadInformation (LoadInformationParams params) = 0;

  virtual void SendResourceStatusUpdate (ResourceStatusUpdateParams params) = 0;

  virtual void SendUeData (UeDataParams params) = 0;
  
  virtual void SetEpcX2PdcpUser (uint32_t teid, EpcX2PdcpUser * s) = 0;

  virtual void SetEpcX2RlcUser (uint32_t teid, EpcX2RlcUser * s) = 0;

  virtual void SendRlcSetupRequest (RlcSetupRequest params) = 0;

  virtual void SendRlcSetupCompleted (UeDataParams params) = 0;
  
};


/**
 * These service primitives of this part of the X2 SAP
 * are provided by the RRC entity and issued by the X2 entity
 */
class EpcX2SapUser : public EpcX2Sap
{
public:
  virtual ~EpcX2SapUser ();

  /**
   * Service primitives
   */

  /**
   * Receive handover request function
   * \param params the handover request parameters
   */
  virtual void RecvHandoverRequest (HandoverRequestParams params) = 0;

  /**
   * Receive handover request ack function
   * \param params the handover request ack parameters
   */
  virtual void RecvHandoverRequestAck (HandoverRequestAckParams params) = 0;
  
  /**
   * Receive handover preparation failure function
   * \param params the handover preparation failure parameters
   */
  virtual void RecvHandoverPreparationFailure (HandoverPreparationFailureParams params) = 0;
  
  /**
   * Receive SN status transfer function
   * \param params the SN status transfer parameters
   */
  virtual void RecvSnStatusTransfer (SnStatusTransferParams params) = 0;
  
  /**
   * Receive UE context release function
   * \param params the receive UE context release parameters
   */
  virtual void RecvUeContextRelease (UeContextReleaseParams params) = 0;

  /**
   * Receive load information function
   * \param params the load information parameters
   */
  virtual void RecvLoadInformation (LoadInformationParams params) = 0;

  /**
   * Receive resource status update function
   * \param params the resource status update parameters
   */
  virtual void RecvResourceStatusUpdate (ResourceStatusUpdateParams params) = 0;

  virtual void RecvRlcSetupRequest (RlcSetupRequest params) = 0;

  virtual void RecvRlcSetupCompleted (UeDataParams params) = 0;

  /**
   * Receive UE data function
   * \param params UE data parameters
   */
  virtual void RecvUeData (UeDataParams params) = 0;
  
};

///////////////////////////////////////

/**
 * EpcX2SpecificEpcX2SapProvider
 */
template <class C>
class EpcX2SpecificEpcX2SapProvider : public EpcX2SapProvider
{
public:
  /**
   * Constructor
   *
   * \param x2 the owner class
   */
  EpcX2SpecificEpcX2SapProvider (C* x2);

  //
  // Interface implemented from EpcX2SapProvider
  //

  /**
   * Send handover request function
   * \param params the hadnover request parameters
   */
  virtual void SendHandoverRequest (HandoverRequestParams params);

  /**
   * Send handover request ack function
   * \param params the handover request ack pararameters
   */
  virtual void SendHandoverRequestAck (HandoverRequestAckParams params);

  /**
   * Send handover preparation failure function
   * \param params the handover preparation failure parameters
   */
  virtual void SendHandoverPreparationFailure (HandoverPreparationFailureParams params);

  /**
   * Send SN status transfer function
   * \param params the SN status transfer parameters
   */
  virtual void SendSnStatusTransfer (SnStatusTransferParams params);

  /**
   * Send UE context release function
   * \param params the UE context release parameters
   */
  virtual void SendUeContextRelease (UeContextReleaseParams params);

  /**
   * Send load information function
   * \param params the load information parameters
   */
  virtual void SendLoadInformation (LoadInformationParams params);

  /**
   * Send resource status update function
   * \param params the resource status update parameters
   */
  virtual void SendResourceStatusUpdate (ResourceStatusUpdateParams params);

  /**
   * Send UE data function
   * \param params the UE data parameters
   */
  virtual void SendUeData (UeDataParams params);

  virtual void SetEpcX2PdcpUser (uint32_t teid, EpcX2PdcpUser * s);

  virtual void SetEpcX2RlcUser (uint32_t teid, EpcX2RlcUser * s);

  virtual void SendRlcSetupRequest (RlcSetupRequest params);

  virtual void SendRlcSetupCompleted (UeDataParams params);

private:
  EpcX2SpecificEpcX2SapProvider ();
  C* m_x2;
};

template <class C>
EpcX2SpecificEpcX2SapProvider<C>::EpcX2SpecificEpcX2SapProvider (C* x2)
  : m_x2 (x2)
{
}

template <class C>
EpcX2SpecificEpcX2SapProvider<C>::EpcX2SpecificEpcX2SapProvider ()
{
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendHandoverRequest (HandoverRequestParams params)
{
  m_x2->DoSendHandoverRequest (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendHandoverRequestAck (HandoverRequestAckParams params)
{
  m_x2->DoSendHandoverRequestAck (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendHandoverPreparationFailure (HandoverPreparationFailureParams params)
{
  m_x2->DoSendHandoverPreparationFailure (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendSnStatusTransfer (SnStatusTransferParams params)
{
  m_x2->DoSendSnStatusTransfer (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendUeContextRelease (UeContextReleaseParams params)
{
  m_x2->DoSendUeContextRelease (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendLoadInformation (LoadInformationParams params)
{
  m_x2->DoSendLoadInformation (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendResourceStatusUpdate (ResourceStatusUpdateParams params)
{
  m_x2->DoSendResourceStatusUpdate (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendUeData (UeDataParams params)
{
  m_x2->DoSendUeData (params);
}

/**
 * EpcX2SpecificEpcX2SapUser
 */
template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SetEpcX2RlcUser (uint32_t teid, EpcX2RlcUser * s)
{
  m_x2->SetDcEpcX2RlcUser (teid, s);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SetEpcX2PdcpUser (uint32_t teid, EpcX2PdcpUser * s)
{
  m_x2->SetDcEpcX2PdcpUser (teid, s);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendRlcSetupRequest (RlcSetupRequest params)
{
  m_x2->DoSendRlcSetupRequest (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapProvider<C>::SendRlcSetupCompleted (UeDataParams params)
{
  m_x2->DoSendRlcSetupCompleted (params);
}

///////////////////////////////////////

/**
 * EpcX2SpecificEpcX2SapUser
 */
template <class C>
class EpcX2SpecificEpcX2SapUser : public EpcX2SapUser
{
public:
 /**
   * Constructor
   *
   * \param rrc RRC
   */
  EpcX2SpecificEpcX2SapUser (C* rrc);

  //
  // Interface implemented from EpcX2SapUser
  //

  /**
   * Receive handover request function
   * \param params the receive handover request parameters
   */
  virtual void RecvHandoverRequest (HandoverRequestParams params);

  /**
   * Receive handover request ack function
   * \param params the receive handover request ack parameters
   */
  virtual void RecvHandoverRequestAck (HandoverRequestAckParams params);

  /**
   * Receive handover preparation failure function
   * \param params the receive handover preparation failure parameters
   */
  virtual void RecvHandoverPreparationFailure (HandoverPreparationFailureParams params);

  /**
   * Receive SN status transfer function
   * \param params the SN status transfer parameters
   */
  virtual void RecvSnStatusTransfer (SnStatusTransferParams params);

  /**
   * Receive UE context release function
   * \param params the UE context release parameters
   */
  virtual void RecvUeContextRelease (UeContextReleaseParams params);

  /**
   * Receive load information function
   * \param params the load information parameters
   */
  virtual void RecvLoadInformation (LoadInformationParams params);

  /**
   * Receive resource status update function
   * \param params the receive resource status update
   */
  virtual void RecvResourceStatusUpdate (ResourceStatusUpdateParams params);

  virtual void RecvRlcSetupRequest (RlcSetupRequest params);

  virtual void RecvRlcSetupCompleted (UeDataParams params);

  /**
   * Receive UE data function
   * \param params the UE data parameters
   */
  virtual void RecvUeData (UeDataParams params);

private:
  EpcX2SpecificEpcX2SapUser ();
  C* m_rrc;
};

template <class C>
EpcX2SpecificEpcX2SapUser<C>::EpcX2SpecificEpcX2SapUser (C* rrc)
  : m_rrc (rrc)
{
}

template <class C>
EpcX2SpecificEpcX2SapUser<C>::EpcX2SpecificEpcX2SapUser ()
{
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvHandoverRequest (HandoverRequestParams params)
{
  m_rrc->DoRecvHandoverRequest (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvHandoverRequestAck (HandoverRequestAckParams params)
{
  m_rrc->DoRecvHandoverRequestAck (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvHandoverPreparationFailure (HandoverPreparationFailureParams params)
{
  m_rrc->DoRecvHandoverPreparationFailure (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvSnStatusTransfer (SnStatusTransferParams params)
{
  m_rrc->DoRecvSnStatusTransfer (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvUeContextRelease (UeContextReleaseParams params)
{
  m_rrc->DoRecvUeContextRelease (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvLoadInformation (LoadInformationParams params)
{
  m_rrc->DoRecvLoadInformation (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvResourceStatusUpdate (ResourceStatusUpdateParams params)
{
  m_rrc->DoRecvResourceStatusUpdate (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvRlcSetupRequest (RlcSetupRequest params)
{
  m_rrc->DoRecvRlcSetupRequest (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvRlcSetupCompleted (UeDataParams params)
{
  m_rrc->DoRecvRlcSetupCompleted (params);
}

template <class C>
void
EpcX2SpecificEpcX2SapUser<C>::RecvUeData (UeDataParams params)
{
  m_rrc->DoRecvUeData (params);
}

/////////////////////////////////////////////

template <class C>
class EpcX2PdcpSpecificProvider : public EpcX2PdcpProvider
{
public:
  EpcX2PdcpSpecificProvider (C* x2);

  // Inherited
  virtual void SendDcPdcpPdu (UeDataParams params);

private:
  EpcX2PdcpSpecificProvider ();
  C* m_x2;
};

template <class C>
EpcX2PdcpSpecificProvider<C>::EpcX2PdcpSpecificProvider (C* x2)
  : m_x2 (x2)
{
}

template <class C>
EpcX2PdcpSpecificProvider<C>::EpcX2PdcpSpecificProvider ()
{
}

template <class C>
void
EpcX2PdcpSpecificProvider<C>::SendDcPdcpPdu(UeDataParams params)
{
  m_x2->DoSendDcPdcpPdu(params);
}

/////////////////////////////////////////////
template <class C>
class EpcX2RlcSpecificProvider : public EpcX2RlcProvider
{
public:
  EpcX2RlcSpecificProvider (C* x2);

  // Inherited
  virtual void ReceiveDcPdcpSdu (UeDataParams params);

private:
  EpcX2RlcSpecificProvider ();
  C* m_x2;
};

template <class C>
EpcX2RlcSpecificProvider<C>::EpcX2RlcSpecificProvider (C* x2)
  : m_x2 (x2)
{
}

template <class C>
EpcX2RlcSpecificProvider<C>::EpcX2RlcSpecificProvider ()
{
}

template <class C>
void
EpcX2RlcSpecificProvider<C>::ReceiveDcPdcpSdu(UeDataParams params)
{
  m_x2->DoReceiveDcPdcpSdu(params);
}

/////////////////////////////////////////////
template <class C>
class EpcX2PdcpSpecificUser : public EpcX2PdcpUser
{
public:
  EpcX2PdcpSpecificUser (C* pdcp);

  // Inherited
  virtual void ReceiveDcPdcpPdu (UeDataParams params);

private:
  EpcX2PdcpSpecificUser ();
  C* m_pdcp;
};

template <class C>
EpcX2PdcpSpecificUser<C>::EpcX2PdcpSpecificUser (C* pdcp)
  : m_pdcp (pdcp)
{
}

template <class C>
EpcX2PdcpSpecificUser<C>::EpcX2PdcpSpecificUser ()
{
}

template <class C>
void
EpcX2PdcpSpecificUser<C>::ReceiveDcPdcpPdu(UeDataParams params)
{
  m_pdcp->DoReceiveDcPdcpPdu(params);
}

/////////////////////////////////////////////
template <class C>
class EpcX2RlcSpecificUser : public EpcX2RlcUser
{
public:
  EpcX2RlcSpecificUser (C* rlc);

  // Inherited
  virtual void SendDcPdcpSdu (UeDataParams params);

private:
  EpcX2RlcSpecificUser ();
  C* m_rlc;
};

template <class C>
EpcX2RlcSpecificUser<C>::EpcX2RlcSpecificUser (C* rlc)
  : m_rlc (rlc)
{
}

template <class C>
EpcX2RlcSpecificUser<C>::EpcX2RlcSpecificUser ()
{
}

template <class C>
void
EpcX2RlcSpecificUser<C>::SendDcPdcpSdu(UeDataParams params)
{
  m_rlc->DoSendDcPdcpSdu(params);
}

} // namespace ns3

#endif // EPC_X2_SAP_H
