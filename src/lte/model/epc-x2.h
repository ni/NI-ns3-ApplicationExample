/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, 2018, University of Padova, Dep. of Information Engineering, SIGNET lab
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

#ifndef EPC_X2_H
#define EPC_X2_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

#include "ns3/epc-x2-sap.h"

#include <map>

namespace ns3 {


/**
 * X2IfaceInfo
 */
class X2IfaceInfo : public SimpleRefCount<X2IfaceInfo>
{
public:
  /**
   * Constructor
   *
   * \param remoteIpAddr remote IP address
   * \param localCtrlPlaneSocket control plane socket
   * \param localUserPlaneSocket user plane socket
   */
  X2IfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket, Ptr<Socket> localUserPlaneSocket);
  virtual ~X2IfaceInfo (void);

  /**
   * Assignment operator
   *
   * \returns X2IfaceInfo&
   */
  X2IfaceInfo& operator= (const X2IfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr;
  Ptr<Socket>   m_localCtrlPlaneSocket;
  Ptr<Socket>   m_localUserPlaneSocket;
};


/**
 * X2CellInfo
 */
class X2CellInfo : public SimpleRefCount<X2CellInfo>
{
public:
  /**
   * Constructor
   *
   * \param localCellId local cell ID
   * \param remoteCellId remote cell ID
   */
  X2CellInfo (uint16_t localCellId, uint16_t remoteCellId);
  virtual ~X2CellInfo (void);

  /**
   * Assignment operator
   *
   * \returns X2CellInfo&
   */
  X2CellInfo& operator= (const X2CellInfo &);

public:
  uint16_t m_localCellId;
  uint16_t m_remoteCellId;
};


/**
 * \ingroup lte
 *
 * This entity is installed inside an eNB and provides the functionality for the X2 interface
 */
class EpcX2 : public Object
{
  friend class EpcX2SpecificEpcX2SapProvider<EpcX2>;
  friend class EpcX2PdcpSpecificProvider<EpcX2>;
  friend class EpcX2RlcSpecificProvider<EpcX2>;

public:
  /** 
   * Constructor
   */
  EpcX2 ();

  /**
   * Destructor
   */
  virtual ~EpcX2 (void);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the X2 SAP User to be used by this EPC X2 entity
   */
  void SetEpcX2SapUser (EpcX2SapUser * s);

  /**
   * \return the X2 SAP Provider interface offered by this EPC X2 entity
   */
  EpcX2SapProvider* GetEpcX2SapProvider ();

  /**
   * \return the X2 Pdcp Provider interface offered by this EPC X2 entity
   */
  EpcX2PdcpProvider* GetEpcX2PdcpProvider ();

  /**
   * \return the X2 Rlc Provider interface offered by this EPC X2 entity
   */
  EpcX2RlcProvider* GetEpcX2RlcProvider ();

  /**
   * \param the teid of the DC device
   * \param the X2 Rlc User interface associated to the teid
   */
  void SetDcEpcX2RlcUser (uint32_t teid, EpcX2RlcUser* rlcUser);

  /**
   * \param the teid of the DC device
   * \param the X2 Pdcp User interface associated to the teid
   */
  void SetDcEpcX2PdcpUser (uint32_t teid, EpcX2PdcpUser* pdcpUser);

  /**
   * Add an X2 interface to this EPC X2 entity
   * \param enb1CellId the cell ID of the current eNodeB
   * \param enb1X2Address the address of the current eNodeB
   * \param enb2CellId the cell ID of the neighbouring eNodeB
   * \param enb2X2Address the address of the neighbouring eNodeB
   */
  void AddX2Interface (uint16_t enb1CellId, Ipv4Address enb1X2Address,
                       uint16_t enb2CellId, Ipv4Address enb2X2Address);


  /** 
   * Method to be assigned to the recv callback of the X2-C (X2 Control Plane) socket.
   * It is called when the eNB receives a packet from the peer eNB of the X2-C interface
   * 
   * \param socket socket of the X2-C interface
   */
  void RecvFromX2cSocket (Ptr<Socket> socket);

