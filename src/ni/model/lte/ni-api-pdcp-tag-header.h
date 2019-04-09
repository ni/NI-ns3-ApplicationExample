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

#ifndef LTE_PDCP_TAG_HEADER_H
#define LTE_PDCP_TAG_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {

  /**
   * Tag to calculate the per-PDU delay from eNb PDCP to UE PDCP
   */

class PdcpTagHeader : public Header
{
public:
  /**
   * Create an empty PDCP-tag header
   */
  PdcpTagHeader ();
  /**
   * Create an PDCP-tag header with the given senderTimestamp
   */
  PdcpTagHeader (Time senderTimestamp);

  ~PdcpTagHeader ();

  static TypeId  GetTypeId (void);
  virtual TypeId  GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t  GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Get the instant when the PDCP delivers the PDU to the MAC SAP provider
   */
  Time  GetSenderTimestamp (void) const;

  /**
   * Set the sender timestamp
   * @param senderTimestamp time stamp of the instant when the PDCP delivers the PDU to the MAC SAP provider
   */
  void  SetSenderTimestamp (Time senderTimestamp);

private:
  Time m_senderTimestamp;

};

}; // namespace ns3

#endif /* LTE_PDCP_TAG_HEADER_H */
