/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Inspired by epc-x2
 */

#ifndef DALI_UE_DCX_H
#define DALI_UE_DCX_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include "ns3/dali-ue-dcx-sap.h"

#include <map>

namespace ns3 {


/**
 * DcxIfaceInfo
 */
class DcxIfaceInfo : public SimpleRefCount<DcxIfaceInfo>
{
public:
  /**
   * Constructor
   *
   * \param remoteIpAddr remote IP address
   * \param localCtrlPlaneSocket control plane socket
   * \param localUserPlaneSocket user plane socket
   */
  DcxIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket, Ptr<Socket> localUserPlaneSocket);
  virtual ~DcxIfaceInfo (void);

  /**
   * Assignment operator
   *
   * \returns DcxIfaceInfo&
   */
  DcxIfaceInfo& operator= (const DcxIfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr; ///< remote IP address
  Ptr<Socket>   m_localCtrlPlaneSocket; ///< local control plane socket
  Ptr<Socket>   m_localUserPlaneSocket; ///< local user plane socket
};


/**
 * DcxUeInfo
 */
class DcxUeInfo : public SimpleRefCount<DcxUeInfo>
{
public:
  /**
   * Constructor
   *
   * \param localImsi local UE Imsi
   * \param remoteImsi remote UE Imsi
   */
  DcxUeInfo (uint64_t localImsi, uint64_t remoteImsi);
  virtual ~DcxUeInfo (void);

  /**
   * Assignment operator
   *
   * \returns DcxUeInfo&
   */
  DcxUeInfo& operator= (const DcxUeInfo &);

public:
  uint64_t m_localImsi; ///< local UE Imsi
  uint64_t m_remoteImsi; ///< remote UE Imsi
};


/**
 * \ingroup lte
 *
 * This entity is installed inside an UE and provides the functionality for the DCX interface
 */
class DaliUeDcx : public Object
{
  friend class UeDcxSpecificUeDcxSapProvider<DaliUeDcx>;
  friend class UeDcxPdcpSpecificProvider<DaliUeDcx>;
  friend class UeDcxRlcSpecificProvider<DaliUeDcx>;

public:
  /**
   * Constructor
   */
  DaliUeDcx ();

  /**
   * Destructor
   */
  virtual ~DaliUeDcx (void);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the Dcx SAP User to be used by this UE DCX entity
   */
  void SetUeDcxSapUser (UeDcxSapUser * s);

  /**
   * \return the DCX SAP Provider interface offered by this UE DCX entity
   */
  UeDcxSapProvider* GetUeDcxSapProvider ();

  /**
   * \return the DCX Pdcp Provider interface offered by this UE DCX entity
   */
  UeDcxPdcpProvider* GetUeDcxPdcpProvider ();

  /**
   * \return the DCX Rlc Provider interface offered by this UE DCX entity
   */
  UeDcxRlcProvider* GetUeDcxRlcProvider ();

  /**
   * \param the teid of the DC device
   * \param the DCX Rlc User interface associated to the teid
   */
  void SetDcUeDcxRlcUser (uint32_t teid, UeDcxRlcUser* rlcUser);

  /**
   * \param the teid of the DC device
   * \param the DCX Pdcp User interface associated to the teid
   */
  void SetDcUeDcxPdcpUser (uint32_t teid, UeDcxPdcpUser* pdcpUser);


  /**
   * Add an DCX interface to this UE DCX entity
   * \param ue1Imsi the Imsi of the current UE
   * \param ue1DcxAddress the address of the current UE
   * \param ue2Imsi the Imsi of the neighbouring UE
   * \param ue2DcxAddress the address of the neighbouring UE
   */
  void AddDcxInterface (uint64_t ue1Imsi, Ipv4Address ue1DcxAddress,
                       uint64_t ue2Imsi, Ipv4Address ue2DcxAddress);

  /**
   * Method to be assigned to the recv callback of the DCX-C (Control Plane) socket.
   * It is called when the UE receives a packet from the peer UE of the DCX-C interface
   *
   * \param socket socket of the DCX-C interface
   */
  void RecvFromDcxcSocket (Ptr<Socket> socket);

  /**
   * Method to be assigned to the recv callback of the DCX-U (User Plane) socket.
   * It is called when the UE receives a packet from the peer UE of the DCX-U interface
   *
   * \param socket socket of the DCX-U interface
   */
  void RecvFromDcxuSocket (Ptr<Socket> socket);

  /**
   * TracedCallback signature for
   *
   * \param [in] source
   * \param [in] target
   * \param [in] bytes The packet size.
   */
  typedef void (* ReceiveTracedCallback)
    (uint64_t sourceImsi, uint64_t targetImsi, uint32_t bytes, bool data);

protected:
  // Interface provided by UeDcxSapProvider

  /**
   * Send DALI Dual Connectivity Connection Reconfiguration message
   * \param msg the message for DALI DC sUE configuration procedure
   */
  virtual void DoSendRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg);
  virtual void DoSendRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg);

  /**
   * Send UE data function
   *
   * \param params UeDcxSapProvider::UeDataParams
   */
  virtual void DoSendUeData (UeDcxSapProvider::UeDataParams params);
  virtual void DoSendDcPdcpPdu (UeDcxSapProvider::UeDataParams params);
  virtual void DoReceiveDcPdcpSdu (UeDcxSapProvider::UeDataParams params);

  // these methods are not used to send messages but to change the internal state of the UeDcx
  virtual void DoAddTeidToBeForwarded(uint32_t teid, uint64_t targetImsi);
  virtual void DoRemoveTeidToBeForwarded(uint32_t teid);

  UeDcxSapUser* m_dcxSapUser;
  UeDcxSapProvider* m_dcxSapProvider;

  /**
   * Map the PdcpUser to a certain teid
   */
  std::map < uint32_t, UeDcxPdcpUser* > m_dcxPdcpUserMap;
  // The PdcpProvider offered by this DCX interface
  UeDcxPdcpProvider* m_dcxPdcpProvider;
  /**
   * Map the RlcUser to a certain teid
   */
  std::map < uint32_t, UeDcxRlcUser* > m_dcxRlcUserMap;
  // The RlcProvider offered by this DCX interface
  UeDcxRlcProvider* m_dcxRlcProvider;

private:

  /**
   * Map the targetImsi to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the DCX message
   */
  std::map < uint16_t, Ptr<DcxIfaceInfo> > m_dcxInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the Dcx message)
   * to the corresponding (sourceImsi, targetImsi) associated with the DCX interface
   */
  std::map < Ptr<Socket>, Ptr<DcxUeInfo> > m_dcxInterfaceImsis;

  /**
   * UDP port to be used for the DCX interface: DCX-C and DCX-U
   */
  uint16_t m_dcxcUdpPort;
  uint16_t m_dcxuUdpPort;

  TracedCallback<uint64_t, uint64_t, uint32_t, bool> m_rxPdu;

  /**
   * Map the gtpTeid to the targetImsi to which the packet should be forwarded
   * during a secondary cell dual connectivity
   */
  std::map <uint32_t, uint16_t> m_teidToBeForwardedMap;

};

} //namespace ns3

#endif // DALI_UE_DCX_H
