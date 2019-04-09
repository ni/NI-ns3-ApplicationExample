/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
 * Copyright (c) 2013 University of Surrey
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
 * Authors: Shahwaiz Afaqui <mafaqui@uoc.edu>
 */
//*******************************New additions by UOC*********************************
#include "pdcp-lcid.h"

#include "ns3/tag.h"
#include "ns3/double.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PdcpLcid);

TypeId
PdcpLcid::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PdcpLcid")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<PdcpLcid> ()
    .AddAttribute ("Pdcp", "The bearer id of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&PdcpLcid::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
PdcpLcid::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

PdcpLcid::PdcpLcid ()
  : m_lcid (0)
{
}

PdcpLcid::PdcpLcid (double lcid)
  : m_lcid (lcid)
{
}

uint32_t
PdcpLcid::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
PdcpLcid::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_lcid);
}

void
PdcpLcid::Deserialize (TagBuffer i)
{
  m_lcid = i.ReadDouble ();
}

void
PdcpLcid::Print (std::ostream &os) const
{
  os << "lcid=" << m_lcid;
}

void
PdcpLcid::Set (double lcid)
{
  m_lcid = lcid;
}

double
PdcpLcid::Get (void) const
{
  return m_lcid;
}

}
