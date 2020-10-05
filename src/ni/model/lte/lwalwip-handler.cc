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
 * Author: Shahwaiz Afaqui <mafaqui@uoc.edu>
 */
#include "ns3/config.h"
#include "ns3/seq-ts-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/pdcp-header.h"
#include "ns3/pdcp-lcid.h"

#include "ns3/ni-logging.h"

#include "lwa-tag.h"
#include "lwip-tag.h"
#include "lwa-header.h"
#include "lwip-header.h"
#include "lwalwip-header.h"

#include "lwalwip-handler.h"


namespace ns3 {

//  NS_LOG_COMPONENT_DEFINE ("LwaLwipHandler");
//
//  NS_OBJECT_ENSURE_REGISTERED (LwaLwipHandler);

  LwaLwipHandler::LwaLwipHandler (Ptr<Socket> lwaapTxSocket,
                                  Ptr<Socket> lwipepTxSocket,
                                  bool niApiEnableTapBridge,
                                  Ipv4Address wifiSta1IpAddr)
  {
    m_lwaapTxSocket = lwaapTxSocket;
    m_lwipepTxSocket = lwipepTxSocket;
    m_niApiEnableTapBridge = niApiEnableTapBridge;
    m_wifiSta1IpAddr = wifiSta1IpAddr;
  }

  LwaLwipHandler::~LwaLwipHandler ()
  {
    // TODO Auto-generated destructor stub
  }

  void LwaLwipHandler::LtePdcpLwaLwipHandler (Ptr< const Packet> p)
  {

    bool clientServerAppEnabled = true; // false for tapbridge
    clientServerAppEnabled = !m_niApiEnableTapBridge; // copy from global value

    uint32_t currentPacketSeqNum=0;
    SeqTsHeader seqTs;

    // copy incoming packet
    Ptr<Packet> currentPacket = p->Copy();

    // remove headers to get sequence number and include lwa/lwip header
    // further this (internal) header removal is needed if packets going through the tapbridge
    PdcpHeader pdcpHeader;
    currentPacket->RemoveHeader (pdcpHeader);
    Ipv4Header ipHeader;
    currentPacket->RemoveHeader (ipHeader);
    UdpHeader udpHeader;
    currentPacket->RemoveHeader (udpHeader);
    // remove sequence number and add UDP/IP/PDCP only for Client-Server application, NOT for tapbridge
    if (clientServerAppEnabled)
      {
        // remove sequence number
        currentPacket->RemoveHeader (seqTs);
        currentPacketSeqNum=seqTs.GetSeq ();
        // add headers again, without sequence number in front
        currentPacket->AddHeader (udpHeader);
        currentPacket->AddHeader (ipHeader);
        currentPacket->AddHeader (pdcpHeader);
      }

    // read lwa/lwip tags added by pdcp layer and add lwa/lwip information
    // as separate header to data packets (won't be removed when transmitting
    // packets via API)
    LwaTag   lwaTag;
    LwipTag  lwipTag;
    PdcpLcid lcidTag;
    uint32_t lcid=0;
    uint32_t bid=0;
    double   lwaMode=0;
    LwaLwipHeader lwaLwipHeader;

    // tag to copy the logical channel id of the frame
    if(currentPacket->FindFirstMatchingByteTag (lcidTag)){
        lcid = lcidTag.Get();
        bid = lcid - 2;
    }
    lwaLwipHeader.SetBearerId (bid);

    // packet handling for lwa packets
    if (currentPacket->PeekPacketTag(lwaTag)){
        // copy additional headers only for Client-Server application, NOT for tapbridge
        if (clientServerAppEnabled)
          {
            //copy the status of LWA activation (LTE, Split, swtiched)
            lwaLwipHeader.SetLwaActivate (lwaTag.Get());
            // add lwa lwip header
            currentPacket->AddHeader (lwaLwipHeader);
            // add current sequence number
            seqTs.SetSeq (currentPacketSeqNum);
            currentPacket->AddHeader (seqTs);
          }
        // send packet over the lwaap socket
        //lwaapTxSocket->Send(currentPacket); // send packet using default port
        m_lwaapTxSocket->SendTo (currentPacket, 0, InetSocketAddress (m_wifiSta1IpAddr, udpHeader.GetDestinationPort())); // send packet using dest port from UDP header (post agnostic transmission)
        NI_LOG_CONSOLE_DEBUG ("LWA: Sent packet with Sequence Number " << currentPacketSeqNum);
    }
    // packet handling for lwip packets
    if (currentPacket->PeekPacketTag (lwipTag)){
        // copy additional headers only for Client-Server application, NOT for tapbridge
        if (clientServerAppEnabled)
          {
            //copy the status of LWA activation (LTE, Split, swtiched)
            lwaLwipHeader.SetLwipActivate (lwipTag.Get());
            // add lwa lwip header
            currentPacket->AddHeader (lwaLwipHeader);
            // add current sequence number
            seqTs.SetSeq (currentPacketSeqNum);
            currentPacket->AddHeader (seqTs);
          }
        // send packet over the lwipep socket
        // lwipepTxSocket->Send(currentPacket); // send packet using default port
        m_lwipepTxSocket->SendTo (currentPacket, 0, InetSocketAddress (m_wifiSta1IpAddr, udpHeader.GetDestinationPort())); // send packet using dest port from UDP header (post agnostic transmission)

        NI_LOG_CONSOLE_DEBUG ("LWIP: Sent packet with Sequence Number " << currentPacketSeqNum);
    }
  }

  // assign callback function to the pdcp object
  // NOTE: object is created after bearer release which is after attach procedure
  void
  LwaLwipHandler::Callback_LtePDCPTX (void)
  {
    std::string path = "/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LtePdcp/TxPDUtrace";
    Config::ConnectWithoutContext (path, MakeCallback (&LwaLwipHandler::LtePdcpLwaLwipHandler, this));
  }

} // namespace ns3
