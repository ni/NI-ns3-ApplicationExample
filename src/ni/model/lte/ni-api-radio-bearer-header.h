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

#ifndef LTE_RADIO_BEARER_HEADER_H
#define LTE_RADIO_BEARER_HEADER_H

#include "ns3/header.h"


namespace ns3 {

/**
 * Header used to define the RNTI and LC id for each MAC packet transmitted
 */

class LteRadioBearerHeader : public Header
{
public:

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  LteRadioBearerHeader ();
  LteRadioBearerHeader (uint16_t rnti, uint8_t lcid, uint8_t layer);
  ~LteRadioBearerHeader ();

  void SetRnti (uint16_t rnti);
  void SetLcid (uint8_t lcid);
  void SetLayer (uint8_t layer);

  uint16_t GetRnti () const;
  uint8_t GetLcid () const;
  uint8_t GetLayer () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint16_t m_rnti;
  uint8_t m_lcid;
  uint8_t m_layer;
};

}; // namespace ns3

#endif /* LTE_RADIO_BEARER_HEADER_H */
