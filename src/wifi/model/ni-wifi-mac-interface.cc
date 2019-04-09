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


#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/core-module.h"
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>

#include <ctype.h>
#include <iomanip>
#include <inttypes.h>

#include "ns3/ni-utils.h"
#include "ni-wifi-mac-interface.h"

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE ("NiWifiMacInterface");

  NS_OBJECT_ENSURE_REGISTERED (NiWifiMacInterface);

  TypeId
  NiWifiMacInterface::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NiWifiMacInterface")
          .SetParent<Object> ()
          .SetGroupName("Wifi")
          .AddConstructor<NiWifiMacInterface> ()
          .AddAttribute ("NumOfStations",
                         "To set the number of stations used in Infrastructure mode",
                         IntegerValue (1),
                         MakeIntegerAccessor (&NiWifiMacInterface::m_NumOfStations),
                         MakeIntegerChecker<uint32_t>())
          .AddAttribute ("niApiWifiApRemoteIpAddrTx",
                         "Remote IP address for UDP socket on AP",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiApRemoteIpAddrTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiApRemotePortTx",
                         "Remote port1 for UDP socket on AP",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiApRemotePortTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiApLocalPortRx",
                         "Local RX port of AP",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiApLocalPortRx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiStaRemoteIpAddrTx",
                         "Remote IP address for UDP socket on STA",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiStaRemoteIpAddrTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiStaRemotePortTx",
                         "Remote port for UDP socket on STA",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiStaRemotePortTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiStaLocalPortRx",
                         "Local RX port of STA",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiStaLocalPortRx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiApMacAddress",
                         "MAC address of AP",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiApMacAddress),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiStaMacAddress",
                         "MAC address of STA",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiStaMacAddress),
                         MakeStringChecker ())

          .AddAttribute ("niApiWifiSta1RemoteIpAddrTx",
                         "Remote IP address for UDP socket on STA1",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta1RemoteIpAddrTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta1RemotePortTx",
                         "Remote port for UDP socket on STA1",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta1RemotePortTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta1LocalPortRx",
                         "Local RX port of STA1",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta1LocalPortRx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta2RemoteIpAddrTx",
                         "Remote IP address for UDP socket on STA2",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta2RemoteIpAddrTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta2RemotePortTx",
                         "Remote port for UDP socket on STA2",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta2RemotePortTx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta2LocalPortRx",
                         "Local RX port of STA2",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta2LocalPortRx),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta1MacAddr",
                         "MAC address of STA1",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta1MacAddr),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiSta2MacAddr",
                         "MAC address of STA2",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiSta2MacAddr),
                         MakeStringChecker ())
          .AddAttribute ("niApiWifiBssidMacAddress",
                         "BSSID MAC address (equal to AP's MAC address in Infrastructure mode)",
                         StringValue (""),
                         MakeStringAccessor (&NiWifiMacInterface::m_niApiWifiBssidMacAddress),
                         MakeStringChecker ())
          .AddAttribute ("niApiDevType",
                         "To set whether the simulation should be run as AP or STA",
                         StringValue ("NIAPI_ALL"),
                         MakeStringAccessor (&NiWifiMacInterface::SetNiWifiDevType),
                         MakeStringChecker ())
          .AddAttribute ("enableNiApiLoopback",
                         "To be set if the simulation should run in UDP Loopback mode",
                         BooleanValue (true),
                         MakeBooleanAccessor (&NiWifiMacInterface::SetNiApiLoopbackEnable),
                         MakeBooleanChecker ())
          .AddAttribute ("niApiConfirmationMessage",
                         "To be set if the simulation should recieve confirmation message from AFW",
                         BooleanValue (false),
                         MakeBooleanAccessor (&NiWifiMacInterface::m_NiApiConfirmationMessage),
                         MakeBooleanChecker ())
          .AddAttribute ("enableNiApi",
                         "Activate/Deactivate NI API Code.",
                         BooleanValue (false),
                         MakeBooleanAccessor (&NiWifiMacInterface::SetNiApiEnable),
                         MakeBooleanChecker ())
          .AddAttribute ("niApiWifiEnablePrintMsgContent",
                         "To be set if the simulation should print packet contents",
                         BooleanValue (true),
                         MakeBooleanAccessor (&NiWifiMacInterface::m_niApiWifiEnablePrintMsgContent),
                         MakeBooleanChecker ())
          .AddAttribute ("niApiWifiMcs",
                         "To set MCS that is used by the 802.11 AFW",
                         IntegerValue (0),
                         MakeIntegerAccessor (&NiWifiMacInterface::m_niApiWifiMcs),
                         MakeIntegerChecker<uint32_t>())
                        ;
    return tid;
  }

  NiWifiMacInterface::NiWifiMacInterface()
  {
    NS_LOG_FUNCTION (this);
    NS_FATAL_ERROR ("This constructor should not be called");
  };

  NiWifiMacInterface::NiWifiMacInterface(Ns3WifiDevType_t ns3WifiDevType)
  : m_ns3WifiDevType(ns3WifiDevType),
    NIAPIIsTxEndPointOpen (false),
    NIAPIIsRxEndPointOpen (false),
    m_initializationDone (false)

  {

  }

  NiWifiMacInterface::~NiWifiMacInterface()
  {
    //NS_LOG_FUNCTION (this);
  }

  void
  NiWifiMacInterface::DoDispose (void)
  {
    if (m_enableNiApi && m_initializationDone)
      {
        DeInitializeNiUdpTransport();
      }
    m_enableNiApi = false;
    m_initializationDone = false;
  }

  // automatically called by NS-3
  void
  NiWifiMacInterface::DoInitialize (void)
  {
    if (m_enableNiApi && !m_initializationDone) {
      InitializeNiUdpTransport();
      m_initializationDone = true;
    }
  }

  void
  NiWifiMacInterface::SetNiWifiDevType (std::string type)
  {
    NI_LOG_DEBUG(this << " - Set NI WIFI API Mode to " << type);

    if (type == "NIAPI_AP")
      m_niApiWifiDevType = NIAPI_AP;
    else if (type == "NIAPI_STA")
      m_niApiWifiDevType = NIAPI_STA;
    else if (type == "NIAPI_ALL")
      m_niApiWifiDevType = NIAPI_WIFI_ALL;
    else if (type == "NIAPI_STA1")
      m_niApiWifiDevType = NIAPI_STA1;
    else if (type == "NIAPI_STA2")
      m_niApiWifiDevType = NIAPI_STA2;
    else if (type == "NIAPI_NONE")
      m_niApiWifiDevType = NIAPI_WIFI_NONE;
    else
      NI_LOG_FATAL("\n Unrecognizable option for running the program in NI API WIFI device mode");

    return;
  }

  NiApiWifiDevType_t
  NiWifiMacInterface::GetNiWifiDevType () const
  {
    return m_niApiWifiDevType;
  }

  Ns3WifiDevType_t
  NiWifiMacInterface::GetNs3DevType () const
  {
    return m_ns3WifiDevType;
  }

  void
  NiWifiMacInterface::SetNiApiEnable (bool enable)
  {
    NI_LOG_DEBUG(this << " - Set NI WiFi API Mode to " << enable);

    m_enableNiApi = enable;

    // initialize udp transport layer
    this->DoInitialize();
  }

  bool
  NiWifiMacInterface::GetNiApiEnable () const
  {
    return m_enableNiApi;
  }

  void
  NiWifiMacInterface::SetNiApiLoopbackEnable (bool enable)
  {
    NI_LOG_DEBUG(this << " - Set NI WiFi API Loopback Mode to " << enable);

    m_enableNiApiLoopback = enable;
  }

  void
  NiWifiMacInterface::InitializeNiUdpTransport()
  {
    NI_LOG_NONE (this << " " << __FILE__ << " " << __func__);

    // deriving thread priorities from ns3 main process
    int ns3Priority = NiUtils::GetThreadPrioriy();
    const int rxThreadPriority = ns3Priority - 5;

    if ((m_ns3WifiDevType == NS3_AP) && ((m_niApiWifiDevType == NIAPI_AP)||(m_niApiWifiDevType==NIAPI_WIFI_ALL)))
      {
        if(m_enableNiApiLoopback)
          {
            uint32_t niApiWifiApRemotePortTx = std::stoi (m_niApiWifiApRemotePortTx.c_str());

            for (uint32_t index = 0; index < m_NumOfStations; ++index)
              {
                // FIXME-NI: Dangerous! Array has 10 elements but m_NumOfStations can be >10, wich leads to segmentation faults
                m_wifiNiUdpTransportArray[index] = CreateObject <NiUdpTransport> ("WIFI_" + std::to_string(index));

                // create udp transport object from AP to STA1 in Infrastructure mode
                NI_LOG_CONSOLE_DEBUG("Initialize AP UDP Tx Socket" << index);
                m_wifiNiUdpTransportArray[index]->OpenUdpSocketTx(m_niApiWifiApRemoteIpAddrTx, m_niApiWifiApRemotePortTx);

                niApiWifiApRemotePortTx++;
                std::stringstream temp;
                temp << niApiWifiApRemotePortTx;
                m_niApiWifiApRemotePortTx = temp.str();

                //				// set callback function for rx packets
                //				m_wifiNiUdpTransportArray[index]->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiStartRxCtrlDataFrame, this));
                //
                //				NI_LOG_CONSOLE_DEBUG("Initialize AP UDP Rx Socket");
                //				m_wifiNiUdpTransportArray[index]->OpenUdpSocketRx(m_niApiWifiApLocalPortRx);
              }

            // set callback function for rx packets
            m_wifiNiUdpTransportArray[0]->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiStartRxCtrlDataFrame, this));

            NI_LOG_CONSOLE_DEBUG("Initialize AP UDP Rx Socket");
            m_wifiNiUdpTransportArray[0]->OpenUdpSocketRx(m_niApiWifiApLocalPortRx, rxThreadPriority);
          }
        else
          {
            // TODO-NI: member m_wifiNiUdpTransport should be replaced by m_wifiNiUdpTransportArray[0]
            m_wifiNiUdpTransport = CreateObject <NiUdpTransport> ("WIFI");
            //    // set callback function for rx packets
            m_wifiNiUdpTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiStartRxCtrlDataFrame, this));

            // create udp transport object from AP to STA1 in Infrastructure mode
            NI_LOG_CONSOLE_DEBUG("Initialize AP UDP Tx Socket1");
            m_wifiNiUdpTransport->OpenUdpSocketTx(m_niApiWifiApRemoteIpAddrTx, m_niApiWifiApRemotePortTx);

            NI_LOG_CONSOLE_DEBUG("Initialize AP UDP Rx Socket");
            m_wifiNiUdpTransport->OpenUdpSocketRx(m_niApiWifiApLocalPortRx, rxThreadPriority);
            // To listen the MAC SAP TX CNF messages on the port 12501 from 802.11 AFW
            if (m_NiApiConfirmationMessage)
				{
					m_wifiNiUdpTxCNF = CreateObject <NiUdpTransport> ("WIFI_txCNF");

					NI_LOG_CONSOLE_DEBUG("Initialize AP UDP TX confirmation receive Socket");
					m_wifiNiUdpTxCNF->OpenUdpSocketRx("12501",rxThreadPriority);

					m_wifiNiUdpTxCNF->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiTXCnfReqDataFrame, this));

				}
          }


      }
    // TODO-NI: remove code duplication -> ports to be set to a local variable depending on the scenario and after this the sockets and callbacks will be setup in one call
    else if ((m_ns3WifiDevType == NS3_STA)  && ((m_niApiWifiDevType == NIAPI_STA)||(m_niApiWifiDevType==NIAPI_WIFI_ALL)))
      {
        m_wifiNiUdpTransport = CreateObject <NiUdpTransport> ("WIFI");
        // set callback function for rx packets
        m_wifiNiUdpTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiStartRxCtrlDataFrame, this));

        // create udp transport object for STA in Infrastructure mode
        NI_LOG_CONSOLE_DEBUG("Initialize STA UDP Tx Socket");
        m_wifiNiUdpTransport->OpenUdpSocketTx(m_niApiWifiStaRemoteIpAddrTx, m_niApiWifiStaRemotePortTx);

        NI_LOG_CONSOLE_DEBUG("Initialize STA UDP Rx Socket");
        m_wifiNiUdpTransport->OpenUdpSocketRx(m_niApiWifiStaLocalPortRx, rxThreadPriority);

        // To listen the MAC SAP TX CNF messages on the port 12501 from 802.11 AFW
        if ((m_NiApiConfirmationMessage)&&(!m_enableNiApiLoopback))
        {
        	m_wifiNiUdpTxCNF = CreateObject <NiUdpTransport> ("WIFI_txCNF");

        	NI_LOG_CONSOLE_DEBUG("Initialize STA UDP TX confirmation receive Socket");
        	m_wifiNiUdpTxCNF->OpenUdpSocketRx("12502",rxThreadPriority);

        	m_wifiNiUdpTxCNF->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiTXCnfReqDataFrame, this));

        }
      }
    else if ((m_ns3WifiDevType == NS3_ADHOC)  && (m_niApiWifiDevType == NIAPI_STA1))
      {
        m_wifiNiUdpTransport = CreateObject <NiUdpTransport> ("WIFI");
        // set callback function for rx packets
        m_wifiNiUdpTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiStartRxCtrlDataFrame, this));

        // create udp transport object for STA1 in ADHOC mode
        NI_LOG_CONSOLE_DEBUG("Initialize ADHOC STA1 UDP Tx Socket ");
        m_wifiNiUdpTransport->OpenUdpSocketTx(m_niApiWifiSta1RemoteIpAddrTx, m_niApiWifiSta1RemotePortTx);

        NI_LOG_CONSOLE_DEBUG("Initialize ADHOC STA1 UDP Rx Socket");
        m_wifiNiUdpTransport->OpenUdpSocketRx(m_niApiWifiSta1LocalPortRx, rxThreadPriority);
        // To listen the MAC SAP TX CNF messages on the port 12502 from 802.11 AFW
        if ((m_NiApiConfirmationMessage)&&(!m_enableNiApiLoopback))
			{
				m_wifiNiUdpTxCNF = CreateObject <NiUdpTransport> ("WIFI_txCNF");

				NI_LOG_CONSOLE_DEBUG("Initialize STA1 UDP TX confirmation receive Socket");
				m_wifiNiUdpTxCNF->OpenUdpSocketRx("12501",rxThreadPriority);

				m_wifiNiUdpTxCNF->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiTXCnfReqDataFrame, this));

			}
      }
    else if ((m_ns3WifiDevType == NS3_ADHOC)  && (m_niApiWifiDevType == NIAPI_STA2))
      {
        m_wifiNiUdpTransport = CreateObject <NiUdpTransport> ("WIFI");
        // set callback function for rx packets
        m_wifiNiUdpTransport->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiStartRxCtrlDataFrame, this));

        // create udp transport object
        NI_LOG_CONSOLE_DEBUG("Initialize ADHOC STA2 UDP Tx Socket");
        m_wifiNiUdpTransport->OpenUdpSocketTx(m_niApiWifiSta2RemoteIpAddrTx, m_niApiWifiSta2RemotePortTx);

        NI_LOG_CONSOLE_DEBUG("Initialize ADHOC STA2 UDP Rx Socket");
        m_wifiNiUdpTransport->OpenUdpSocketRx(m_niApiWifiSta2LocalPortRx, rxThreadPriority);
        // To listen the MAC SAP TX CNF messages on the port 12502 from 802.11 AFW
        if ((m_NiApiConfirmationMessage)&&(!m_enableNiApiLoopback))
			{
				m_wifiNiUdpTxCNF = CreateObject <NiUdpTransport> ("WIFI_txCNF");

				NI_LOG_CONSOLE_DEBUG("Initialize STA2 UDP TX confirmation receive Socket");
				m_wifiNiUdpTxCNF->OpenUdpSocketRx("12502",rxThreadPriority);

				m_wifiNiUdpTxCNF->SetNiApiDataEndOkCallback (MakeCallback (&NiWifiMacInterface::NiTXCnfReqDataFrame, this));

			}
      }

    return;
  }

  void
  NiWifiMacInterface::DeInitializeNiUdpTransport()
  {
    NI_LOG_NONE (this << " " << __FILE__ << " " << __func__);

    // TODO-NI: implement de-initialization method
    //          see NiLtePhyInterface::DeInitializeNiUdpTransport () for referernce
  }

  // Writes the current ns-3 Packet (from MAC High) into a new one and adds the ns-3 WifiMacHeader to it.
  Ptr<const Packet>
  NiWifiMacInterface::NiCreateCombinedPacket(Ptr<const Packet> packet, const WifiMacHeader header)
  {
    Ptr<Packet> combinedPacket;

    // packet from the upper layer copied in the combined packet
    combinedPacket = packet->Copy ();

    // MAC header serialized to the packet
    combinedPacket->AddHeader (header);

    NI_LOG_DEBUG("NiWifiMacInterface::NiCreateCombinedPacket: "
        << " size of packet = " << packet->GetSerializedSize()
        << " size of header = " << header.GetSerializedSize()
        << " size combined packet = " << combinedPacket->GetSerializedSize());

    return combinedPacket;
  };

  // Creates a TX Config Req message.
  // Note that most of its parameters are set statically due to ensure interoperability between our ns-3
  // application example and the 802.11 AFW.
  void
  NiWifiMacInterface::NiCreateTxConfigReq(
      Ptr<const Packet> combinedPacket,
      const WifiMacHeader macHeader,
      uint32_t mcs,
      uint8_t* p_buffer,
      uint32_t* p_bufferOffset,
      uint8_t staType
  )
  {
    // extra buffers used for type conversion of MAC addresses (U8) into U32 for serialization
    uint8_t addrBuffer1[6], addrBuffer2[6], addrBuffer3[6];

    NiapiCommonHeader txConfigReqHdr;
    TxConfigReqBody txConfigReqBody;

    // =========== header declaration ===========

    txConfigReqHdr.genMsgHdr.msgType    = TX_CONFIG_REQ;
    txConfigReqHdr.genMsgHdr.refId      = 1;

    if (staType == 0)
      txConfigReqHdr.genMsgHdr.instId     = 1;
    else if (staType == 1)
      txConfigReqHdr.genMsgHdr.instId     = 2;
    else txConfigReqHdr.genMsgHdr.instId    = 0;

    // (MSDU TX param set length + param set header) + (PHY TX param set length + param set header)
    // + General Messager Header + SAP Sub-Message Header
    txConfigReqHdr.genMsgHdr.bodyLength = (txConfigReqBody.msduTxParams.parSetLength + 4) +
        (txConfigReqBody.phyTxParams.parSetLength + 4) + 8 + 8;

    txConfigReqHdr.subMsgHdr.resv       = 0;
    txConfigReqHdr.subMsgHdr.timestmp   = 0x10;
    txConfigReqHdr.subMsgHdr.numSubMsg  = 2;
    txConfigReqHdr.subMsgHdr.cnfMode    = 1;

    // ======== parameter set declaration =======

    txConfigReqBody.msduTxParams.msduIndex    = 1;

    // fill field which can be taken from the ns-3 WifiMacHeader
    if (macHeader.IsMgt())			// management frame
      {
        txConfigReqBody.msduTxParams.frameType  = 0;
        // subtype has to be mapped differently (subType cases must be written in subType)
        txConfigReqBody.msduTxParams.subType    = macHeader.GetType();
      }
    else if (macHeader.IsCtl())		// control frame
      {
        txConfigReqBody.msduTxParams.frameType = 1;		// ref: MAC Middle SAP Specification.docx, p. 21
        txConfigReqBody.msduTxParams.subType   = 0;		// control frames not supported in 802.11 AFW 2.1
      }
    else if (macHeader.IsData())	// data frame
      {
        txConfigReqBody.msduTxParams.frameType = 2;
        txConfigReqBody.msduTxParams.subType   = 0;		// ref: MAC Middle SAP Specification.docx, p. 21
      }

    if (macHeader.IsFromDs())
      txConfigReqBody.msduTxParams.fromDs = 1;
    //else txConfigReqBody.msduTxParams.fromDs = 0;
    if (macHeader.IsToDs())
      txConfigReqBody.msduTxParams.toDs = 1;
    //else txConfigReqBody.msduTxParams.toDs = 0;

    // copy MAC addresses into U8 buffers
    macHeader.GetAddr1().CopyTo(addrBuffer1);
    macHeader.GetAddr2().CopyTo(addrBuffer2);
    macHeader.GetAddr3().CopyTo(addrBuffer3);

    // convert buffers into U32 to make them work with SerializeMessage
    for (uint32_t i = 0; i<6; i++)
      {
        txConfigReqBody.msduTxParams.destAddr [i]   = (uint32_t) addrBuffer1[i];
        txConfigReqBody.msduTxParams.sourceAddr [i] = (uint32_t) addrBuffer2[i];
        txConfigReqBody.msduTxParams.bssid [i]      = (uint32_t) addrBuffer3[i];
      }

    //fill with recipient station or transmitter station MAC address only if toDs AND fromDs == 1
    //if (txConfigReqBody.msduTxParams.fromDs && txConfigReqBody.msduTxParams.toDs)

    txConfigReqBody.msduTxParams.msduLength = (uint16_t) combinedPacket->GetSerializedSize();

    txConfigReqBody.phyTxParams.msduIndex     = 1;
    txConfigReqBody.phyTxParams.format        = 2;
    txConfigReqBody.phyTxParams.bandwidth     = 0;
    txConfigReqBody.phyTxParams.mcs           = mcs;

    // ==========================================

    SerializeMessage(&txConfigReqHdr, (uint32_t*) &txConfigReqBody, p_buffer, p_bufferOffset);
  }

  // Creates a TX Payload Req message.
  // Note that most of its parameters are set statically due to ensure interoperability between our ns-3
  // application example and the 802.11 AFW.
  void
  NiWifiMacInterface::NiCreateTxPayloadReq(
      Ptr<const Packet> combinedPacket,
      uint8_t* p_buffer,
      uint32_t* p_bufferOffset,
      uint8_t staType
  )
  {
    // extra buffer used for type conversion of MSDU data (packet in U8) into U32 for serialization
    uint8_t msduDataBuffer[4065];

    NiapiCommonHeader txPayloadReqHdr;
    TxPayloadReqBody txPayloadReqBody;

    // determine MSDU length (here the length of the ns-3 packet in bytes)
    uint32_t msduLength = combinedPacket->GetSerializedSize();

    // =========== header declaration ===========

    txPayloadReqHdr.genMsgHdr.msgType    	 = TX_PAYLOAD_REQ;
    txPayloadReqHdr.genMsgHdr.refId          = 1;

    if (staType == 0)
      txPayloadReqHdr.genMsgHdr.instId     = 1;
    else if (staType == 1)
      txPayloadReqHdr.genMsgHdr.instId     = 2;
    else txPayloadReqHdr.genMsgHdr.instId    = 0;

    // MSDU+ General Messager Header + SAP Sub-Message Header
    txPayloadReqHdr.genMsgHdr.bodyLength = msduLength + 4 + 8 + 8;

    txPayloadReqHdr.subMsgHdr.timestmp          = 0x20;
    txPayloadReqHdr.subMsgHdr.numSubMsg  		= 1;
    txPayloadReqHdr.subMsgHdr.cnfMode    		= 1;

    // ======== parameter set declaration =======

    txPayloadReqBody.msduTxPayload.msduIndex    = 1;
    txPayloadReqBody.msduTxPayload.parSetLength = msduLength + 4;
    txPayloadReqBody.msduTxPayload.msduLength	= msduLength;

    // write content of ns-3 packet into U8 buffer
    combinedPacket->Serialize(msduDataBuffer, msduLength);

    // convert buffer into U32 to make it work with SerializeMessage
    for (uint32_t i = 0; i<msduLength; i++) txPayloadReqBody.msduTxPayload.msduData [i] = (uint32_t) msduDataBuffer [i];

    // ==========================================

    SerializeMessage(&txPayloadReqHdr, (uint32_t*) &txPayloadReqBody, p_buffer, p_bufferOffset);

  }

  void
  NiWifiMacInterface::NiStartTxCtrlDataFrame(Ptr<const Packet> packet, WifiMacHeader hdr)
  {
    if ((m_ns3WifiDevType == NS3_AP) && ((m_niApiWifiDevType == NIAPI_AP)||(m_niApiWifiDevType==NIAPI_WIFI_ALL)))
      {
        NI_LOG_DEBUG ("AP Tx Start with packet of size = " << packet->GetSerializedSize() << " bytes");


        /*	Because the 802.11 AFW expects two messages (TX Configuration Request, afterwards TX Payload Req)
         * 	our application example must send two UDP datagrams per ns-3 packet.
         *
         * 	The TX Configuration Request message takes some of the WifiMacHeader's parameters and writes them
         *	into its own message body.
         *
         *	The MSDU data of a TX Payload Request is a combined packet, consisting of the ns-3 WifiMacHeader and
         *	the ns-3 packet.
         */

        // create combined packet consisting of the ns-3 WifiMacHeader and the ns-3 packet
        Ptr<const Packet> combinedPacket = NiCreateCombinedPacket(packet, hdr);

        // modify the header's MAC addresses to make it work with 802.11 AFW
        // Note that the original WifiMacHeader contents are still available in combinedPacket!
        if(!m_enableNiApiLoopback)
          {
            hdr.SetAddr1 (Mac48Address(m_niApiWifiStaMacAddress.c_str()));	//assign destination address
            hdr.SetAddr2 (Mac48Address(m_niApiWifiApMacAddress.c_str()));	//assign source address
            hdr.SetAddr3 (Mac48Address(m_niApiWifiBssidMacAddress.c_str()));
          }

        // create TX Configuration Request message
        // Note: Station type is hard-coded to zero.
        NiCreateTxConfigReq(combinedPacket, hdr, m_niApiWifiMcs, m_bufferTx, &m_bufferOffsetTx, 0);

        if (m_niApiWifiEnablePrintMsgContent && hdr.IsData())
          {
            // print packet contents
            PrintBufferU8(m_bufferTx, &m_bufferOffsetTx, 36);
          }

        // send serialized TX Configuration Request message to the UDP socket
        if(!m_enableNiApiLoopback)
          {
            m_wifiNiUdpTransport->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);
          }
        else
          {
            for (uint8_t index = 0; index < m_NumOfStations; ++index)
              {
                m_wifiNiUdpTransportArray[index]->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);
              }
          }

        // reset buffer
        m_bufferOffsetTx = 0;

        // create TX Payload Request message
        // Note: Station type is hard-coded to zero.
        NiCreateTxPayloadReq(combinedPacket, m_bufferTx, &m_bufferOffsetTx, 0);

        if (m_niApiWifiEnablePrintMsgContent && hdr.IsData())
          {
            // print packet contents
            PrintBufferU8(m_bufferTx, &m_bufferOffsetTx, 34);
          }

        // send serialized TX Configuration Request message to the UDP socket
        if(!m_enableNiApiLoopback)
          {
            m_wifiNiUdpTransport->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);
          }
        else
          {
            for (uint8_t index = 0; index < m_NumOfStations; ++index)
              {
                m_wifiNiUdpTransportArray[index]->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);
              }
          }

        // reset buffer
        m_bufferOffsetTx = 0;
      }
    else if ((m_ns3WifiDevType == NS3_STA)  && ((m_niApiWifiDevType == NIAPI_STA)||(m_niApiWifiDevType==NIAPI_WIFI_ALL)))
      {
        NI_LOG_DEBUG ("STA Tx Start with packet of size = " << packet->GetSerializedSize() << " bytes");

        // create combined packet consisting of the ns-3 WifiMacHeader and the ns-3 packet
        Ptr<const Packet> combinedPacket = NiCreateCombinedPacket(packet, hdr);

        // modify the header's MAC addresses to make it work with 802.11 AFW
        // Note that the original WifiMacHeader contents are still available in combinedPacket!
        if(!m_enableNiApiLoopback)
          {
            hdr.SetAddr1 (Mac48Address(m_niApiWifiApMacAddress.c_str()));			//assign destination address
            hdr.SetAddr2 (Mac48Address(m_niApiWifiStaMacAddress.c_str()));		//assign source address
            hdr.SetAddr3 (Mac48Address(m_niApiWifiBssidMacAddress.c_str()));
          }

        // create TX Configuration Request message
        // Note: Station type is hard-coded to zero.
        NiCreateTxConfigReq(combinedPacket, hdr, m_niApiWifiMcs, m_bufferTx, &m_bufferOffsetTx, 1);

        if (m_niApiWifiEnablePrintMsgContent)
          {
            // print packet contents
            PrintBufferU8(m_bufferTx, &m_bufferOffsetTx, 36);
          }

        // send serialized TX Configuration Request message to the UDP socket
        //NIWifiApiSendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);
        m_wifiNiUdpTransport->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);

        // reset buffer
        m_bufferOffsetTx = 0;

        // create TX Payload Request message
        // Note: Station type is hard-coded to zero.
        NiCreateTxPayloadReq(combinedPacket, m_bufferTx, &m_bufferOffsetTx, 1);

        if (m_niApiWifiEnablePrintMsgContent)
          {
            // print packet contents
            PrintBufferU8(m_bufferTx, &m_bufferOffsetTx, 34);
          }

        // send serialized TX Configuration Request message to the UDP socket
        //NIWifiApiSendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);
        m_wifiNiUdpTransport->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);

        // reset buffer
        m_bufferOffsetTx = 0;
      }
    else if (m_ns3WifiDevType == NS3_ADHOC)
      {

        NI_LOG_DEBUG("ADHOC Tx Start with packet of size = " << packet->GetSerializedSize() << " bytes");

        uint8_t adhoc_staType = 0; //local variable for TX Configuration Request message. Initialized to 0.

        // create combined packet consisting of the ns-3 WifiMacHeader and the ns-3 packet
        Ptr<const Packet> combinedPacket = NiCreateCombinedPacket(packet, hdr);

        // modify the header's MAC addresses to make it work with 802.11 AFW
        // Note that the original WifiMacHeader contents are still available in combinedPacket!
        if(!m_enableNiApiLoopback)
          {
            if (m_niApiWifiDevType == NIAPI_STA1)
              {
                hdr.SetAddr1 (Mac48Address(m_niApiWifiSta2MacAddr.c_str()));	//assign destination address
                hdr.SetAddr2 (Mac48Address(m_niApiWifiSta1MacAddr.c_str()));	//assign source address
                adhoc_staType = 0; // Set to 0 for station 1
              }
            else if (m_niApiWifiDevType == NIAPI_STA2)
              {
                hdr.SetAddr1 (Mac48Address(m_niApiWifiSta1MacAddr.c_str()));	//assign destination address
                hdr.SetAddr2 (Mac48Address(m_niApiWifiSta2MacAddr.c_str()));	//assign source address
                adhoc_staType = 1; // Set to 1 for station 2
              }

            hdr.SetAddr3 (Mac48Address(m_niApiWifiBssidMacAddress.c_str()));
          }

        // create TX Configuration Request message
        NiCreateTxConfigReq(combinedPacket, hdr, m_niApiWifiMcs, m_bufferTx, &m_bufferOffsetTx, adhoc_staType);

        if (m_niApiWifiEnablePrintMsgContent)
          {
            // print packet contents
            PrintBufferU8 (m_bufferTx, &m_bufferOffsetTx, 36);
          }

        // send serialized TX Configuration Request message to the UDP socket
        m_wifiNiUdpTransport->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);

        // reset buffer
        m_bufferOffsetTx = 0;

        // create TX Payload Request message
        NiCreateTxPayloadReq(combinedPacket, m_bufferTx, &m_bufferOffsetTx, adhoc_staType);

        // print packet contents
        if (m_niApiWifiEnablePrintMsgContent)
          {
            PrintBufferU8 (m_bufferTx, &m_bufferOffsetTx, 34);
          }

        // send serialized TX Configuration Request message to the UDP socket
        m_wifiNiUdpTransport->SendToUdpSocketTx(m_bufferTx, m_bufferOffsetTx);

        // reset buffer
        m_bufferOffsetTx = 0;

      }else {
          // do nothing
      }
  }

  // Identifies the received messages type ID by extracting the first four bytes.
  uint16_t
  NiWifiMacInterface::GetMsgTypeId (uint8_t* p_buffer)
  {
    uint16_t msgTypeId = (uint16_t) p_buffer[1] | ( ((uint16_t) p_buffer[0]) << 8);
    return msgTypeId;
  }

  // Deserializes a RX Config Ind message from a received buffer.
  RxConfigIndBody
  NiWifiMacInterface::DeserializeRxConfigInd(uint8_t* p_buffer, uint32_t* p_bufferOffset)
  {
    NiapiCommonHeader rxConfigIndHdr;
    RxConfigIndBody rxConfigIndBody;

    DeserializeMessage(&rxConfigIndHdr, (uint32_t*) &rxConfigIndBody, p_buffer, p_bufferOffset);

    return rxConfigIndBody;
  }

  // Deserializes a RX Payload Ind message from a received buffer.
  RxPayloadIndBody
  NiWifiMacInterface::DeserializeRxPayloadInd(uint8_t* p_buffer, uint32_t* p_bufferOffset)
  {
    NiapiCommonHeader rxPayloadIndHdr;
    RxPayloadIndBody rxPayloadIndBody;

    DeserializeMessage(&rxPayloadIndHdr, (uint32_t*) &rxPayloadIndBody, p_buffer, p_bufferOffset);

    return rxPayloadIndBody;
  }

  // Deserializes a TX Confirmation Ind message from a received buffer.
   TxCnfBody
   NiWifiMacInterface::DeserializeTxCnfInd(uint8_t* p_buffer, uint32_t* p_bufferOffset)
   {
 	  NiapiCommonHeader txCnfIndHdr; //Common header as specified in 802.11 MAC specification.
 	  TxCnfBody txCnfIndBody;

 	  DeserializeMessage(&txCnfIndHdr, (uint32_t*) &txCnfIndBody, p_buffer, p_bufferOffset);

 	  switch (txCnfIndBody.cnfStatus)
 	  {
 		case WIFI_CNF_SUCCESS:
 		  NI_LOG_DEBUG("WIFI_CNF_SUCCESS");
 		  break;
 		case WIFI_CNF_UNKNOWN_MESSAGE:
 		  NI_LOG_WARN("WIFI_CNF_UNKNOWN_MESSAGE");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_UNKNOWN_MESSAGE");
 		  break;
 		case WIFI_CNF_MESSAGE_NOT_SUPPORTED:
 		  NI_LOG_WARN("WIFI_CNF_MESSAGE_NOT_SUPPORTED");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_MESSAGE_NOT_SUPPORTED");
 		  break;
 		case WIFI_CNF_UNKNOWN_PARAMETER_SET:
 		  NI_LOG_WARN("WIFI_CNF_UNKNOWN_PARAMETER_SET");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_UNKNOWN_PARAMETER_SET");
 		  break;
 		case WIFI_CNF_MISSING_PARAMETER_SET:
 		  NI_LOG_WARN("WIFI_CNF_MISSING_PARAMETER_SET");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_MISSING_PARAMETER_SET");
 		  break;
 		case WIFI_CNF_PARAMETER_SET_REPETITION:
 		  NI_LOG_WARN("WIFI_CNF_PARAMETER_SET_REPETITION");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_PARAMETER_SET_REPETITION");
 		  break;
 		case WIFI_CNF_RANGE_VIOLATION:
 		  NI_LOG_WARN("WIFI_CNF_RANGE_VIOLATION");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_RANGE_VIOLATION");
 		  break;
 		case WIFI_CNF_STATE_VIOLATION:
 		  NI_LOG_WARN("WIFI_CNF_STATE_VIOLATION");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_STATE_VIOLATION");
 		  break;
 		case WIFI_CNF_TIMEOUT:
 		  NI_LOG_WARN("WIFI_CNF_TIMEOUT");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_TIMEOUT");
 		  break;
 		case WIFI_CNF_CONFIG_PAYLOAD_MISMATCH:
 		  NI_LOG_WARN("WIFI_CNF_CONFIG_PAYLOAD_MISMATCH");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_CONFIG_PAYLOAD_MISMATCH");
 		  break;
 		case WIFI_CNF_LENGTH_MISMATCH:
 		  NI_LOG_WARN("WIFI_CNF_LENGTH_MISMATCH");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_LENGTH_MISMATCH");
 		  break;
 		case WIFI_CNF_INPUT_BUFFER_FULL:
 		  NI_LOG_WARN("WIFI_CNF_INPUT_BUFFER_FULL");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_INPUT_BUFFER_FULL");
 		  break;
 		case WIFI_CNF_INTERNAL_ERROR:
 		  NI_LOG_WARN("WIFI_CNF_INTERNAL_ERROR");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_INTERNAL_ERROR");
 		  break;
 		case WIFI_CNF_INSTANCE_ID_MISMATCH:
 		  NI_LOG_WARN("WIFI_CNF_INSTANCE_ID_MISMATCH");
 		  NI_LOG_CONSOLE_INFO("WIFI_CNF_INSTANCE_ID_MISMATCH");
 		  break;
 		default:
 		  NI_LOG_FATAL("Received UNKNOWN confirm. WIFI_CnfStatus=" << txCnfIndBody.cnfStatus);
 		  break;
 	  }

 	  return txCnfIndBody;
   }

  // Converts a TX Config Req into RX Config Ind.
  // It is needed when running the application example in UDP Loopback mode, where TX Req messages are sent over
  // directly from one ns-3 MAC High to the other.
  void
  NiWifiMacInterface::ConvertTxConfigReq(uint8_t* p_buffer, uint32_t* p_bufferOffset)
  {
    NiapiCommonHeader txConfigReqHdr;
    TxConfigReqBody txConfigReqBody;

    NiapiCommonHeader rxConfigIndHdr;
    RxConfigIndBody rxConfigIndBody;

    DeserializeMessage(&txConfigReqHdr, (uint32_t*) &txConfigReqBody, p_buffer, p_bufferOffset);

    rxConfigIndHdr.genMsgHdr.msgType = RX_CONFIG_IND;
    rxConfigIndHdr.genMsgHdr.refId = txConfigReqHdr.genMsgHdr.refId;
    rxConfigIndHdr.genMsgHdr.bodyLength = txConfigReqHdr.genMsgHdr.bodyLength;

    rxConfigIndHdr.subMsgHdr.timestmp = txConfigReqHdr.subMsgHdr.timestmp;
    rxConfigIndHdr.subMsgHdr.numSubMsg = 4;
    rxConfigIndHdr.subMsgHdr.cnfMode = 0;

    rxConfigIndBody.msduRxParams = txConfigReqBody.msduTxParams;
    rxConfigIndBody.phyRxParams = txConfigReqBody.phyTxParams;

    *p_bufferOffset = 0;

    SerializeMessage(&rxConfigIndHdr, (uint32_t*) &rxConfigIndBody, p_buffer, p_bufferOffset);
  }

  // Converts a TX Payload Req into RX Payload Ind.
  // It is needed when running the application example in UDP Loopback mode, where TX Req messages are sent over
  // directly from one ns-3 MAC High to the other.
  void
  NiWifiMacInterface::ConvertTxPayloadReq(uint8_t* p_buffer, uint32_t* p_bufferOffset)
  {
    NiapiCommonHeader txPayloadReqHdr;
    TxPayloadReqBody txPayloadReqBody;

    NiapiCommonHeader rxPayloadIndHdr;
    RxPayloadIndBody rxPayloadIndBody;

    DeserializeMessage(&txPayloadReqHdr, (uint32_t*) &txPayloadReqBody, p_buffer, p_bufferOffset);

    rxPayloadIndHdr.genMsgHdr.msgType = RX_PAYLOAD_IND;
    rxPayloadIndHdr.genMsgHdr.refId = txPayloadReqHdr.genMsgHdr.refId;
    rxPayloadIndHdr.genMsgHdr.bodyLength = txPayloadReqHdr.genMsgHdr.bodyLength;

    rxPayloadIndHdr.subMsgHdr.timestmp = txPayloadReqHdr.subMsgHdr.timestmp;
    rxPayloadIndHdr.subMsgHdr.numSubMsg = 1;
    rxPayloadIndHdr.subMsgHdr.cnfMode = 0;

    rxPayloadIndBody.msduRxPayload = txPayloadReqBody.msduTxPayload;

    *p_bufferOffset = 0;

    SerializeMessage(&rxPayloadIndHdr, (uint32_t*) &rxPayloadIndBody, p_buffer, p_bufferOffset);
  }

  // Separates the ns-3 WifiMacHeader and the ns-3 Packet (from MAC High) from each other.
  void
  NiWifiMacInterface::SeparateCombinedPacket(Ptr<Packet> combinedPacket)
  {
    Ptr<Packet> packet;
    packet = combinedPacket->Copy();

    WifiMacHeader macHeader;

    packet->RemoveHeader(macHeader);

    m_rxMacHeader = macHeader;
    m_rxPacket = packet->Copy();

    return;
  }

  // Accesses a pointer to the extracted ns-3 WifiMacHeader.
  WifiMacHeader*
  NiWifiMacInterface::GetMacHeaderPtr ()
  {
    return &m_rxMacHeader;
  }

  // Accesses a pointer to the extracted ns-3 Packet (from MAC High).
  Ptr<Packet>
  NiWifiMacInterface::GetPacket ()
  {
    return m_rxPacket;
  }

  // Waits in a loop for any packets to be received, processes the packet contents (according to the functionality of
  // 802.11 AFW's MAC High) and propagates the for ns-3 necessary WifiMacHeader and Packet to the ns-3 Receive function.
  bool
  NiWifiMacInterface::NiStartRxCtrlDataFrame(uint8_t* m_bufferRx )
  {
    NI_LOG_DEBUG("Received data packet");

    /*	On the RX side we must distinguish between two cases regarding the UDP Loopback mode:
     *
     *	1) UDP Loopback mode enabled:
     *			The RX socket only receives TX Config Req and TX Payload Req messages that were sent
     *			from another ns-3 MAC High. Those Request messages must furthermore be converted to
     *			match the expected RX Config Ind and RX Payload Ind message formats.
     *			Therefore, the received message are deserialized first. The message body parameters that are also
     *			present in the Ind messages are used to generate those Ind messages, which are finally serialized
     *			again.
     *
     *	2) UDP Loopback mode disabled:
     *			The RX socket expects to receive RX Config Ind and RX Payload Ind messages
     *			from the 802.11 AFW.
     *
     *	In both cases the processing of the buffers containing the serialized messages is performed
     *	depending on the message header's message type ID. Also, a RX Payload Ind is not processed further
     *	if there was not any RX Config Ind payload received beforehand.
     *
     *	Generally, most of the message's parameters are not used after deserializing, except of the MSDU data
     *	and the MSDU length. These are utilized to recover the ns-3 WifiMacHeader and ns-3 Packet.
     */

    // if UDP Loopback mode is enabled, all received TX Request messages must be converted to RX Indication messages
    if (m_enableNiApiLoopback)
      {
        if (GetMsgTypeId(m_bufferRx) == TX_CONFIG_REQ)
          ConvertTxConfigReq(m_bufferRx, &m_bufferOffsetRx);

        else if (GetMsgTypeId(m_bufferRx) == TX_PAYLOAD_REQ)
          ConvertTxPayloadReq(m_bufferRx, &m_bufferOffsetRx);

        else	NI_LOG_CONSOLE_DEBUG("NI.WIFI.MAC.IF: Received unknown message! Message type ID does not match neither"
            << " TX Configuration Request (0x5101) nor TX Payload Request (0x5102)!");
      }

    // reset buffer offset
    m_bufferOffsetRx = 0;

    // Depending on the message header's message type ID either RX Config Ind or RX Payload Ind is deserialized.
    // Also, the order of the messages received is important: Using the control variables m_configIndReceived and
    // m_configIndReceived, a RX Payload Ind can only be further processed if a RX Config Ind was received beforehand.

    if (GetMsgTypeId(m_bufferRx) == RX_CONFIG_IND)
      {
        NI_LOG_DEBUG("NiWifiMacInterface::NiStartRxCtrlDataFrame: RX Config Ind received");

        m_rxConfigIndBody = DeserializeRxConfigInd(m_bufferRx, &m_bufferOffsetRx);
        m_configIndReceived = true;
      }

    else if (GetMsgTypeId(m_bufferRx) == RX_PAYLOAD_IND)
      {
        NI_LOG_DEBUG("NiWifiMacInterface::NiStartRxCtrlDataFrame: RX Payload Ind received");

        if (m_configIndReceived)
          {
            m_rxPayloadIndBody = DeserializeRxPayloadInd(m_bufferRx, &m_bufferOffsetRx);

            // The payload must be written in a separate buffer, that will be used to derive
            // the ns-3 WifiMacHeader and Packet.
            m_payloadBuffer = m_bufferRx + 24;
            m_msduLength = m_rxPayloadIndBody.msduRxPayload.msduLength;

            m_payloadIndReceived = true;
          }

        else NI_LOG_DEBUG("Received RX Payload Indication, but RX Configuration Indication is awaited.\n"
            << "The current packet will be dropped!\n\n");
      }

    else NI_LOG_CONSOLE_DEBUG("NI.WIFI.MAC.IF: Received unknown message! Message type ID does not match neither"
        << " RX Configuration Indication (0x5081) nor RX Payload Indication (0x5082)!\n\n");

    if (m_payloadIndReceived)
      {
        // write received payload buffer (serialized ns-3 WifiMacHeader and packet) into one new packet
        Ptr<Packet> combinedPacket = Create<Packet> ((uint8_t const*)m_payloadBuffer, m_msduLength, true);

        NI_LOG_DEBUG("Received packet of size " << combinedPacket->GetSerializedSize() << " bytes")

        if (m_niApiWifiEnablePrintMsgContent)
          {
            // print received payload
            PrintBufferU8(m_payloadBuffer, &m_msduLength, 32);
          }

        // check for parameter mismatches between MSDU indices and MSDU length values of the RX indications
        // if mismatch detected, drop current packet
        if ((m_rxConfigIndBody.msduRxParams.msduIndex  == m_rxPayloadIndBody.msduRxPayload.msduIndex) &&
            (m_rxConfigIndBody.msduRxParams.msduLength == m_rxPayloadIndBody.msduRxPayload.msduLength)	)
          {
            NI_LOG_DEBUG("NiWifiMacInterface::NiStartRxCtrlDataFrame: separate packet and header");

            // separate ns-3 WifiMacHeader and ns-3 Packet
            SeparateCombinedPacket(combinedPacket);

            // trigger Receive function to continue in "normal" ns-3 program flow
            //Receive (GetPacket(), GetMacHeaderPtr());
            m_NiApWifiRxDataEndOkCallback(GetPacket(), GetMacHeaderPtr());

          }

        else NI_LOG_CONSOLE_DEBUG("NI.WIFI.MAC.IF: Parameter mismatch between MSDU indices or MSDU length values of the RX Indications!"
            << "The current packet will be dropped!\n\n");

        // reset control variables for next packets to be received
        m_configIndReceived = 0;
        m_payloadIndReceived = 0;
        m_msduLength = 0;
        m_rxConfigIndBody = {};
        m_rxPayloadIndBody = {};
      }

    // reset buffer offset
    m_bufferOffsetRx = 0;

  }

  void
  NiWifiMacInterface::SetNiApWifiRxDataEndOkCallback (NiApWifiRxDataEndOkCallback c)
  {
    //NI_LOG_CONSOLE_DEBUG("EndOkCallback is here");
    m_NiApWifiRxDataEndOkCallback = c;
  }

  bool
  NiWifiMacInterface::NiTXCnfReqDataFrame (uint8_t* m_bufferRx )
  {
	NI_LOG_DEBUG("NiWifiMacInterface::NiTXCnfReqDataFrame");

	if(!m_enableNiApiLoopback)
	  {
		//Confirmation status implemented for Infra Access point and Adhoc Station 1.
		if (GetMsgTypeId(m_bufferRx) == 0x5201) // MAC SAP TX CNF message defination from 802.11 MAC middle SAP specification
			  {
				NI_LOG_DEBUG("NiWifiMacInterface::NiTXCnfReqDataFrame: message TypeID matched.");

				m_txCnfBody = DeserializeTxCnfInd(m_bufferRx, &m_bufferOffsetRx);

				m_bufferOffsetRx = 0;

			  }
			else
			NS_FATAL_ERROR ("TX confirmation message TypeID mismatch from AFW");
		 }
     }
}
