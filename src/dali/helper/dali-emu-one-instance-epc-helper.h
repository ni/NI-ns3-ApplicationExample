/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
 * Copyright (c) 2019, Universitat Politecnica de Catalunya
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Support for real S1AP link
 *          	Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 *          Support for emulation S1-U, S1-MME ,X2 and DCX links
 */

#ifndef DALI_EMU_ONE_INSTANCE_EPC_HELPER_H
#define DALI_EMU_ONE_INSTANCE_EPC_HELPER_H

#include <ns3/lte-module.h>
#include <ns3/object.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/data-rate.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/epc-helper.h>

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class EpcSgwPgwApplication;
class EpcX2;
class EpcUeNas;
class EpcMmeApplication;
class EpcS1apEnb;
class EpcS1apMme;
class DaliUeDcx;

/**
 * \ingroup lte
 *
 * \brief Create an EPC network using EmuFdNetDevice
 *
 * This Helper will create an EPC network topology comprising of a
 * single node that implements both the SGW and PGW functionality, and
 * an MME node. The S1-U, S1-AP, X2-U, X2-C, DCX-U and DCX-C interfaces
 * are realized using EmuFdNetDevice; in particular, one device is used
 * to send all the traffic related to these interfaces.
 */
class DaliEmuOneInstanceEpcHelper : public EpcHelper
{
public:

  /**
   * Constructor
   */
	DaliEmuOneInstanceEpcHelper ();

  /**
   * Destructor
   */
  virtual ~DaliEmuOneInstanceEpcHelper ();

  // inherited from Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId () const;
  virtual void DoInitialize ();
  virtual void DoDispose ();

  // inherited from EpcHelper
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice, uint16_t cellId);
  virtual void AddUe (Ptr<NetDevice> ueLteDevice, uint64_t imsi);
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueLteDevice, uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueLteDevice, Ptr<EpcUeNas> ueNas, uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer);
  virtual Ptr<Node> GetPgwNode ();
  virtual Ipv4InterfaceContainer AssignUeIpv4Address (NetDeviceContainer ueDevices);
  virtual Ipv4Address GetUeDefaultGatewayAddress ();

  //New for DALI Dual Connectivity
  virtual void AddDaliUe (Ptr<Node> ue, Ptr<NetDevice> ueLteDevice, uint64_t imsi);
  virtual void AddDcxInterface (Ptr<Node> ueNode1, Ptr<Node> ueNode2);

private:

  /**
   * IPv4 addresses mask for devices
   */
  Ipv4Mask m_ipMask;

  /**
   * helper to assign IPv4 addresses to UE devices as well as to the TUN device of the SGW/PGW
   */
  Ipv4AddressHelper m_uePgwAddressHelper;

  /**
   * IPv4 addresses subnet for UE devices as well as to the TUN device of the SGW/PGW
   */
  Ipv4Address m_ueIpSubnet;

  /**
   * helper to assign addresses to S1-U NetDevices
   */
  Ipv4AddressHelper m_epcIpv4AddressHelper;

  /**
   * IPv4 addresses subnet for S1-U NetDevices
   */
  Ipv4Address m_epcIpSubnet;

  /**
   * helper to assign addresses to S1-AP NetDevices
   */
  Ipv4AddressHelper m_mmeIpv4AddressHelper;

  /**
   * IPv4 addresses subnet for S1-AP NetDevices
   */
  Ipv4Address m_mmeIpSubnet;

  /**
   * helper to assign addresses to X2 NetDevices
   */
  Ipv4AddressHelper m_x2Ipv4AddressHelper;

  /**
   * IPv4 addresses subnet for X2 NetDevices
   */
  Ipv4Address m_x2IpSubnet;

  /**
   * helper to assign addresses to DCX NetDevices
   */
  Ipv4AddressHelper m_dcxIpv4AddressHelper;

  /**
   * IPv4 addresses subnet for DCX NetDevices
   */
  Ipv4Address m_dcxIpSubnet;

  /**
   * TUN device containing IPv4 address and  implementing tunneling of user data over GTP-U/UDP/IP
   */
  Ptr<VirtualNetDevice> m_tunDevice;

  /**
   * SGW-PGW network element
   */
  Ptr<Node> m_sgwPgw;

  /**
   * SGW-PGW application
   */
  Ptr<EpcSgwPgwApplication> m_sgwPgwApp;

  /**
   * MME network element
   */
  Ptr<Node> m_mmeNode;

  /**
   * MME application
   */
  Ptr<EpcMmeApplication> m_mmeApp;

  /**
   * UDP port where the GTP-U Socket is bound, fixed by the standard as 2152
   */
  uint16_t m_gtpuUdpPort;

  /**
   * UDP port where the UDP Socket is bound, fixed by the standard as
   * 36412 (it should be sctp, but it is not supported in ns-3)
   */
  uint16_t m_s1apUdpPort;

  /**
   * Map storing for each IMSI the corresponding eNB NetDevice
   *
   */
  std::map<uint64_t, Ptr<NetDevice> > m_imsiEnbDeviceMap;

  /**
   * Container for Ipv4Interfaces of the SGW/PGW
   */
  Ipv4InterfaceContainer m_sgwIpIfaces;

  /**
   * Container for Ipv4Interfaces of the MME
   */
  Ipv4InterfaceContainer m_mmeIpIfaces;

  /**
   * The name of the device used for the S1-U interface of the SGW
   */
  std::string m_sgwDeviceName;

  /**
   * The name of the device used for the S1-MME interface of the MME
   */
  std::string m_mmeDeviceName;

  /**
   * The name of the device used for the S1-U interface of the eNB
   */
  std::string m_enbS1uDeviceName;

  /**
   * The name of the device used for the S1-MME interface of the eNB
   */
  std::string m_enbS1MmeDeviceName;

  /**
   * The name of the device used for the X2 interface of the eNB
   */
  std::string m_enbX2DeviceName;

  /**
   * The name of the device used for the DCX interface of the UE
   */
  std::string m_ueDcxDeviceName;

  /**
   * MAC address used for the SGW
   */
  std::string m_sgwMacAddress;

  /**
   * MAC address used for the MME
   */
  std::string m_mmeMacAddress;

  /**
   * First 5 bytes of the eNB S1-U interface MAC address base
   */
  std::string m_enbMacAddressBaseS1u;

  /**
   * First 5 bytes of the eNB S1-MME interface MAC address base
   */
  std::string m_enbMacAddressBaseS1Mme;

  /**
   * First 4 bytes of the eNB X2 interface MAC address base
   */
  std::string m_enbMacAddressBaseX2;

  /**
   * 5th byte of the eNB X2 interface MAC address base, to ensure different addresses between links
   */
  uint16_t m_enbMacAddress5thByteX2;

  /**
   * First 4 bytes of the UE DCX interface MAC address base
   */
  std::string m_ueMacAddressBaseDcx;

  /**
   * 5th byte of the UE DCX interface MAC address base, to ensure different addresses between links
   */
  uint16_t m_ueMacAddress5thByteDcx;

};


} // namespace ns3

#endif // DALI_EMU_ONE_INSTANCE_EPC_HELPER_H
