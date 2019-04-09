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
#include "ns3/ni-api-radio-bearer-header.h"

NS_LOG_COMPONENT_DEFINE ("LteRadioBearerHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LteRadioBearerHeader);

LteRadioBearerHeader::LteRadioBearerHeader ()
  : m_rnti (0),
    m_lcid (0),
    m_layer (0)
{
}

LteRadioBearerHeader::LteRadioBearerHeader (uint16_t rnti, uint8_t lcid, uint8_t layer)
  : m_rnti (rnti),
    m_lcid (lcid),
    m_layer (layer)
{
}

LteRadioBearerHeader::~LteRadioBearerHeader ()
{
}

void
LteRadioBearerHeader::SetRnti (uint16_t rnti)
{
  m_rnti = rnti;
}

void
LteRadioBearerHeader::SetLcid (uint8_t lcid)
{
  m_lcid = lcid;
}

void
LteRadioBearerHeader::SetLayer (uint8_t layer)
{
  m_layer = layer;
}

uint16_t
LteRadioBearerHeader::GetRnti () const
{
  return m_rnti;
}

uint8_t
LteRadioBearerHeader::GetLcid () const
{
  return m_lcid;
}

uint8_t
LteRadioBearerHeader::GetLayer () const
{
  return m_layer;
}

TypeId
LteRadioBearerHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteRadioBearerHeader")
    .SetParent<Header> ()
    .AddConstructor<LteRadioBearerHeader> ()
  ;
  return tid;
}

TypeId
LteRadioBearerHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LteRadioBearerHeader::Print (std::ostream &os)  const
{
  os << "RNTI=" << m_rnti;
  os << " LCID=" << (uint16_t) m_lcid;
  os << " Layer=" << (uint16_t) m_layer;
}

uint32_t
LteRadioBearerHeader::GetSerializedSize (void) const
{
  return 4; //RNTI=2Bytes LCID=1Byte Layer=1Byte
}

void
LteRadioBearerHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_rnti);
  start.WriteU8 (m_lcid);
  start.WriteU8 (m_layer);
}

uint32_t
LteRadioBearerHeader::Deserialize (Buffer::Iterator start)
{
  m_rnti = start.ReadNtohU16 ();
  m_lcid = start.ReadU8 ();
  m_layer = start.ReadU8 ();

  return GetSerializedSize ();
}

}; // namespace ns3
