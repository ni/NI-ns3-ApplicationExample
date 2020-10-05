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

#ifndef LWALWIP_HANDLER_H
#define LWALWIP_HANDLER_H

#include <iostream>
//#include "ns3/core-module.h"
//#include "ns3/network-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/network-module.h"
#include "ns3/virtual-net-device.h"

namespace ns3 {


  // tunnel class for lwip ipsec emulation
  class LwaLwipHandler
  {
    public:
      LwaLwipHandler (Ptr<Socket> lwaapTxSocket,
                      Ptr<Socket> lwipepTxSocket,
                      bool niApiEnableTapBridge,
                      Ipv4Address wifiSta1IpAddr);
      ~LwaLwipHandler ();

      void LtePdcpLwaLwipHandler (Ptr< const Packet> p);
      void Callback_LtePDCPTX (void);

    private:
      // global variables
      Ptr<Socket> m_lwaapTxSocket;
      Ptr<Socket> m_lwipepTxSocket;
      bool m_niApiEnableTapBridge;
      Ipv4Address m_wifiSta1IpAddr;

  };
} // namespace ns3

#endif /* LWALWIP_HANDLER_H */
