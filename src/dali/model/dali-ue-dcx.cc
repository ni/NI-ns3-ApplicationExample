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

#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/epc-gtpu-header.h"
#include "ns3/lte-pdcp-tag.h"

#include "ns3/dali-ue-dcx-header.h"
#include "ns3/lte-rrc-header.h"
#include "ns3/dali-ue-dcx.h"

#include "ns3/ni-logging.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UeDcx");

DcxIfaceInfo::DcxIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket, Ptr<Socket> localUserPlaneSocket)
{
  m_remoteIpAddr = remoteIpAddr;
  m_localCtrlPlaneSocket = localCtrlPlaneSocket;
  m_localUserPlaneSocket = localUserPlaneSocket;
}

DcxIfaceInfo::~DcxIfaceInfo (void)
{
  m_localCtrlPlaneSocket = 0;
  m_localUserPlaneSocket = 0;
}

DcxIfaceInfo&
DcxIfaceInfo::operator= (const DcxIfaceInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_remoteIpAddr = value.m_remoteIpAddr;
  m_localUserPlaneSocket = value.m_localUserPlaneSocket;
  return *this;
}

///////////////////////////////////////////

DcxUeInfo::DcxUeInfo (uint64_t localImsi, uint64_t remoteImsi)
{
  m_localImsi = localImsi;
  m_remoteImsi = remoteImsi;
}

DcxUeInfo::~DcxUeInfo (void)
{
  m_localImsi = 0;
  m_remoteImsi = 0;
}

DcxUeInfo&
DcxUeInfo::operator= (const DcxUeInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_localImsi = value.m_localImsi;
  m_remoteImsi = value.m_remoteImsi;
  return *this;
}

///////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (DaliUeDcx);

DaliUeDcx::DaliUeDcx ()
   : m_dcxcUdpPort (44444),
     m_dcxuUdpPort (22152)
{
  NS_LOG_FUNCTION (this);

  m_dcxSapProvider = new UeDcxSpecificUeDcxSapProvider<DaliUeDcx> (this);
  m_dcxPdcpProvider = new UeDcxPdcpSpecificProvider<DaliUeDcx> (this);
  m_dcxRlcProvider = new UeDcxRlcSpecificProvider<DaliUeDcx> (this);
}

DaliUeDcx::~DaliUeDcx ()
{
  NS_LOG_FUNCTION (this);
}

void
DaliUeDcx::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_dcxInterfaceSockets.clear ();
  m_dcxInterfaceImsis.clear ();
  m_dcxRlcUserMap.clear ();
  m_dcxPdcpUserMap.clear ();
  delete m_dcxSapProvider;
  delete m_dcxRlcProvider;
  delete m_dcxPdcpProvider;
}

TypeId
DaliUeDcx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DaliUeDcx")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&DaliUeDcx::m_rxPdu),
                     "ns3::DaliUeDcx::ReceiveTracedCallback");
  return tid;
}

void
DaliUeDcx::SetUeDcxSapUser (UeDcxSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_dcxSapUser = s;
}

UeDcxSapProvider*
DaliUeDcx::GetUeDcxSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_dcxSapProvider;
}

// Get and Set interfaces with PDCP and RLC
UeDcxPdcpProvider*
DaliUeDcx::GetUeDcxPdcpProvider ()
{
  NS_LOG_FUNCTION(this);
  return m_dcxPdcpProvider;
}

UeDcxRlcProvider*
DaliUeDcx::GetUeDcxRlcProvider ()
{
  return m_dcxRlcProvider;
}

void
DaliUeDcx::SetDcUeDcxRlcUser (uint32_t teid, UeDcxRlcUser* rlcUser)
{
  NS_LOG_INFO("Add UeDcxRlcUser for teid " << teid);
  m_dcxRlcUserMap[teid] = rlcUser;
}

void
DaliUeDcx::SetDcUeDcxPdcpUser (uint32_t teid, UeDcxPdcpUser* pdcpUser)
{
  NS_LOG_INFO("Add UeDcxPdcpUser for teid " << teid);
  m_dcxPdcpUserMap[teid] = pdcpUser;
}

