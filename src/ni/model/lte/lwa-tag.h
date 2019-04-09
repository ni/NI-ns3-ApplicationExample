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
#ifndef LWA_TAG_H
#define LWA_TAG_H

#include "ns3/packet.h"

namespace ns3 {

class Tag;

class LwaTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create a LwaTag with the default lwa 0
   */
  LwaTag ();

  /**
   * Create a LwaTag with the given LWA value
   * \param lwa the given LWA value
   */
  LwaTag (double lwa);

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * Set the LWA activate status to the given value.
   *
   * \param lwa the value of the lwa to set
   */
  void Set (double lwa);
  /**
   * Return the LWA value.
   *
   * \return the LWA value
   */
  double Get (void) const;


private:
  double m_lwa;  //!< LWA value
};

}
#endif /* LWA_TAG_H */
