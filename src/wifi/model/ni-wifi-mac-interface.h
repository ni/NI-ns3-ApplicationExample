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

#ifndef NI_WIFI_MAC_INTERFACE_H_
#define NI_WIFI_MAC_INTERFACE_H_

#include "ns3/object.h"
#include "ns3/mac48-address.h"
#include "ns3/packet.h"
#include "ns3/traced-value.h"
#include "ns3/wifi-mode.h"
#include "wifi-mac-header.h"
#include "qos-utils.h"
//#include "adhoc-wifi-mac.h"
#include "regular-wifi-mac.h"
#include <netinet/in.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <netdb.h>

#include <map>
#include <list>

#include "ns3/ni.h"

namespace ns3{

  // device identifier
  typedef enum {
    NS3_AP  = 0,
    NS3_STA = 1,
    NS3_ADHOC = 2
  } Ns3WifiDevType_t;

  // device identifier
  typedef enum {
    NIAPI_AP  = 0,
    NIAPI_STA = 1,
    NIAPI_WIFI_ALL = 2,
    NIAPI_STA1 = 3,
    NIAPI_STA2 = 4,
    NIAPI_WIFI_NONE = 5
  } NiApiWifiDevType_t;

  typedef Callback< void, Ptr<Packet>, const WifiMacHeader *> NiApWifiRxDataEndOkCallback;

  class NiWifiMacInterface : public Object
  {
  public:
    NiWifiMacInterface( );

    NiWifiMacInterface(Ns3WifiDevType_t ns3WifiDevType);

    virtual ~NiWifiMacInterface();

    virtual void DoDispose (void);
    virtual void DoInitialize (void);

    static TypeId GetTypeId (void);

    void SetNiApWifiRxDataEndOkCallback (NiApWifiRxDataEndOkCallback c);

    NiApiWifiDevType_t GetNiWifiDevType () const;
    Ns3WifiDevType_t GetNs3DevType () const;
    bool GetNiApiEnable () const;

    void NiStartTxCtrlDataFrame(Ptr<const Packet> packet, WifiMacHeader hdr);

    // Writes the current ns-3 Packet (from MAC High) into a new one and adds the ns-3 WifiMacHeader to it.
    Ptr<const Packet> NiCreateCombinedPacket (Ptr<const Packet> packet, const WifiMacHeader hdr);

    // Creates a TX Config Req message.
    // Note that most of its parameters are set statically due to ensure interoperability between our ns-3
    // application example and the 802.11 AFW.
    void NiCreateTxConfigReq(
        Ptr<const Packet> combinedPacket,
        const WifiMacHeader macHeader,
        uint32_t mcs,
        uint8_t* p_buffer,
        uint32_t* p_bufferOffset,
        uint8_t staType
    );

    // Creates a TX Payload Req message.
    // Note that most of its parameters are set statically due to ensure interoperability between our ns-3
    // application example and the 802.11 AFW.
    void NiCreateTxPayloadReq(
        Ptr<const Packet> combinedPacket,
        uint8_t* p_buffer,
        uint32_t* p_bufferOffset,
        uint8_t staType
    );

  private:

    void SetNiWifiDevType (std::string type);
    void SetNiApiEnable (bool enable);
    void SetNiApiLoopbackEnable (bool enable);


    void InitializeNiUdpTransport();
    void DeInitializeNiUdpTransport();

    // Identifies the received messages type ID by extracting the first four bytes.
    uint16_t GetMsgTypeId (uint8_t* p_buffer);

    // Deserializes a RX Config Ind message from a received buffer.
    RxConfigIndBody DeserializeRxConfigInd(uint8_t* p_buffer, uint32_t* p_bufferOffset);
    // Deserializes a RX Payload Ind message from a received buffer.
    RxPayloadIndBody DeserializeRxPayloadInd(uint8_t* p_buffer, uint32_t* p_bufferOffset);
    // Deserializes a TX Confirmation Ind message from a received buffer.
     TxCnfBody DeserializeTxCnfInd(uint8_t* p_buffer, uint32_t* p_bufferOffset);

