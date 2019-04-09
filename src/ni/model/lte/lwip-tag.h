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
#ifndef LWIP_TAG_H
#define LWIP_TAG_H

#include "ns3/packet.h"

namespace ns3 {

class Tag;

class LwipTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create a LwipTag with the default lwip 0
   */
  LwipTag ();

  /**
   * Create a LwipTag with the given LWIP value
   * \param lwip the given LWIP value
   */
  LwipTag (double lwip);

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * Set the LWIP activate status to the given value.
   *
   * \param lwip the value of the lwip to set
   */
  void Set (double lwip);
  /**
   * Return the LWIP value.
   *
   * \return the LWIP value
   */
  double Get (void) const;


private:
  double m_lwip;  //!< LWIP value
};

}
#endif /* LWIP_TAG_H */
