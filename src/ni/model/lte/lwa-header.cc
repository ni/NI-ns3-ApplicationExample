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

#include "lwa-header.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LwaHeader");

NS_OBJECT_ENSURE_REGISTERED (LwaHeader);

LwaHeader::LwaHeader ()
  : m_lwaactivate (0),
    m_bearerid (0)
{
  NS_LOG_FUNCTION (this);
}

void
LwaHeader::SetLwaActivate (uint32_t activate)
{
  NS_LOG_FUNCTION (this << activate);
  m_lwaactivate = activate;
}
uint32_t
LwaHeader::GetLwaActivate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lwaactivate;
}

void
LwaHeader::SetBearerId (uint32_t bearerid)
{
  NS_LOG_FUNCTION (this << bearerid);
  m_bearerid = bearerid;
}

uint32_t
LwaHeader::GetBearerId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_bearerid;
}

TypeId
LwaHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LwaHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<LwaHeader> ()
    //.SetParent<Header> ()
    //.AddConstructor<LwaHeader> ()
  ;
  return tid;
}
TypeId
LwaHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
LwaHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  //os << "(seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
  os << " LWA activate status " << m_lwaactivate;
  os << " Bearer id of LWA link " << m_bearerid;
}
uint32_t
LwaHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 2;
}

void
LwaHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_lwaactivate);
  i.WriteHtonU32 (m_bearerid);
}
uint32_t
LwaHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_lwaactivate = i.ReadNtohU32 ();
  m_bearerid = i.ReadNtohU32 ();
  return GetSerializedSize ();
}

} // namespace ns3
