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
 * Inspired by epc-x2-header
 */

#include "ns3/log.h"
#include "ns3/dali-ue-dcx-header.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DaliUeDcxHeader");

NS_OBJECT_ENSURE_REGISTERED (DaliUeDcxHeader);

DaliUeDcxHeader::DaliUeDcxHeader ()
  : m_messageType (0xfa),
    m_procedureCode (0xfa),
    m_lengthOfIes (0xfa),
    m_numberOfIes (0xfa)
{
}

DaliUeDcxHeader::~DaliUeDcxHeader ()
{
  m_messageType = 0xfb;
  m_procedureCode = 0xfb;
  m_lengthOfIes = 0xfb;
  m_numberOfIes = 0xfb;
}

TypeId
DaliUeDcxHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DaliUeDcxHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<DaliUeDcxHeader> ()
  ;
  return tid;
}

TypeId
DaliUeDcxHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DaliUeDcxHeader::GetSerializedSize (void) const
{
  return 7;
}

void
DaliUeDcxHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_messageType);
  i.WriteU8 (m_procedureCode);

  i.WriteU8 (0x00); // criticality = REJECT
  i.WriteU8 (m_lengthOfIes + 3);
  i.WriteHtonU16 (0);
  i.WriteU8 (m_numberOfIes);
}

uint32_t
DaliUeDcxHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_messageType = i.ReadU8 ();
  m_procedureCode = i.ReadU8 ();

  i.ReadU8 ();
  m_lengthOfIes = i.ReadU8 () - 3;
  i.ReadNtohU16 ();
  m_numberOfIes = i.ReadU8 ();
  
  return GetSerializedSize ();
}

void
DaliUeDcxHeader::Print (std::ostream &os) const
{
  os << "MessageType=" << (uint32_t) m_messageType;
  os << " ProcedureCode=" << (uint32_t) m_procedureCode;
  os << " LengthOfIEs=" << (uint32_t) m_lengthOfIes;
  os << " NumberOfIEs=" << (uint32_t) m_numberOfIes;
}

uint8_t
DaliUeDcxHeader::GetMessageType () const
{
  return m_messageType;
}

void
DaliUeDcxHeader::SetMessageType (uint8_t messageType)
{
  m_messageType = messageType;
}

uint8_t
DaliUeDcxHeader::GetProcedureCode () const
{
  return m_procedureCode;
}

void
DaliUeDcxHeader::SetProcedureCode (uint8_t procedureCode)
{
  m_procedureCode = procedureCode;
}


void
DaliUeDcxHeader::SetLengthOfIes (uint32_t lengthOfIes)
{
  m_lengthOfIes = lengthOfIes;
}

void
DaliUeDcxHeader::SetNumberOfIes (uint32_t numberOfIes)
{
  m_numberOfIes = numberOfIes;
}


} // namespace ns3
