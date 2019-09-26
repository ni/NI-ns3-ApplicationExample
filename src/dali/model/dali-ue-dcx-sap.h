/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 * Inspired by epc-x2-sap
 */

#ifndef DALI_UE_DCX_SAP_H
#define DALI_UE_DCX_SAP_H

#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include <ns3/lte-rrc-sap.h>
#include <bitset>
#include <map>

namespace ns3 {


class Node;

/**
 * The DCX SAP defines the service between the DCX entity and the RRC entity.
 *
 * The DCX SAP follows the needs of DALI Dual Connectivity Scenario and
 * allow to communicate UEs directly.
 *
 * Offer interfaces to communicate PDCP and RLC of different UEs
 *
 */

/**
 * \brief Common structures for UeDcxSapProvider and UeDcxSapUser
 */
class DaliUeDcxSap
{
public:
  virtual ~DaliUeDcxSap ();

  static const uint16_t m_maxPdcpSn = 4096;

  /**
   * \brief Parameters of the UE DATA primitive
   *
   * Forward UE data during DALI Dual Connectivity procedure from source UE (sourceImsi)
   * to target UE (targetImsi) using a GTP-U tunnel (gtpTeid)
   */
  struct UeDataParams
  {
	uint64_t    sourceImsi; ///< source UE Imsi
    uint64_t    targetImsi; ///< target UE Imsi
    uint32_t    gtpTeid; ///< GTP TEID
    Ptr<Packet> ueData; ///< UE data
  };

  /**
   * \brief Parameters of the RrcDcConnectionReconfiguration to handle DALI dual connectivity
   *
   * Forward LteRrcSap::RrcConnectionReconfiguration during the DC setup
   */
  /*struct RrcDcConnectionReconfiguration
  {
	uint64_t    sourceImsi; ///< source UE Imsi
	uint64_t    targetImsi; ///< target UE Imsi
	LteRrcSap::RrcConnectionReconfiguration RrcConnectionReconfiguration;
  };*/

  /**
   * \brief Parameters of the RrcDcConnectionReconfigurationCompleted structure
   *
   * Return LteRrcSap::RrcConnectionReconfigurationCompleted msg
   */
  /*struct RrcDcConnectionReconfigurationCompleted
  {
  	uint64_t    sourceImsi; ///< source UE Imsi
  	uint64_t    targetImsi; ///< target UE Imsi
  	LteRrcSap::RrcConnectionReconfigurationCompleted RrcConnectionReconfigurationCompleted;
  };*/
};

/**
 * DC primitives. Part of DCX entity, called by PDCP
 */
class UeDcxPdcpProvider : public DaliUeDcxSap
{
public:
  virtual ~UeDcxPdcpProvider ();

  /*
   * Service primitives
   */
  // DCX sends a Pdcp PDU in uplink to the UE connected to SeNB for transmission to the eNB
  virtual void SendDcPdcpPdu (UeDataParams params) = 0;
};


/**
 * DC primitives. Part of PDCP entity, called by DCX
 */
class UeDcxPdcpUser : public DaliUeDcxSap
{
public:
  virtual ~UeDcxPdcpUser ();

  /*
   * Service primitives
   */
  // Receive a PDCP PDU in Downlink from the UE connected to the SeNB for transmission to upper layers
  virtual void ReceiveDcPdcpPdu (UeDataParams params) = 0;
};


/**
 * DC primitives. Part of DCX entity, called by RLC
 */
class UeDcxRlcProvider : public DaliUeDcxSap
{
public:
  virtual ~UeDcxRlcProvider ();

  /*
   * Service primitives
   */
  // Receive a PDCP SDU from RLC for downlink transmission to PDCP in UE connected to MeNB
  virtual void ReceiveDcPdcpSdu (UeDataParams params) = 0;
};


/**
 * DC primitives. Part of RLC entity, called by DCX
 */
class UeDcxRlcUser : public DaliUeDcxSap
{
public:
  virtual ~UeDcxRlcUser ();

  /*
   * Service primitives
   */
  // DCX sends a PDCP SDU to RLC for uplink transmission to the SeNB
  virtual void SendDcPdcpSdu (UeDataParams params) = 0;
};

/**
 * These service primitives of this part of the DCX SAP
 * are provided by the DCX entity and issued by RRC entity
 */
class UeDcxSapProvider : public DaliUeDcxSap
{
public:
  virtual ~UeDcxSapProvider ();

  /**
   * Service primitives
   */
  virtual void SendUeData (UeDataParams params) = 0;

  virtual void SetUeDcxPdcpUser (uint32_t teid, UeDcxPdcpUser * s) = 0;

  virtual void SetUeDcxRlcUser (uint32_t teid, UeDcxRlcUser * s) = 0;

