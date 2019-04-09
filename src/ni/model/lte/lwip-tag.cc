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
#include "lwip-tag.h"

#include "ns3/tag.h"
#include "ns3/double.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LwipTag);

TypeId
LwipTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LwipTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<LwipTag> ()
    .AddAttribute ("Lwip", "The Lwip status of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LwipTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
LwipTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

LwipTag::LwipTag ()
  : m_lwip (0)
{
}

LwipTag::LwipTag (double lwip)
  : m_lwip (lwip)
{
}

uint32_t
LwipTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
LwipTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_lwip);
}

void
LwipTag::Deserialize (TagBuffer i)
{
  m_lwip = i.ReadDouble ();
}

void
LwipTag::Print (std::ostream &os) const
{
  os << "Pdcpc=" << m_lwip;
}

void
LwipTag::Set (double lwip)
{
  m_lwip = lwip;
}

double
LwipTag::Get (void) const
{
  return m_lwip;
}

}
