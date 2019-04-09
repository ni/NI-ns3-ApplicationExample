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

//*******************************New additions by UOC*********************************
#include "lwa-tag.h"

#include "ns3/tag.h"
#include "ns3/double.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LwaTag);

TypeId
LwaTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LwaTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<LwaTag> ()
    .AddAttribute ("Lwa", "The Lwa status of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LwaTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
LwaTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

LwaTag::LwaTag ()
  : m_lwa (0)
{
}

LwaTag::LwaTag (double lwa)
  : m_lwa (lwa)
{
}

uint32_t
LwaTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
LwaTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_lwa);
}

void
LwaTag::Deserialize (TagBuffer i)
{
  m_lwa = i.ReadDouble ();
}

void
LwaTag::Print (std::ostream &os) const
{
  os << "LWA=" << m_lwa;
}

void
LwaTag::Set (double lwa)
{
  m_lwa = lwa;
}

double
LwaTag::Get (void) const
{
  return m_lwa;
}

}
