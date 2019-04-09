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
 * Authors: Authors: Shahwaiz Afaqui <mafaqui@uoc.edu>
 */
//*******************************New additions by UOC*********************************
#ifndef PDCP_LCID_H
#define PDCP_LCID_H

#include "ns3/packet.h"

namespace ns3 {

class Tag;

class PdcpLcid : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create a PdcpTag with the default pdcp 0
   */
  PdcpLcid ();

  /**
   * Create a PdcpTag with the given PDCP value
   * \param pdcp the given PDCP value
   */
  PdcpLcid (double lcid);

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * Set the lcid logicaL link to the given value.
   *
   * \param pdcp the value of the pdcp to set
   */
  void Set (double lcid);
  /**
   * Return the logicaL link  value.
   *
   * \return the logicaL link  value
   */
  double Get (void) const;


private:
  double m_lcid;  //!< PDCP value
};

}
#endif /* PDCP_LCID_H */
