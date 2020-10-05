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

#include <sstream>
#include <iomanip>
#include <stdio.h>

#include "ns3/ni-logging.h"
#include "ns3/ni-utils.h"

#include "ni-udp-transport.h"

namespace ns3
{

  NiUdpTransport::NiUdpTransport ()
  {
    m_context = "none";
    m_rxApiThreadStop = true;
    m_niApiDataEndOkCallback = MakeNullCallback< bool, uint8_t* >();
  }

  NiUdpTransport::NiUdpTransport (std::string context)
  {
    m_context = context;
    m_rxApiThreadStop = true;
    m_niApiDataEndOkCallback = MakeNullCallback< bool, uint8_t* >();
  }

  NiUdpTransport::~NiUdpTransport ()
  {
    // TODO Auto-generated destructor stub
  }

  void
  NiUdpTransport::SetNiApiDataEndOkCallback (NiUdpTransportDataEndOkCallback c)
  {
    m_niApiDataEndOkCallback = c;
  }

  bool
  NiUdpTransport::GetTxEndPointOpen () const
  {
    return m_niApiTxEndPointOpen;
  }

  bool
  NiUdpTransport::GetRxEndPointOpen () const
  {
    return m_niApiRxEndPointOpen;
  }

  void
  NiUdpTransport::OpenUdpSocketTx(std::string remoteTxIpAddr, std::string remoteTxPort)
  {
    m_niUdpRemoteIpAddrTx = remoteTxIpAddr;
    m_niUdpRemotePortTx   = remoteTxPort;

    NI_LOG_DEBUG(m_context << " - create UDP Tx socket with remote addr = " << remoteTxIpAddr << ", remote port = " << remoteTxPort);

    struct sockaddr_in localAddr;

    // create udp socket
    if ((m_sockFdTx=socket (AF_INET, SOCK_DGRAM, 0)) < 0)
      {
        NI_LOG_FATAL (m_context << " - Error Tx UDP socket creation failed!");
      }

    // bind socket to all local communication interfaces with corresponding
    // ip address and pick any port number
    memset((char *)&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family      = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port        = htons(0);
    // bind tx socket
    if (bind(m_sockFdTx, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
      {
        NI_LOG_FATAL (m_context << " - Error Tx UDP socket bind failed!");
      }

    // set remote ip address and port
    memset ((char *)&m_remoteAddr, 0, sizeof (m_remoteAddr));
    m_remoteAddr.sin_family      = AF_INET;
    m_remoteAddr.sin_port        = htons (atoi (remoteTxPort.c_str ()));
    // convert dot address to network address
    if (inet_aton (remoteTxIpAddr.c_str (), &m_remoteAddr.sin_addr)==0)
      {
        NI_LOG_FATAL (m_context << " - Error Tx UDP Socket inet_aton() failed!");
      }

    NI_LOG_DEBUG(m_context << " - UDP TX socket created and opened with"
        << " local socket = "<< m_sockFdTx
        << " address = " << inet_ntoa(localAddr.sin_addr)
        << " port = "<< ntohs(localAddr.sin_port));

    m_niApiTxEndPointOpen = true;
  }

  void
  NiUdpTransport::SendToUdpSocketTx(uint8_t *txBuffer, uint32_t txBufferSize)
  {
    if(!m_niApiTxEndPointOpen)
      {
        NI_LOG_FATAL (m_context << "- Error Tx UDP Socket not open");
      }

    NI_LOG_DEBUG(m_context << " - Data of size " << txBufferSize << " sent to ip addr=" << inet_ntoa(m_remoteAddr.sin_addr) << " / port=" << ntohs(m_remoteAddr.sin_port));
    //PrintBufferU8(txBuffer, 16);

    // send tx buffer content to m_remoteAddr
    if (sendto(m_sockFdTx, txBuffer, txBufferSize, 0, (struct sockaddr *)&m_remoteAddr, sizeof(m_remoteAddr))==-1)
      {
        NI_LOG_FATAL (m_context << "- Tx UDP Socket send failed");
      }
  }

  void
  NiUdpTransport::CloseUdpSocketTx(void)
  {
    if (m_sockFdTx >= 0);
    {
        close(m_sockFdTx);
        NI_LOG_DEBUG(m_context << " - UDP Tx socket closed");
    }
  }

  void
  NiUdpTransport::OpenUdpSocketRx(std::string localRxPort, int rxThreadPriority)
  {
    m_niUdpLocaPortRx = localRxPort;

    NI_LOG_DEBUG(m_context << " - create UDP Rx socket on local port = " << localRxPort);

    struct addrinfo localAddrInfoHints, *serverInfo, *pAddrInfo;

    // create addr info structure
    memset (&localAddrInfoHints, 0, sizeof (localAddrInfoHints));
    localAddrInfoHints.ai_family   = AF_INET; //AF_UNSPEC; // set to AF_INET to force IPv4
    localAddrInfoHints.ai_socktype = SOCK_DGRAM;
    localAddrInfoHints.ai_flags    = AI_PASSIVE; // use my IP

    // clear the set
    FD_ZERO(&m_readFds);

    // translation of name of service location (UDP port) to set of socket addresses
    int rxRetVal;
    if ((rxRetVal = getaddrinfo (NULL, localRxPort.c_str(), &localAddrInfoHints, &serverInfo)) != 0)
      {
        NI_LOG_FATAL (m_context << " - sockFdRx getaddrinfo: " << gai_strerror(rxRetVal));
      }

    for (pAddrInfo = serverInfo; pAddrInfo != NULL; pAddrInfo = pAddrInfo->ai_next)
      {
        // create udp socket
        if ((m_sockFdRx = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol)) == -1)
          {
            NI_LOG_DEBUG(m_context << " - Rx UDP socket bind failed");
            continue;
          }

        // lose the pesky "address already in use" error message
        int yes = 1;
        if (setsockopt(m_sockFdRx, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
          {
            NI_LOG_FATAL(m_context << " - Error Rx UDP setsockopt reuse " << m_sockFdRx);
          }
        // set receive time out for socket in order to be able to close the thread on de-init
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 2000; // 2ms
        if (setsockopt(m_sockFdRx, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
          {
            NI_LOG_FATAL(m_context << " - Error Rx UDP setsockopt timeout " << m_sockFdRx);
          }

        // bind socket to all local communication interfaces with
        // the local address ai_addr with length ai_addrlen
        if (bind(m_sockFdRx, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen) == -1)
          {
            //close socket if error has occurred
            close(m_sockFdRx);

            NI_LOG_FATAL(m_context << " - Error Rx UDP socket bind failed");
            continue;
          }
        else
          {
            NI_LOG_DEBUG(m_context << " - Rx UDP socket bind successful");
            break;
          }
      }

    if (pAddrInfo == NULL)
      {
        NI_LOG_FATAL (m_context << " - Error Rx UDP socket bind failed!");
      }

    m_niApiRxEndPointOpen = true;

    NI_LOG_DEBUG(m_context << " - Rx UDP socket bound to address " << pAddrInfo->ai_addrlen);

    freeaddrinfo (serverInfo);

    // Add the sockfd to the m_readFds set
    FD_SET (m_sockFdRx, &m_readFds);

    // buffer for NIAPI messages
    m_pBufU8Rx = (uint8_t*)malloc(sizeof(uint8_t)*m_maxUdpRxPacketSize*m_numBufU8RxEntries);
    if (m_pBufU8Rx == NULL)
      {
        NI_LOG_FATAL("NiUdpTransport::OpenUdpSocketRx: malloc for rxPayload failed")
      }
    memset(m_pBufU8Rx, 0, m_maxUdpRxPacketSize*m_numBufU8RxEntries);

    // create system thread for listening to local rx port
    m_rxApiThreadStop = false;
    m_niApiRxThreadPriority = rxThreadPriority;
    m_niApiRxThread = Create<SystemThread> (MakeCallback (&NiUdpTransport::ReceiveFromUdpSocketRx, this));
    m_niApiRxThread->Start ();

    return;
  }

  void
  NiUdpTransport::ReceiveFromUdpSocketRx()
  {
    // set thread priority
    NiUtils::SetThreadPrioriy(m_niApiRxThreadPriority);
    NiUtils::AddThreadInfo (m_context + " UdpRx Thread");
    NI_LOG_DEBUG(m_context << " - UDP Rx thread with id:" <<  m_niApiRxThread->Self() << " started");

    // TODO-NI set thread priority!
    //NiUtils::SetThreadPrioriy(...);

    int32_t numBytesRx = 0;
    uint32_t bufU8RxEntry = 0;

    struct sockaddr_storage rxAddr;
    socklen_t rxAddrLen = sizeof (rxAddr);

    if(!m_niApiRxEndPointOpen)
      {
        m_rxApiThreadStop = true;
        NI_LOG_FATAL (m_context << " - Rx UDP Socket not open!");
      }

    while(!m_rxApiThreadStop)
      {
        // the descriptor has data
        if (FD_ISSET (m_sockFdRx, &m_readFds))
          {
            // calculate offset to current buffer entry
            uint32_t bufU8RxOffset = bufU8RxEntry * m_maxUdpRxPacketSize;
            //read m_maxUdpRxPacketSize bytes from m_rxBuf into m_sockFdRx, their_addr to identify the sender
            numBytesRx = recvfrom (m_sockFdRx, m_pBufU8Rx + bufU8RxOffset,\
                                   m_maxUdpRxPacketSize, 0, \
                                   (struct sockaddr *)&rxAddr, \
                                   &rxAddrLen);
            if (numBytesRx > 0 && !m_rxApiThreadStop)
              {
                NI_LOG_DEBUG(m_context << " - Udp packet of size " << numBytesRx << " from ip addr=" << inet_ntoa(((struct sockaddr_in *)&rxAddr)->sin_addr) << " received.");
                //PrintBufferU8(rxBuffer, 16);
                // check im ns-3 is running
                const bool ns3Running = ((Simulator::Now().GetMicroSeconds() > 0) || !(Simulator::GetContext () == Simulator::NO_CONTEXT));
                const bool callbackValid = (m_niApiDataEndOkCallback != MakeNullCallback< bool, uint8_t* >());
                // call function for rx packet processing
                if (callbackValid && ns3Running)
                  {
                    m_niApiDataEndOkCallback(m_pBufU8Rx + bufU8RxOffset);
                  }
                // switch to the next buffer entry
                bufU8RxEntry = (bufU8RxEntry + 1) % m_numBufU8RxEntries;
                //printf("%d, %d, %p\n", bufU8RxEntry, bufU8RxOffset, (m_pBufU8Rx + bufU8RxOffset));
              }
          }
      } // end while loop
    NI_LOG_DEBUG(m_context << " - NiUdpTransport::ReceiveFromUdpSocketRx: stopped");
  }

  void
  NiUdpTransport::CloseUdpSocketRx(void)
  {
    m_rxApiThreadStop = true;
    m_niApiRxThread->Join();
    if (m_sockFdRx >= 0)
      {
        close(m_sockFdRx);
      }
    free (m_pBufU8Rx);
    NI_LOG_DEBUG(m_context << " - NiUdpTransport::CloseUdpSocketRx: socket and thread closed");
  }

  void
  NiUdpTransport::PrintBufferU8(uint8_t* p_buffer, uint32_t p_bufferOffset)
  {
    std::stringstream ss;
    ss << std::hex;

    for (uint32_t i = 0; i < p_bufferOffset; ++i)
      {
              if ((i!=0) && (i%16 == 0)) //printf("\n");
                ss << "\n";

              //printf("%02x", p_buffer[i]);
              ss << std::setw(2) << std::setfill('0') << (int)p_buffer[i];

              if ((i != p_bufferOffset-1)  || (i == 0) || (i%14 != 0)) //printf (" | ");
                ss << " | ";

      }

    std::string tmp = ss.str();

    NI_LOG_CONSOLE_DEBUG(m_context << " NiUdpTransport::PrintBufferU8: " << tmp.c_str());
  }

} // end ns3 namespace
