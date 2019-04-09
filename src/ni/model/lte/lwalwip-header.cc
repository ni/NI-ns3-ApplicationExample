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

#include "lwalwip-header.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LwaLwipHeader");

NS_OBJECT_ENSURE_REGISTERED (LwaLwipHeader);

LwaLwipHeader::LwaLwipHeader ()
  : m_lwaactivate (0),
    m_lwipactivate (0),
    m_bearerid (0)
{
  NS_LOG_FUNCTION (this);
}

void
LwaLwipHeader::SetLwaActivate (uint32_t activate)
{
  NS_LOG_FUNCTION (this << activate);
  m_lwaactivate = activate;
}
uint32_t
LwaLwipHeader::GetLwaActivate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lwaactivate;
}

void
LwaLwipHeader::SetLwipActivate (uint32_t activate)
{
  NS_LOG_FUNCTION (this << activate);
  m_lwipactivate = activate;
}
uint32_t
LwaLwipHeader::GetLwipActivate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lwipactivate;
}

void
LwaLwipHeader::SetBearerId (uint32_t bearerid)
{
  NS_LOG_FUNCTION (this << bearerid);
  m_bearerid = bearerid;
}

uint32_t
LwaLwipHeader::GetBearerId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_bearerid;
}

TypeId
LwaLwipHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LwaLwipHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<LwaLwipHeader> ()
    //.SetParent<Header> ()
    //.AddConstructor<LwipHeader> ()
  ;
  return tid;
}
TypeId
LwaLwipHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
LwaLwipHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  //os << "(seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
  os << " LWA activate status " << m_lwaactivate;
  os << " LWIP activate status " << m_lwipactivate;
  os << " Bearer id of LWIP link " << m_bearerid;
}
uint32_t
LwaLwipHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 4;
}

void
LwaLwipHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_lwaactivate);
  i.WriteHtonU32 (m_lwipactivate);
  i.WriteHtonU32 (m_bearerid);
}
uint32_t
LwaLwipHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_lwaactivate = i.ReadNtohU32 ();
  m_lwipactivate = i.ReadNtohU32 ();
  m_bearerid = i.ReadNtohU32 ();
  return GetSerializedSize ();
}

} // namespace ns3