// Add DCX endpoint
void
DaliUeDcx::AddDcxInterface (uint64_t localImsi, Ipv4Address localDcxAddress, uint64_t remoteImsi, Ipv4Address remoteDcxAddress)
{
  NS_LOG_FUNCTION (this << localImsi << localDcxAddress << remoteImsi << remoteDcxAddress);

  int retval;

  // Get local UE where this DCX entity belongs to
  Ptr<Node> localUE = GetObject<Node> ();

  // Create DCX-C socket for the local UE
  Ptr<Socket> localDcxcSocket = Socket::CreateSocket (localUE, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = localDcxcSocket->Bind (InetSocketAddress (localDcxAddress, m_dcxcUdpPort));
  NS_ASSERT (retval == 0);
  localDcxcSocket->SetRecvCallback (MakeCallback (&DaliUeDcx::RecvFromDcxcSocket, this));

  // Create DCX-U socket for the local UE
  Ptr<Socket> localDcxuSocket = Socket::CreateSocket (localUE, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = localDcxuSocket->Bind (InetSocketAddress (localDcxAddress, m_dcxuUdpPort));
  NS_ASSERT (retval == 0);
  localDcxuSocket->SetRecvCallback (MakeCallback (&DaliUeDcx::RecvFromDcxuSocket, this));


  NS_ASSERT_MSG (m_dcxInterfaceSockets.find (remoteImsi) == m_dcxInterfaceSockets.end (),
                 "Mapping for remoteImsi = " << remoteImsi << " is already known");
  m_dcxInterfaceSockets [remoteImsi] = Create<DcxIfaceInfo> (remoteDcxAddress, localDcxcSocket, localDcxuSocket);

  NS_ASSERT_MSG (m_dcxInterfaceImsis.find (localDcxcSocket) == m_dcxInterfaceImsis.end (),
                 "Mapping for control plane localSocket = " << localDcxcSocket << " is already known");
  m_dcxInterfaceImsis [localDcxcSocket] = Create<DcxUeInfo> (localImsi, remoteImsi);

  NS_ASSERT_MSG (m_dcxInterfaceImsis.find (localDcxuSocket) == m_dcxInterfaceImsis.end (),
                 "Mapping for data plane localSocket = " << localDcxuSocket << " is already known");
  m_dcxInterfaceImsis [localDcxuSocket] = Create<DcxUeInfo> (localImsi, remoteImsi);
}

void
DaliUeDcx::DoAddTeidToBeForwarded(uint32_t gtpTeid, uint64_t targetImsi)
{
  NS_LOG_FUNCTION(this << " add an entry to the map of teids to be forwarded: teid " << gtpTeid << " targetImsi " << targetImsi);
  NS_ASSERT_MSG(m_teidToBeForwardedMap.find(gtpTeid) == m_teidToBeForwardedMap.end(), "TEID already in the map");
  m_teidToBeForwardedMap.insert(std::pair<uint32_t, uint16_t> (gtpTeid, targetImsi));
}

void 
DaliUeDcx::DoRemoveTeidToBeForwarded(uint32_t gtpTeid)
{
  NS_LOG_FUNCTION(this << " remove and entry from the map of teids to be forwarded: teid " << gtpTeid);
  NS_ASSERT_MSG(m_teidToBeForwardedMap.find(gtpTeid) != m_teidToBeForwardedMap.end(), "TEID not in the map");
  m_teidToBeForwardedMap.erase(m_teidToBeForwardedMap.find(gtpTeid));
}

void 
DaliUeDcx::RecvFromDcxcSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv DCX message: from Socket");
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NS_ASSERT_MSG (m_dcxInterfaceImsis.find (socket) != m_dcxInterfaceImsis.end (),
                 "Missing infos of local and remote Imsi");
  Ptr<DcxUeInfo> uesInfo = m_dcxInterfaceImsis [socket];

  m_rxPdu(uesInfo->m_remoteImsi, uesInfo->m_localImsi, packet->GetSize (), 0);

  uint32_t PacketSize = packet->GetSize ();
  if (PacketSize > 1)
    {
      NS_LOG_LOGIC ("Recv DCX message: RRC DC CONNECTION RECONFIGURATION");

      RrcConnectionReconfigurationHeader dcxConnReconfHeader;
      packet->RemoveHeader (dcxConnReconfHeader);

      NS_LOG_INFO ("DCX Dual Connectivity Connection Reconfiguration header: " << dcxConnReconfHeader);

      LteRrcSap::RrcConnectionReconfiguration msg = dcxConnReconfHeader.GetMessage();

      m_dcxSapUser->RecvRrcDcConnectionReconfiguration (uesInfo->m_remoteImsi, uesInfo->m_localImsi, msg);

    }
  else
    {
	  RrcConnectionReconfigurationCompleteHeader dcxConnReconfCompHeader;
	  packet->RemoveHeader (dcxConnReconfCompHeader);

	  NS_LOG_INFO ("DCX Dual Connectivity Connection Reconfiguration Completed header: " << dcxConnReconfCompHeader);

	  LteRrcSap::RrcConnectionReconfigurationCompleted msg = dcxConnReconfCompHeader.GetMessage();

	  m_dcxSapUser->RecvRrcDcConnectionReconfigurationCompleted (uesInfo->m_remoteImsi, uesInfo->m_localImsi, msg);
    }
}

void
DaliUeDcx::RecvFromDcxuSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv UE DATA through DCX interface from Socket");
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NS_ASSERT_MSG (m_dcxInterfaceImsis.find (socket) != m_dcxInterfaceImsis.end (),
                 "Missing infos of local and remote Imsi");
  Ptr<DcxUeInfo> uesInfo = m_dcxInterfaceImsis [socket];

  NS_LOG_INFO("localImsi = " << uesInfo->m_localImsi);
  NS_LOG_INFO("remoteImsi = " << uesInfo->m_remoteImsi);

  m_rxPdu(uesInfo->m_remoteImsi, uesInfo->m_localImsi, packet->GetSize (), 1);

  GtpuHeader gtpu;
  packet->RemoveHeader (gtpu);

  NS_LOG_LOGIC ("GTP-U header: " << gtpu);

  UeDcxSapUser::UeDataParams params;
  params.sourceImsi = uesInfo->m_remoteImsi;
  params.targetImsi = uesInfo->m_localImsi;
  params.gtpTeid = gtpu.GetTeid ();
  params.ueData = packet;

  NS_LOG_LOGIC("Received packet on DCX, size " << packet->GetSize()
    << " source " << params.sourceImsi << " target " << params.targetImsi << " type " << gtpu.GetMessageType());

  if(m_teidToBeForwardedMap.find(params.gtpTeid) == m_teidToBeForwardedMap.end())
  {
    if(gtpu.GetMessageType() == DaliUeDcxHeader::DcForwardDownlinkData)
    {
      // add PdcpTag
      PdcpTag pdcpTag (Simulator::Now ());
      params.ueData->AddByteTag (pdcpTag);
      // call rlc interface
      UeDcxRlcUser* user = m_dcxRlcUserMap.find(params.gtpTeid)->second;
      if(user != 0)
      {
        user -> SendDcPdcpSdu(params);
      }
      else
      {
        NS_LOG_INFO("Not implemented: Forward to the other UE");
      }
    } 
    else if (gtpu.GetMessageType() == DaliUeDcxHeader::DcForwardUplinkData)
    {
      // call pdcp interface
      NS_LOG_INFO("Call PDCP interface");
      m_dcxPdcpUserMap[params.gtpTeid] -> ReceiveDcPdcpPdu(params);
    }
    else
    {
      m_dcxSapUser->RecvUeData (params);
    }
  }
  else // the packet was received during a secondary cell HO, forward to the target cell
  {
	  NS_LOG_INFO("There is no map to forward Packet");
  }
}

