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
#include "pdcp-tagc.h"

#include "ns3/tag.h"
#include "ns3/double.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PdcpTagc);

TypeId
PdcpTagc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PdcpTagc")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<PdcpTagc> ()
    .AddAttribute ("Pdcp", "The pdcpc of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&PdcpTagc::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
PdcpTagc::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

PdcpTagc::PdcpTagc ()
  : m_pdcpc (0)
{
}

PdcpTagc::PdcpTagc (double pdcpc)
  : m_pdcpc (pdcpc)
{
}

uint32_t
PdcpTagc::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
PdcpTagc::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_pdcpc);
}

void
PdcpTagc::Deserialize (TagBuffer i)
{
  m_pdcpc = i.ReadDouble ();
}

void
PdcpTagc::Print (std::ostream &os) const
{
  os << "Pdcpc=" << m_pdcpc;
}

void
PdcpTagc::Set (double pdcpc)
{
  m_pdcpc = pdcpc;
}

double
PdcpTagc::Get (void) const
{
  return m_pdcpc;
}

}