    // Converts a TX Config Req into RX Config Ind.
    // It is needed when running the application example in UDP Loopback mode, where TX Req messages are sent over
    // directly from one ns-3 MAC High to the other.
    void ConvertTxConfigReq(uint8_t* p_buffer, uint32_t* p_bufferOffset);
    // Converts a TX Payload Req into RX Payload Ind.
    void ConvertTxPayloadReq(uint8_t* p_buffer, uint32_t* p_bufferOffset);

    // Separates the ns-3 WifiMacHeader and the ns-3 Packet (from MAC High) from each other.
    void SeparateCombinedPacket(Ptr<Packet> combinedPacket);

    // Accesses a pointer to the extracted ns-3 WifiMacHeader.
    WifiMacHeader* GetMacHeaderPtr ();
    // Accesses a pointer to the extracted ns-3 Packet (from MAC High).
    Ptr<Packet> GetPacket ();

    bool NiStartRxCtrlDataFrame (uint8_t* m_bufferRx );

    bool NiTXCnfReqDataFrame (uint8_t* m_bufferRx );

    NiApWifiRxDataEndOkCallback m_NiApWifiRxDataEndOkCallback;

    bool m_enableNiApi;
    bool m_enableNiApiLoopback;

    Ns3WifiDevType_t m_ns3WifiDevType;
    NiApiWifiDevType_t m_niApiWifiDevType;

    bool m_niApiWifiEnablePrintMsgContent;

    uint32_t m_NumOfStations;

    Ptr<NiUdpTransport> m_wifiNiUdpTransport; // TODO-NI: remove

    Ptr<NiUdpTransport> m_wifiNiUdpTransportArray[10]; // TODO-NI: replace magic number by define / constant value which can be used for sanity checks

    Ptr<NiUdpTransport> m_wifiNiUdpTxCNF;

    std::string m_niApiWifiApRemoteIpAddrTx;
    std::string m_niApiWifiApRemotePortTx;
    std::string m_niApiWifiApLocalPortRx;

    std::string m_niApiWifiStaRemoteIpAddrTx;
    std::string m_niApiWifiStaRemotePortTx;
    std::string m_niApiWifiStaLocalPortRx;

    std::string m_niApiWifiApMacAddress;
    std::string m_niApiWifiStaMacAddress;
    std::string m_niApiWifiBssidMacAddress;

    // IP address and port variables used for socket creation MAC High of ADHOC module
    std::string m_niApiWifiSta1RemoteIpAddrTx;
    std::string m_niApiWifiSta1RemotePortTx;
    std::string m_niApiWifiSta1LocalPortRx;

    std::string m_niApiWifiSta2RemoteIpAddrTx;
    std::string m_niApiWifiSta2RemotePortTx;
    std::string m_niApiWifiSta2LocalPortRx;

    std::string m_niApiWifiSta1MacAddr;
    std::string m_niApiWifiSta2MacAddr;

    uint32_t m_niApiWifiMcs;

    uint8_t  m_bufferTx[9000];
    //uint8_t  m_bufferRx[9500];
    uint32_t m_bufferOffsetTx = 0;
    uint32_t m_bufferOffsetRx = 0;

    uint8_t* m_payloadBuffer;
    uint32_t m_msduLength;

    RxConfigIndBody m_rxConfigIndBody;
    RxPayloadIndBody m_rxPayloadIndBody;

    bool m_configIndReceived = false;
    bool m_payloadIndReceived = false;

    bool NIAPIIsTxEndPointOpen;
    bool NIAPIIsRxEndPointOpen;

    WifiMacHeader m_rxMacHeader;
    Ptr<Packet> m_rxPacket;

    bool m_initializationDone;

    bool m_NiApiConfirmationMessage = true;
    TxCnfBody m_txCnfBody;
  };

}

#endif /* NI_WIFI_MAC_INTERFACE_H_ */
