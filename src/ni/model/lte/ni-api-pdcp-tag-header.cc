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

#include "ns3/ni-api-pdcp-tag-header.h"

#include "ns3/header.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PdcpTagHeader);

PdcpTagHeader::PdcpTagHeader ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}


PdcpTagHeader::PdcpTagHeader (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)

{
  // Nothing to do here
}

PdcpTagHeader::~PdcpTagHeader ()
{
}

TypeId
PdcpTagHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PdcpTagHeader")
    .SetParent<Header> ()
    .AddConstructor<PdcpTagHeader> ();
  return tid;
}

TypeId
PdcpTagHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
PdcpTagHeader::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
PdcpTagHeader::Serialize (Buffer::Iterator start) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  start.WriteHtonU64 ((uint64_t)senderTimestamp);
}

uint32_t
PdcpTagHeader::Deserialize (Buffer::Iterator start)
{
  int64_t senderTimestamp = start.ReadNtohU64 ();
  m_senderTimestamp = NanoSeconds (senderTimestamp);
  return GetSerializedSize ();
}

void
PdcpTagHeader::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
PdcpTagHeader::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
PdcpTagHeader::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


} // namespace ns3




