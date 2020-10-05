/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 UOC-Universitat Oberta de Catalunya
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
 *
 */

#ifndef LWIP_IPSEC_H
#define LWIP_IPSEC_H

#include <iostream>
#include "ns3/internet-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/network-module.h"
#include "ns3/virtual-net-device.h"

namespace ns3 {

  // tunnel class for lwip ipsec emulation
  class Tunnel
  {
    Ptr<Socket> m_n3Socket;
    Ptr<Socket> m_n1Socket;
    Ipv4Address m_n3Address;
    Ipv4Address m_n1Address;
    Ptr<UniformRandomVariable> m_rng;
    Ptr<VirtualNetDevice> m_n1Tap;
    Ptr<VirtualNetDevice> m_n3Tap;

    bool
    N1VirtualSend (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
    {
      m_n1Socket->SendTo (packet, 0, InetSocketAddress (m_n3Address, 667));
      return true;
    }

    bool
    N3VirtualSend (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
    {
      m_n3Socket->SendTo (packet, 0, InetSocketAddress (m_n1Address, 667));
      return true;
    }

    void N3SocketRecv (Ptr<Socket> socket)
    {
      Ptr<Packet> packet = socket->Recv (65535, 0);
      m_n3Tap->Receive (packet, 0x0800, m_n3Tap->GetAddress (), m_n3Tap->GetAddress (), NetDevice::PACKET_HOST);
    }

    void N1SocketRecv (Ptr<Socket> socket)
    {
      Ptr<Packet> packet = socket->Recv (65535, 0);
      m_n1Tap->Receive (packet, 0x0800, m_n1Tap->GetAddress (), m_n1Tap->GetAddress (), NetDevice::PACKET_HOST);
    }

  public:

    Tunnel (Ptr<Node> n3, Ptr<Node> n1,
            Ipv4Address n3Addr, Ipv4Address n1Addr)
  : m_n3Address (n3Addr), m_n1Address (n1Addr)
  {
      m_rng = CreateObject<UniformRandomVariable> ();
      m_n3Socket = Socket::CreateSocket (n3, TypeId::LookupByName ("ns3::UdpSocketFactory"));
      m_n3Socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 667));
      m_n3Socket->SetRecvCallback (MakeCallback (&Tunnel::N3SocketRecv, this));

      m_n1Socket = Socket::CreateSocket (n1, TypeId::LookupByName ("ns3::UdpSocketFactory"));
      m_n1Socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 667));
      m_n1Socket->SetRecvCallback (MakeCallback (&Tunnel::N1SocketRecv, this));

      // n1 tap device
      m_n1Tap = CreateObject<VirtualNetDevice> ();
      m_n1Tap->SetAddress (Mac48Address ("11:00:01:02:03:02"));
      m_n1Tap->SetSendCallback (MakeCallback (&Tunnel::N1VirtualSend, this));
      n1->AddDevice (m_n1Tap);
      Ptr<Ipv4> ipv4 = n1->GetObject<Ipv4> ();
      uint32_t i = ipv4->AddInterface (m_n1Tap);
      ipv4->AddAddress (i, Ipv4InterfaceAddress (Ipv4Address ("11.0.0.254"), Ipv4Mask ("255.255.255.0")));
      ipv4->SetUp (i);

      // n3 tap device
      m_n3Tap = CreateObject<VirtualNetDevice> ();
      m_n3Tap->SetAddress (Mac48Address ("11:00:01:02:03:04"));
      m_n3Tap->SetSendCallback (MakeCallback (&Tunnel::N3VirtualSend, this));
      n3->AddDevice (m_n3Tap);
      ipv4 = n3->GetObject<Ipv4> ();
      i = ipv4->AddInterface (m_n3Tap);
      ipv4->AddAddress (i, Ipv4InterfaceAddress (Ipv4Address ("11.0.0.1"), Ipv4Mask ("255.255.255.0")));
      ipv4->SetUp (i);
  }

  };
  //**********************************************************************************************
} // namespace ns3

#endif /* LWIP_IPSEC_H */
