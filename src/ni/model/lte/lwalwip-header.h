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

#ifndef LWALWIP_HEADER_H
#define LWALWIP_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
/**
 *
 *
 * The header is made of a 32bits sequence number followed by
 * a 64bits time stamp.
 */
class LwaLwipHeader : public Header
{
public:
  LwaLwipHeader ();

  /**
   * \param set the lwaactivate status
   */
  void SetLwaActivate (uint32_t activate);
  /**
   * \return the lwa status
   */
  uint32_t GetLwaActivate (void) const;
  /**
   * \set the lwaactivate status
   */
  void SetLwipActivate (uint32_t activate);
  /**
   * \get the lwaactivate status
   */
  uint32_t GetLwipActivate (void) const;
  /**
   * \return bearer id
   */
  void SetBearerId (uint32_t bearerid);
  /**
   * \return bearer id
   */
  uint32_t GetBearerId (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_lwaactivate; //!< LWA activate status
  uint32_t m_lwipactivate; //!< LWIP activate status
  uint32_t m_bearerid; //!< Bearer id of LWIP link
};

} // namespace ns3

#endif /* SEQ_TS_HEADER_H */
