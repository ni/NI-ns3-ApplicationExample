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


#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"
#include <cstdlib>
#include <cstdio>

#include "ns3/ni.h"
#include "ni-udp-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NiUdpClient");

NS_OBJECT_ENSURE_REGISTERED (NiUdpClient);

TypeId
NiUdpClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NiUdpClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<NiUdpClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&NiUdpClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets", TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&NiUdpClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&NiUdpClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&NiUdpClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&NiUdpClient::m_size),
                   MakeUintegerChecker<uint32_t> (12,1500))
  ;
  return tid;
}

NiUdpClient::NiUdpClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
}

NiUdpClient::~NiUdpClient ()
{
  NS_LOG_FUNCTION (this);
}

void
NiUdpClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
NiUdpClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
NiUdpClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
NiUdpClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind6 ();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind6 ();
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetAllowBroadcast (true);
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &NiUdpClient::Send, this);
}

void
NiUdpClient::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
NiUdpClient::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());

  // 8+4 : the size of the seqTs header
  uint32_t pktSize = m_size-(8+4);

  // create packet for destination address #1
  Ptr<Packet> pktSend;
  if (pktSize > 32)
    {
      // include real payload in the packet
      uint8_t buffer[pktSize];
      for (uint32_t i = 0; i < pktSize; i++) buffer[i] = rand() % 0xff;
      pktSend = Create<Packet> (buffer  , pktSize );

    }
  else
    {
      // send zero filled packet
      pktSend = Create<Packet> (pktSize);
    }

  // create and add sequence header
  SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);
  pktSend->AddHeader (seqTs);

  // get destination ip adress
  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }

  // send packet to destination address
  if ((m_socket->Send (pktSend)) >= 0)
    {
      ++m_sent;
      NI_LOG_CONSOLE_DEBUG ("NI.CLIENT: sent " << m_size
                            << " bytes to " << peerAddressStringStream.str ()
                            << " Uid: " << pktSend->GetUid ()
                            << " Sequence number: " << seqTs.GetSeq ());
    }
  else
    {
      NI_LOG_CONSOLE_DEBUG ("Error while sending " << m_size << " bytes to "
                   << peerAddressStringStream.str ());
    }

  if (m_sent < m_count)
    {
      m_sendEvent = Simulator::Schedule (m_interval, &NiUdpClient::Send, this);
    }
}

} // Namespace ns3