  virtual void AddTeidToBeForwarded (uint32_t gtpTeid, uint64_t targetImsi) = 0;
  // notify the UeDcx class that packets for a certain TEID must not be forwarded anymore
  virtual void RemoveTeidToBeForwarded (uint32_t gtpTeid) = 0;

  /**
   * \brief Send an _RrcDcConnectionReconfiguration_ message to a sUE
   *        during an DALI dual connectivity connection reconfiguration procedure
   * \param msg the message based on RrcDcConnectionReconfiguration structure
   */
  virtual void SendRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg) = 0;

  /**
   * \brief Send an _RrcDcConnectionReconfigurationCompleted_ message to a mUE
   *        during an DALI dual connectivity connection reconfiguration procedure
   * \param msg the message based on RrcDcConnectionReconfigurationConfiguration structure
   */
  virtual void SendRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg) = 0;
};


/**
 * These service primitives of this part of the DCX SAP
 * are provided by the RRC entity and issued by the DCX entity
 */
class UeDcxSapUser : public DaliUeDcxSap
{
public:
  virtual ~UeDcxSapUser ();

  /*
   * Service primitives
   */
  /**
   * Receive UE data function
   * \param params UE data parameters
   */
  virtual void RecvUeData (UeDataParams params) = 0;

  /**
   * \brief Receive an _RrcDcConnectionReconfiguration_ message from mUE
   *        during an DALI dual connectivity connection reconfiguration procedure
   * \param msg the message based on RrcDcConnectionReconfigurationConfiguration structure
   */
  virtual void RecvRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg) = 0;

  /**
   * \brief Receive an _RrcDcConnectionReconfigurationCompleted_ message from sUE
   *        during an DALI dual connectivity connection reconfiguration procedure
   * \param msg the message based on RrcDcConnectionReconfigurationConfigurationCompleted structure
   */
  virtual void RecvRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg) = 0;

};

///////////////////////////////////////

/**
 * UeDcxSpecificUeDcxSapProvider
 */
template <class C>
class UeDcxSpecificUeDcxSapProvider : public UeDcxSapProvider
{
public:
  /**
   * Constructor
   *
   * \param dcx the owner class
   */
  UeDcxSpecificUeDcxSapProvider (C* dcx);

  //
  // Interface implemented from UeDcxSapProvider
  //
  virtual void SendUeData (UeDataParams params);

  virtual void SetUeDcxPdcpUser (uint32_t teid, UeDcxPdcpUser * s);

  virtual void SetUeDcxRlcUser (uint32_t teid, UeDcxRlcUser * s);

  virtual void AddTeidToBeForwarded (uint32_t gtpTeid, uint64_t targetImsi);

  virtual void RemoveTeidToBeForwarded (uint32_t gtpTeid);

  virtual void SendRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg);

  virtual void SendRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg);

private:
  UeDcxSpecificUeDcxSapProvider ();
  C* m_dcx; ///< owner class
};

template <class C>
UeDcxSpecificUeDcxSapProvider<C>::UeDcxSpecificUeDcxSapProvider (C* dcx)
  : m_dcx (dcx)
{
}

template <class C>
UeDcxSpecificUeDcxSapProvider<C>::UeDcxSpecificUeDcxSapProvider ()
{
}

template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::SendUeData (UeDataParams params)
{
  m_dcx->DoSendUeData (params);
}

/**
 * UeDcxSpecificUeDcxSapUser
 */
template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::SetUeDcxRlcUser (uint32_t teid, UeDcxRlcUser * s)
{
  m_dcx->SetDcUeDcxRlcUser (teid, s);
}

template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::SetUeDcxPdcpUser (uint32_t teid, UeDcxPdcpUser * s)
{
  m_dcx->SetDcUeDcxPdcpUser (teid, s);
}
template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::AddTeidToBeForwarded (uint32_t gtpTeid, uint64_t targetImsi)
{
  m_dcx->DoAddTeidToBeForwarded(gtpTeid, targetImsi);
}

template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::RemoveTeidToBeForwarded (uint32_t gtpTeid)
{
  m_dcx->DoRemoveTeidToBeForwarded(gtpTeid);
}

template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::SendRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg)
{
  m_dcx->DoSendRrcDcConnectionReconfiguration (sourceImsi, targetImsi, msg);
}

template <class C>
void
UeDcxSpecificUeDcxSapProvider<C>::SendRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  m_dcx->DoSendRrcDcConnectionReconfigurationCompleted (sourceImsi, targetImsi, msg);
}

///////////////////////////////////////

template <class C>
class UeDcxSpecificUeDcxSapUser : public UeDcxSapUser
{
public:
  /**
   * Constructor
   *
   * \param rrc RRC
   */
	UeDcxSpecificUeDcxSapUser (C* rrc);