//
// Implementation of the DCX SAP Provider
//

void
DaliUeDcx::DoSendRrcDcConnectionReconfiguration (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfiguration msg)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceImsi = " << sourceImsi);
  NS_LOG_LOGIC ("targetImsi = " << targetImsi);

  NS_ASSERT_MSG (m_dcxInterfaceSockets.find (targetImsi) != m_dcxInterfaceSockets.end (),
                 "Missing infos for targetImsi = " << targetImsi);
  Ptr<DcxIfaceInfo> socketInfo = m_dcxInterfaceSockets [targetImsi];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send DCX message: RRC DC CONNECTION RECONFIGURATION");

  // Build the DCX message
  RrcConnectionReconfigurationHeader dcxRrcConnConfHeader;
  dcxRrcConnConfHeader.SetMessage(msg);

//  NS_LOG_INFO ("DCX header: " << dcxHeader);
  NS_LOG_INFO ("DCX RrcDcConnectionReconfiguration header: " << dcxRrcConnConfHeader);

  // Build the DCX packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (dcxRrcConnConfHeader);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the DCX message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_dcxcUdpPort));
}

void
DaliUeDcx::DoSendRrcDcConnectionReconfigurationCompleted (uint64_t sourceImsi, uint64_t targetImsi, LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceImsi = " << sourceImsi);
  NS_LOG_LOGIC ("targetImsi = " << targetImsi);

  NS_ASSERT_MSG (m_dcxInterfaceSockets.find (targetImsi) != m_dcxInterfaceSockets.end (),
                 "Missing infos for targetImsi = " << targetImsi);
  Ptr<DcxIfaceInfo> socketInfo = m_dcxInterfaceSockets [targetImsi];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send DCX message: RRC DC CONNECTION RECONFIGURATION COMPLETED");

  // Build the DCX message
  RrcConnectionReconfigurationCompleteHeader dcxRrcConnConfCompHeader;
  dcxRrcConnConfCompHeader.SetMessage(msg);

//  NS_LOG_INFO ("dcx header: " << dcxHeader);
  NS_LOG_INFO ("DCX RrcDcConnectionReconfigurationCompleted header: " << dcxRrcConnConfCompHeader);

  // Build the DCX packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (dcxRrcConnConfCompHeader);


  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the DCX message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_dcxcUdpPort));
}


