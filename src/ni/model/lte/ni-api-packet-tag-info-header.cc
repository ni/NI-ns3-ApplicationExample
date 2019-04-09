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

#include "ns3/ni-api-packet-tag-info-header.h"

#include "ns3/header.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LtePacketTagInfoHeader);

LtePacketTagInfoHeader::LtePacketTagInfoHeader ()
  : m_numOfTagHeaders (0),
    m_headerTypeList (0)
{
  // Nothing to do here
}


LtePacketTagInfoHeader::LtePacketTagInfoHeader (uint16_t numOfTagHeaders, const std::vector<uint8_t>& headerTypeList)
  : m_numOfTagHeaders (numOfTagHeaders),
    m_headerTypeList (headerTypeList)

{
  // Nothing to do here
}

LtePacketTagInfoHeader::~LtePacketTagInfoHeader ()
{
}

TypeId
LtePacketTagInfoHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtePacketTagInfoHeader")
    .SetParent<Header> ()
    .AddConstructor<LtePacketTagInfoHeader> ();
  return tid;
}

TypeId
LtePacketTagInfoHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LtePacketTagInfoHeader::GetSerializedSize (void) const
{
  return sizeof(m_numOfTagHeaders) + m_headerTypeList.size ();
}

void
LtePacketTagInfoHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_numOfTagHeaders);
  for (uint8_t i=0; i<m_numOfTagHeaders; i++)
  {
    start.WriteU8 (m_headerTypeList[i]);
  }
}

uint32_t
LtePacketTagInfoHeader::Deserialize (Buffer::Iterator start)
{
  m_numOfTagHeaders = start.ReadNtohU16 ();
  m_headerTypeList.clear ();
  uint8_t tagType;
  for (uint8_t i=0; i<m_numOfTagHeaders; i++)
  {
    tagType = start.ReadU8 ();
    m_headerTypeList.push_back (tagType);
  }
  return GetSerializedSize ();
}

void
LtePacketTagInfoHeader::Print (std::ostream &os) const
{
  os << m_numOfTagHeaders;
}

uint16_t
LtePacketTagInfoHeader::GetNumOfTagHeaders (void) const
{
  return m_numOfTagHeaders;
}

std::vector<uint8_t>
LtePacketTagInfoHeader::GetHeaderTypeList (void) const
{
  return m_headerTypeList;
}

/**
 * Set the sender timestamp
 * @param senderTimestamp time stamp of the instant when the PDCP delivers the PDU to the MAC SAP provider
 */
void
LtePacketTagInfoHeader::SetNumOfTagHeaders (uint16_t numOfTagHeaders)
{
  m_numOfTagHeaders = numOfTagHeaders;
}

void
LtePacketTagInfoHeader::SetHeaderTypeList (const std::vector<uint8_t>& headerTypeList)
{
  m_headerTypeList = headerTypeList;
}

} // namespace ns3