  //
  // Interface implemented from UeDcxSapUser
  //

  /**
   * Receive UE data function
   * \param params the UE data parameters
   */
  virtual void RecvUeData (UeDataParams params);

  virtual void RecvRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg);

  virtual void RecvRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg);

private:
  UeDcxSpecificUeDcxSapUser ();
  C* m_rrc; ///< owner class
};

template <class C>
UeDcxSpecificUeDcxSapUser<C>::UeDcxSpecificUeDcxSapUser (C* rrc)
  : m_rrc (rrc)
{
}

template <class C>
UeDcxSpecificUeDcxSapUser<C>::UeDcxSpecificUeDcxSapUser ()
{
}

template <class C>
void
UeDcxSpecificUeDcxSapUser<C>::RecvUeData (UeDataParams params)
{
  m_rrc->DoRecvEnbData (params);
}

template <class C>
void
UeDcxSpecificUeDcxSapUser<C>::RecvRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg)
{
  m_rrc->DoRecvRrcDcConnectionReconfiguration (sourceImsi, targetImsi, msg);
}

template <class C>
void
UeDcxSpecificUeDcxSapUser<C>::RecvRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  m_rrc->DoRecvRrcDcConnectionReconfigurationCompleted (sourceImsi, targetImsi, msg);
}

/////////////////////////////////////////////
template <class C>
class UeDcxPdcpSpecificProvider : public UeDcxPdcpProvider
{
public:
	UeDcxPdcpSpecificProvider (C* dcx);

  // Inherited
  virtual void SendDcPdcpPdu (UeDataParams params);

private:
  UeDcxPdcpSpecificProvider ();
  C* m_dcx;
};

template <class C>
UeDcxPdcpSpecificProvider<C>::UeDcxPdcpSpecificProvider (C* dcx)
  : m_dcx (dcx)
{
}

template <class C>
UeDcxPdcpSpecificProvider<C>::UeDcxPdcpSpecificProvider ()
{
}

template <class C>
void
UeDcxPdcpSpecificProvider<C>::SendDcPdcpPdu(UeDataParams params)
{
  m_dcx->DoSendDcPdcpPdu(params);
}

/////////////////////////////////////////////
template <class C>
class UeDcxRlcSpecificProvider : public UeDcxRlcProvider
{
public:
	UeDcxRlcSpecificProvider (C* dcx);

  // Inherited
  virtual void ReceiveDcPdcpSdu (UeDataParams params);

private:
  UeDcxRlcSpecificProvider ();
  C* m_dcx;
};

template <class C>
UeDcxRlcSpecificProvider<C>::UeDcxRlcSpecificProvider (C* dcx)
  : m_dcx (dcx)
{
}

template <class C>
UeDcxRlcSpecificProvider<C>::UeDcxRlcSpecificProvider ()
{
}

template <class C>
void
UeDcxRlcSpecificProvider<C>::ReceiveDcPdcpSdu(UeDataParams params)
{
  m_dcx->DoReceiveDcPdcpSdu(params);
}

/////////////////////////////////////////////
template <class C>
class UeDcxPdcpSpecificUser : public UeDcxPdcpUser
{
public:
	UeDcxPdcpSpecificUser (C* pdcp);

  // Inherited
  virtual void ReceiveDcPdcpPdu (UeDataParams params);

private:
  UeDcxPdcpSpecificUser ();
  C* m_pdcp;
};

template <class C>
UeDcxPdcpSpecificUser<C>::UeDcxPdcpSpecificUser (C* pdcp)
  : m_pdcp (pdcp)
{
}

template <class C>
UeDcxPdcpSpecificUser<C>::UeDcxPdcpSpecificUser ()
{
}

template <class C>
void
UeDcxPdcpSpecificUser<C>::ReceiveDcPdcpPdu(UeDataParams params)
{
  m_pdcp->DoReceiveDcPdcpPdu(params);
}

/////////////////////////////////////////////
template <class C>
class UeDcxRlcSpecificUser : public UeDcxRlcUser
{
public:
  UeDcxRlcSpecificUser (C* rlc);

  // Inherited
  virtual void SendDcPdcpSdu (UeDataParams params);

private:
  UeDcxRlcSpecificUser ();
  C* m_rlc;
};

template <class C>
UeDcxRlcSpecificUser<C>::UeDcxRlcSpecificUser (C* rlc)
  : m_rlc (rlc)
{
}

template <class C>
UeDcxRlcSpecificUser<C>::UeDcxRlcSpecificUser ()
{
}

template <class C>
void
UeDcxRlcSpecificUser<C>::SendDcPdcpSdu(UeDataParams params)
{
  m_rlc->DoSendDcPdcpSdu(params);
}

} // namespace ns3

#endif // DALI_UE_DCX_SAP_H