  /** 
   * Method to be assigned to the recv callback of the X2-U (X2 User Plane) socket.
   * It is called when the eNB receives a packet from the peer eNB of the X2-U interface
   * 
   * \param socket socket of the X2-U interface
   */
  void RecvFromX2uSocket (Ptr<Socket> socket);

  /**
   * TracedCallback signature for
   *
   * \param [in] source
   * \param [in] target
   * \param [in] bytes The packet size.
   * \param [in] delay Delay since sender timestamp, in ns.
   */
  typedef void (* ReceiveTracedCallback)
    (uint16_t sourceCellId, uint16_t targetCellId, uint32_t bytes, uint64_t delay, bool data);
    
protected:
  // Interface provided by EpcX2SapProvider
  /**
   * Send handover request function
   * \param params the send handover request parameters
   */
  virtual void DoSendHandoverRequest (EpcX2SapProvider::HandoverRequestParams params);
  virtual void DoSendRlcSetupRequest (EpcX2SapProvider::RlcSetupRequest params);
  virtual void DoSendRlcSetupCompleted (EpcX2SapProvider::UeDataParams);
  /**
   * Send handover request ack function
   * \param params the send handover request ack parameters
   */
  virtual void DoSendHandoverRequestAck (EpcX2SapProvider::HandoverRequestAckParams params);
  /**
   * Send handover preparation failure function
   * \param params the handover preparation failure parameters
   */
  virtual void DoSendHandoverPreparationFailure (EpcX2SapProvider::HandoverPreparationFailureParams params);
  /**
   * Send SN status transfer function
   * \param params the SN status transfer parameters
   */
  virtual void DoSendSnStatusTransfer (EpcX2SapProvider::SnStatusTransferParams params);
  /**
   * Send UE context release function
   * \param params the UE context release parameters
   */
  virtual void DoSendUeContextRelease (EpcX2SapProvider::UeContextReleaseParams params);
  /**
   * Send load information function
   * \param params the send load information parameters
   */
  virtual void DoSendLoadInformation (EpcX2SapProvider::LoadInformationParams params);
  /**
   * Send resource status update function
   * \param params the send resource status update parameters
   */
  virtual void DoSendResourceStatusUpdate (EpcX2SapProvider::ResourceStatusUpdateParams params);
  /**
   * Send UE data function
   *
   * \param params EpcX2SapProvider::UeDataParams
   */
  virtual void DoSendUeData (EpcX2SapProvider::UeDataParams params);
  virtual void DoSendDcPdcpPdu (EpcX2SapProvider::UeDataParams params);
  virtual void DoReceiveDcPdcpSdu (EpcX2SapProvider::UeDataParams params);
  
  EpcX2SapUser* m_x2SapUser;
  EpcX2SapProvider* m_x2SapProvider;

  /**
   * Map the PdcpUser to a certain teid
   */
  std::map < uint32_t, EpcX2PdcpUser* > m_x2PdcpUserMap;
  // The PdcpProvider offered by this X2 interface
  EpcX2PdcpProvider* m_x2PdcpProvider;
  /**
   * Map the RlcUser to a certain teid
   */
  std::map < uint32_t, EpcX2RlcUser* > m_x2RlcUserMap;
  // The RlcProvider offered by this X2 interface
  EpcX2RlcProvider* m_x2RlcProvider;

private:

  /**
   * Map the targetCellId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the X2 message
   */
  std::map < uint16_t, Ptr<X2IfaceInfo> > m_x2InterfaceSockets;

  /**
   * Map the localSocket (the one receiving the X2 message)
   * to the corresponding (sourceCellId, targetCellId) associated with the X2 interface
   */
  std::map < Ptr<Socket>, Ptr<X2CellInfo> > m_x2InterfaceCellIds;

  /**
   * UDP ports to be used for the X2 interfaces: X2-C and X2-U
   */
  uint16_t m_x2cUdpPort;
  uint16_t m_x2uUdpPort;

};

} //namespace ns3

#endif // EPC_X2_H