void
DaliUeDcx::DoSendUeData (UeDcxSapProvider::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceImsi = " << params.sourceImsi);
  NS_LOG_LOGIC ("targetImsi = " << params.targetImsi);
  NS_LOG_LOGIC ("gtpTeid = " << params.gtpTeid);

  NS_ASSERT_MSG (m_dcxInterfaceSockets.find (params.targetImsi) != m_dcxInterfaceSockets.end (),
                 "Missing infos for targetImsi = " << params.targetImsi);
  Ptr<DcxIfaceInfo> socketInfo = m_dcxInterfaceSockets [params.targetImsi];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  GtpuHeader gtpu;
  gtpu.SetTeid (params.gtpTeid);
  gtpu.SetLength (params.ueData->GetSize () + gtpu.GetSerializedSize () - 8); /// \todo This should be done in GtpuHeader
  NS_LOG_INFO ("GTP-U header: " << gtpu);

  Ptr<Packet> packet = params.ueData;
  packet->AddHeader (gtpu);

  NS_LOG_INFO ("Forward UE DATA through DCX interface");
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_dcxuUdpPort));
}

void
DaliUeDcx::DoSendDcPdcpPdu(DaliUeDcxSap::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceImsi = " << params.sourceImsi);
  NS_LOG_LOGIC ("targetImsi = " << params.targetImsi);
  NS_LOG_LOGIC ("gtpTeid = " << params.gtpTeid);

  NS_ASSERT_MSG (m_dcxInterfaceSockets.find (params.targetImsi) != m_dcxInterfaceSockets.end (),
                 "Missing infos for targetImsi = " << params.targetImsi);
  Ptr<DcxIfaceInfo> socketInfo = m_dcxInterfaceSockets [params.targetImsi];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  // add a message type to the gtpu header, so that it is possible to distinguish at receiver
  GtpuHeader gtpu;
  gtpu.SetTeid (params.gtpTeid);
  gtpu.SetMessageType(DaliUeDcxHeader::DcForwardDownlinkData);
  gtpu.SetLength (params.ueData->GetSize () + gtpu.GetSerializedSize () - 8); /// \todo This should be done in GtpuHeader
  NS_LOG_INFO ("GTP-U header: " << gtpu);

  Ptr<Packet> packet = params.ueData;
  packet->AddHeader (gtpu);

  NS_LOG_INFO ("Forward DC UE DATA through DCX interface");
  NI_LOG_CONSOLE_DEBUG("LTE.UE.DCX: Forward DC UE DATA through DCX interface");
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_dcxuUdpPort));
}

void
DaliUeDcx::DoReceiveDcPdcpSdu(DaliUeDcxSap::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceImsi = " << params.sourceImsi);
  NS_LOG_LOGIC ("targetImsi = " << params.targetImsi);
  NS_LOG_LOGIC ("gtpTeid = " << params.gtpTeid);

  NS_ASSERT_MSG (m_dcxInterfaceSockets.find (params.targetImsi) != m_dcxInterfaceSockets.end (),
                 "Missing infos for targetImsi = " << params.targetImsi);
  Ptr<DcxIfaceInfo> socketInfo = m_dcxInterfaceSockets [params.targetImsi];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  // add a message type to the gtpu header, so that it is possible to distinguish at receiver
  GtpuHeader gtpu;
  gtpu.SetTeid (params.gtpTeid);
  gtpu.SetMessageType(DaliUeDcxHeader::DcForwardUplinkData);
  gtpu.SetLength (params.ueData->GetSize () + gtpu.GetSerializedSize () - 8); /// \todo This should be done in GtpuHeader
  NS_LOG_INFO ("GTP-U header: " << gtpu);

  Ptr<Packet> packet = params.ueData;
  packet->AddHeader (gtpu);

  NS_LOG_INFO ("Forward DC UE DATA through DCX interface");
  NI_LOG_CONSOLE_DEBUG("LTE.UE.DCX: Forward DC UE DATA through DCX interface");
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_dcxuUdpPort));
}

} // namespace ns3
