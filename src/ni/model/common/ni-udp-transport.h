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

#ifndef NI_UDP_TRANSPORT_H_
#define NI_UDP_TRANSPORT_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iomanip>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include <ns3/object.h>
#include <ns3/system-thread.h>
#include "ni-common-constants.h"

namespace ns3
{

  typedef Callback< bool, uint8_t* > NiUdpTransportDataEndOkCallback;

  // note - used as member of ns-3 object class - mainly used for callback functionality
  class NiUdpTransport : public Object
  {
  public:
    NiUdpTransport ();
    NiUdpTransport (std::string context);
    virtual
    ~NiUdpTransport ();

    void SetNiApiDataEndOkCallback (NiUdpTransportDataEndOkCallback c);
    bool GetTxEndPointOpen () const;
    bool GetRxEndPointOpen () const;

    void OpenUdpSocketTx(std::string remoteTxIpAddr, std::string remoteTxPort);
    void SendToUdpSocketTx(uint8_t* txBuffer, uint32_t txBufferSize);
    void CloseUdpSocketTx(void);

    void OpenUdpSocketRx(std::string locaRxPort, int rxThreadPriority);
    void ReceiveFromUdpSocketRx();
    void CloseUdpSocketRx(void);

    void PrintBufferU8(uint8_t* p_buffer, uint32_t p_bufferOffset);

  private:

    NiUdpTransportDataEndOkCallback m_niApiDataEndOkCallback;

    std::string m_niUdpRemoteIpAddrTx;
    std::string m_niUdpRemotePortTx;
    std::string m_niUdpLocaPortRx;

    std::string m_context; // "LTE" or "WIFI"

    struct sockaddr_in m_remoteAddr;

    int32_t m_sockFdTx, m_sockFdRx;
    fd_set m_readFds;

    uint8_t*  m_pBufU8Rx;
    const uint32_t m_numBufU8RxEntries = 128; // number of buffer entries for UDP Rx Thread
    const size_t m_maxUdpRxPacketSize = NI_COMMON_CONST_MAX_PAYLOAD_SIZE; // bytes

    bool m_niApiTxEndPointOpen;
    bool m_niApiRxEndPointOpen;
    bool m_rxApiThreadStop;

    //member variables for RX threads
    Ptr<SystemThread> m_niApiRxThread;
    int m_niApiRxThreadPriority;
  };

}

#endif /* NI_UDP_TRANSPORT_H_ */
