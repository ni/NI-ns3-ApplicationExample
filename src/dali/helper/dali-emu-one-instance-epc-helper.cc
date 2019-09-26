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
#include <ns3/dali-emu-one-instance-epc-helper.h>
#include <ns3/log.h>
#include <ns3/inet-socket-address.h>
#include <ns3/mac48-address.h>
#include <ns3/eps-bearer.h>
#include <ns3/ipv4-address.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/packet-socket-helper.h>
#include <ns3/packet-socket-address.h>
#include <ns3/epc-enb-application.h>
#include <ns3/epc-sgw-pgw-application.h>
#include <ns3/emu-fd-net-device-helper.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/epc-x2.h>
#include <ns3/epc-s1ap.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/epc-mme-application.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/string.h>
#include <ns3/abort.h>
#include <ns3/ipv4-address-generator.h>
#include <ns3/icmpv6-l4-protocol.h>
#include <ns3/dali-ue-dcx.h>

#include <iomanip>
#include <iostream>

// TODO Consider implementing X2 interface use same FdNetDevice created for S1-AP interface as proposed by Michele Polese

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DaliEmuOneInstanceEpcHelper");

NS_OBJECT_ENSURE_REGISTERED (DaliEmuOneInstanceEpcHelper);


DaliEmuOneInstanceEpcHelper::DaliEmuOneInstanceEpcHelper ()
  : m_gtpuUdpPort (2152),  // fixed by the standard
    m_s1apUdpPort (36412)
{
  NS_LOG_FUNCTION (this);
  // To access the attribute value within the constructor
  ObjectBase::ConstructSelf (AttributeConstructionList ());
}

DaliEmuOneInstanceEpcHelper::~DaliEmuOneInstanceEpcHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DaliEmuOneInstanceEpcHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DaliEmuOneInstanceEpcHelper")
    .SetParent<EpcHelper> ()
    .SetGroupName("Lte")
    .AddConstructor<DaliEmuOneInstanceEpcHelper> ()
    .AddAttribute ("sgwDeviceName",
                   "The name of the device used for the S1-U interface of the SGW",
                   StringValue ("veth0"),
                   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_sgwDeviceName),
                   MakeStringChecker ())
	.AddAttribute ("mmeDeviceName",
			       "The name of the device used for the S1-MME interface of the MME",
				   StringValue ("veth1"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_mmeDeviceName),
				   MakeStringChecker ())
    .AddAttribute ("enbS1uDeviceName",
                   "The name of the device used for the S1-U interface of the eNB",
                   StringValue ("veth2"),
                   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_enbS1uDeviceName),
                   MakeStringChecker ())
	.AddAttribute ("enbS1MmeDeviceName",
			       "The name of the device used for the S1-MME interface of the eNB",
				   StringValue ("veth3"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_enbS1MmeDeviceName),
				   MakeStringChecker ())
	.AddAttribute ("enbX2DeviceName",
				   "The name of the device used for the X2 interfaces of the eNB",
				   StringValue ("veth4"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_enbX2DeviceName),
				   MakeStringChecker ())
	.AddAttribute ("ueDcxDeviceName",
				   "The name of the device used for the DCX interfaces of the UE",
				   StringValue ("veth5"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_ueDcxDeviceName),
				   MakeStringChecker ())
	.AddAttribute ("IpMask",
				   "IPv4 addresses mask for devices",
				   Ipv4MaskValue ("255.255.255.0"),
				   MakeIpv4MaskAccessor (&DaliEmuOneInstanceEpcHelper::m_ipMask),
				   MakeIpv4MaskChecker ())
	.AddAttribute ("ueIpSubnet",
				   "IPv4 addresses subnet for UE devices as well as to the TUN device of the SGW/PGW",
				   Ipv4AddressValue ("7.0.0.0"),
				   MakeIpv4AddressAccessor (&DaliEmuOneInstanceEpcHelper::m_ueIpSubnet),
				   MakeIpv4AddressChecker ())
	.AddAttribute ("epcIpSubnet",
				   "IPv4 addresses subnet for S1-U NetDevice",
				   Ipv4AddressValue ("11.0.0.0"),
				   MakeIpv4AddressAccessor (&DaliEmuOneInstanceEpcHelper::m_epcIpSubnet),
				   MakeIpv4AddressChecker ())
	.AddAttribute ("mmeIpSubnet",
				   "IPv4 addresses subnet for S1-AP NetDevices",
				   Ipv4AddressValue ("12.0.0.0"),
				   MakeIpv4AddressAccessor (&DaliEmuOneInstanceEpcHelper::m_mmeIpSubnet),
				   MakeIpv4AddressChecker ())
	.AddAttribute ("x2IpSubnet",
				   "IPv4 addresses subnet for X2 NetDevices",
				   Ipv4AddressValue ("13.0.0.0"),
				   MakeIpv4AddressAccessor (&DaliEmuOneInstanceEpcHelper::m_x2IpSubnet),
				   MakeIpv4AddressChecker ())
	.AddAttribute ("dcxIpSubnet",
				   "IPv4 addresses subnet for DCX NetDevices",
				   Ipv4AddressValue ("14.0.0.0"),
				   MakeIpv4AddressAccessor (&DaliEmuOneInstanceEpcHelper::m_dcxIpSubnet),
				   MakeIpv4AddressChecker ())
    .AddAttribute ("SgwMacAddress",
                   "MAC address used for the SGW ",
                   StringValue ("00:00:00:59:00:aa"),
                   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_sgwMacAddress),
                   MakeStringChecker ())
	.AddAttribute ("MmeMacAddress",
				   "MAC address used for the MME ",
				   StringValue ("00:00:00:59:00:bb"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_mmeMacAddress),
				   MakeStringChecker ())
    .AddAttribute ("EnbMacAddressBaseS1u",
                   "First 5 bytes of the eNB S1-U interface MAC address base",
                   StringValue ("00:00:00:eb:00"),
                   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_enbMacAddressBaseS1u),
                   MakeStringChecker ())
	.AddAttribute ("EnbMacAddressBaseS1Mme",
				   "First 5 bytes of the eNB S1-MME interface MAC address base",
				   StringValue ("00:00:00:eb:01"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_enbMacAddressBaseS1Mme),
				   MakeStringChecker ())
	.AddAttribute ("EnbMacAddressBaseX2",
				   "First 4 bytes of the eNB X2 interface MAC address base",
				   StringValue ("00:00:00:eb"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_enbMacAddressBaseX2),
				   MakeStringChecker ())
	.AddAttribute ("EnbMacAddress5thByteX2",
				   "5th byte of the eNB X2 interface MAC address base",
				   UintegerValue (31),
				   MakeUintegerAccessor (&DaliEmuOneInstanceEpcHelper::m_enbMacAddress5thByteX2),
				   MakeUintegerChecker<uint16_t> ())
	.AddAttribute ("UeMacAddressBaseDcx",
				   "First 4 bytes of the UE DCX interface MAC address base",
				   StringValue ("00:00:00:ue"),
				   MakeStringAccessor (&DaliEmuOneInstanceEpcHelper::m_ueMacAddressBaseDcx),
				   MakeStringChecker ())
	.AddAttribute ("UeMacAddress5thByteDcx",
				   "5th byte of the UE DCX interface MAC address base",
				   UintegerValue (0),
				   MakeUintegerAccessor (&DaliEmuOneInstanceEpcHelper::m_ueMacAddress5thByteDcx),
				   MakeUintegerChecker<uint16_t> ())
    ;
  return tid;
}

TypeId
DaliEmuOneInstanceEpcHelper::GetInstanceTypeId () const
{
  return GetTypeId ();
}

void
DaliEmuOneInstanceEpcHelper::DoInitialize ()
{
  NS_LOG_LOGIC (this);

  // we use a /8 net for all UEs
  m_uePgwAddressHelper.SetBase (m_ueIpSubnet, "255.0.0.0");


  // create SgwPgwNode
  m_sgwPgw = CreateObject<Node> ();
  InternetStackHelper internet;
  internet.SetIpv4StackInstall (true);
  internet.Install (m_sgwPgw);

  // create MmeNode
  m_mmeNode = CreateObject<Node> ();
  internet.Install (m_mmeNode);

  // create S1-U socket for SgwPgwNode
  Ptr<Socket> sgwPgwS1uSocket = Socket::CreateSocket (m_sgwPgw, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  int retval = sgwPgwS1uSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_gtpuUdpPort));
  NS_ASSERT (retval == 0);

  // create S1-AP socket for MmeNode
  Ptr<Socket> mmeS1apSocket = Socket::CreateSocket (m_mmeNode, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = mmeS1apSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_s1apUdpPort)); // it listens on any IP, port m_s1apUdpPort
  NS_ASSERT (retval == 0);

  // create TUN device containing IPv4 address and implementing tunneling of user data over GTP-U/UDP/IP
  m_tunDevice = CreateObject<VirtualNetDevice> ();

  // allow jumbo packets
  m_tunDevice->SetAttribute ("Mtu", UintegerValue (30000));

  // yes we need this
  m_tunDevice->SetAddress (Mac48Address::Allocate ());

  m_sgwPgw->AddDevice (m_tunDevice);
  NetDeviceContainer tunDeviceContainer;
  tunDeviceContainer.Add (m_tunDevice);

  // the TUN device is on the same subnet as the UEs, so when a packet
  // addressed to an UE IPv4 address arrives at the intenet to the WAN interface of
  // the PGW it will be forwarded to the TUN device.
  Ipv4InterfaceContainer tunDeviceIpv4IfContainer = AssignUeIpv4Address (tunDeviceContainer);

  // create EpcSgwPgwApplication
  m_sgwPgwApp = CreateObject<EpcSgwPgwApplication> (m_tunDevice, sgwPgwS1uSocket);
  m_sgwPgw->AddApplication (m_sgwPgwApp);

  // connect SgwPgwApplication and virtual net device for tunneling
  m_tunDevice->SetSendCallback (MakeCallback (&EpcSgwPgwApplication::RecvFromTunDevice, m_sgwPgwApp));

  // create S1apMme object and aggregate it with the m_mmeNode
  Ptr<EpcS1apMme> s1apMme = CreateObject<EpcS1apMme> (mmeS1apSocket, 1); // for now, only one mme!
  m_mmeNode->AggregateObject(s1apMme);

  // create EpcMmeApplication and connect with SGW via S11 interface
  m_mmeApp = CreateObject<EpcMmeApplication> ();
  m_mmeNode->AddApplication (m_mmeApp);
  m_mmeApp->SetS11SapSgw (m_sgwPgwApp->GetS11SapSgw ());
  m_sgwPgwApp->SetS11SapMme (m_mmeApp->GetS11SapMme ());
  // connect m_mmeApp to the s1apMme
  m_mmeApp->SetS1apSapMmeProvider(s1apMme->GetEpcS1apSapMmeProvider());
  s1apMme->SetEpcS1apSapMmeUser(m_mmeApp->GetS1apSapMme());

  // Create EmuFdNetDevice for SGW
  EmuFdNetDeviceHelper emu_sgw;
  NS_LOG_LOGIC ("SGW device: " << m_sgwDeviceName);
  emu_sgw.SetDeviceName (m_sgwDeviceName);
  NetDeviceContainer sgwDevices = emu_sgw.Install (m_sgwPgw);
  Ptr<NetDevice> sgwDevice = sgwDevices.Get (0);
  NS_LOG_LOGIC ("MAC address of SGW: " << m_sgwMacAddress);
  sgwDevice->SetAttribute ("Address", Mac48AddressValue (m_sgwMacAddress.c_str ()));

  // Create EmuFdNetDevice for MME
  EmuFdNetDeviceHelper emu_mme;
  NS_LOG_LOGIC ("MME device: " << m_mmeDeviceName);
  emu_mme.SetDeviceName (m_mmeDeviceName);
  NetDeviceContainer mmeDevices = emu_mme.Install (m_mmeNode);
  Ptr<NetDevice> mmeDevice = mmeDevices.Get (0);
  NS_LOG_LOGIC ("MAC address of MME: " << m_mmeMacAddress);
  mmeDevice->SetAttribute ("Address", Mac48AddressValue (m_mmeMacAddress.c_str ()));

  // we use a /24 subnet so the SGW and the eNBs can talk directly to each other
  m_epcIpv4AddressHelper.SetBase (m_epcIpSubnet, m_ipMask, "0.0.0.1");
  m_sgwIpIfaces = m_epcIpv4AddressHelper.Assign (sgwDevices);
  m_epcIpv4AddressHelper.SetBase (m_epcIpSubnet, m_ipMask, "0.0.0.101");

  // we use a /24 subnet so the MME and the eNBs can talk directly to each other
  m_mmeIpv4AddressHelper.SetBase (m_mmeIpSubnet, m_ipMask, "0.0.0.1"); //configure netmask /24
  m_mmeIpIfaces = m_mmeIpv4AddressHelper.Assign (mmeDevices);
  m_mmeIpv4AddressHelper.SetBase (m_mmeIpSubnet, m_ipMask, "0.0.0.101");

  // we use a /30 subnet which can hold exactly two addresses for X2 interfaces
  m_x2Ipv4AddressHelper.SetBase (m_x2IpSubnet, "255.255.255.252");

  // we use a /30 subnet which can hold exactly two addresses for DCX interfaces
  m_dcxIpv4AddressHelper.SetBase (m_dcxIpSubnet, "255.255.255.252");


  EpcHelper::DoInitialize ();
}

void
DaliEmuOneInstanceEpcHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_tunDevice->SetSendCallback (MakeNullCallback<bool, Ptr<Packet>, const Address&, const Address&, uint16_t> ());
  m_tunDevice = 0;
  m_sgwPgwApp = 0;
  m_sgwPgw->Dispose ();
  m_mmeApp =0;
  m_mmeNode->Dispose();
}


void
DaliEmuOneInstanceEpcHelper::AddEnb (Ptr<Node> enb, Ptr<NetDevice> lteEnbNetDevice, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << enb << lteEnbNetDevice << cellId);

  Initialize ();

  NS_ASSERT (enb == lteEnbNetDevice->GetNode ());

  // add an Internet stack to the previously created eNB
  InternetStackHelper internet;
  internet.Install (enb);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after node creation: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());


  // Create an EmuFdNetDevice for the eNB to connect with the SGW and other eNBs
  EmuFdNetDeviceHelper emu_sgw;
  NS_LOG_LOGIC ("eNB device: " << m_enbS1uDeviceName);
  emu_sgw.SetDeviceName (m_enbS1uDeviceName);
  NetDeviceContainer enbDevices = emu_sgw.Install (enb);

  NS_ABORT_IF ((cellId == 0) || (cellId > 255));
  std::ostringstream enbMacAddress;
  enbMacAddress << m_enbMacAddressBaseS1u << ":" << std::hex << std::setfill ('0') << std::setw (2) << cellId;
  NS_LOG_LOGIC ("MAC address of enB with cellId " << cellId << " : " << enbMacAddress.str ());
  Ptr<NetDevice> enbDev = enbDevices.Get (0);
  enbDev->SetAttribute ("Address", Mac48AddressValue (enbMacAddress.str ().c_str ()));

  //emu.EnablePcap ("enbDevice", enbDev);

  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after installing emu dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());
  Ipv4InterfaceContainer enbIpIfaces = m_epcIpv4AddressHelper.Assign (enbDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after assigning Ipv4 addr to S1 dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());

  Ipv4Address enbAddress = enbIpIfaces.GetAddress (0);
  Ipv4Address sgwAddress = m_sgwIpIfaces.GetAddress (0);

  // create S1-U socket for the ENB
  Ptr<Socket> enbS1uSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  int retval = enbS1uSocket->Bind (InetSocketAddress (enbAddress, m_gtpuUdpPort));
  NS_ASSERT (retval == 0);

  // create LTE socket for the ENB
  Ptr<Socket> enbLteSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::PacketSocketFactory"));
  PacketSocketAddress enbLteSocketBindAddress;
  enbLteSocketBindAddress.SetSingleDevice (lteEnbNetDevice->GetIfIndex ());
  enbLteSocketBindAddress.SetProtocol (Ipv4L3Protocol::PROT_NUMBER);
  retval = enbLteSocket->Bind (enbLteSocketBindAddress);
  NS_ASSERT (retval == 0);
  PacketSocketAddress enbLteSocketConnectAddress;
  enbLteSocketConnectAddress.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  enbLteSocketConnectAddress.SetSingleDevice (lteEnbNetDevice->GetIfIndex ());
  enbLteSocketConnectAddress.SetProtocol (Ipv4L3Protocol::PROT_NUMBER);
  retval = enbLteSocket->Connect (enbLteSocketConnectAddress);
  NS_ASSERT (retval == 0);


  // Create an EmuFdNetDevice for the eNB to connect with the MME
  EmuFdNetDeviceHelper emu_mme;
  NS_LOG_LOGIC ("eNB device: " << m_enbS1MmeDeviceName);
  emu_mme.SetDeviceName (m_enbS1MmeDeviceName);
  NetDeviceContainer enbapDevices = emu_mme.Install (enb);

  NS_ABORT_IF ((cellId == 0) || (cellId > 255));
  std::ostringstream enbapMacAddressS1AP;
  enbapMacAddressS1AP << m_enbMacAddressBaseS1Mme << ":" << std::hex << std::setfill ('0') << std::setw (2) << cellId;
  NS_LOG_LOGIC ("MAC address of enB with cellId " << cellId << " : " << enbapMacAddressS1AP.str ());
  Ptr<NetDevice> enbapDev = enbapDevices.Get (0);
  enbapDev->SetAttribute ("Address", Mac48AddressValue (enbapMacAddressS1AP.str ().c_str ()));

  //emu.EnablePcap ("enbapDevice", enbapDev);

  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after installing emu dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());
  Ipv4InterfaceContainer enbapIpIfaces = m_mmeIpv4AddressHelper.Assign (enbapDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after assigning Ipv4 addr to S1 dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());

  Ipv4Address mme_enbAddress = enbapIpIfaces.GetAddress (0);
  Ipv4Address mmeAddress = m_mmeIpIfaces.GetAddress (0);

  // create S1-AP socket for the ENB
  Ptr<Socket> enbS1apSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = enbS1apSocket->Bind (InetSocketAddress (mme_enbAddress, m_s1apUdpPort));
  NS_ASSERT (retval == 0);

  NS_LOG_INFO ("create EpcEnbApplication");
  Ptr<EpcEnbApplication> enbApp = CreateObject<EpcEnbApplication> (enbLteSocket, enbS1uSocket, enbAddress, sgwAddress, cellId);
  enb->AddApplication (enbApp);
  NS_ASSERT (enb->GetNApplications () == 1);
  NS_ASSERT_MSG (enb->GetApplication (0)->GetObject<EpcEnbApplication> () != 0, "cannot retrieve EpcEnbApplication");
  NS_LOG_LOGIC ("enb: " << enb << ", enb->GetApplication (0): " << enb->GetApplication (0));


  NS_LOG_INFO ("Create EpcX2 entity");
  Ptr<EpcX2> x2 = CreateObject<EpcX2> ();
  enb->AggregateObject (x2);

  NS_LOG_INFO ("connect S1-AP interface");

  uint16_t mmeId = 1;
  Ptr<EpcS1apEnb> s1apEnb = CreateObject<EpcS1apEnb> (enbS1apSocket, mme_enbAddress, mmeAddress, cellId, mmeId); // only one mme!
  enb->AggregateObject(s1apEnb);
  enbApp->SetS1apSapMme (s1apEnb->GetEpcS1apSapEnbProvider ());
  s1apEnb->SetEpcS1apSapEnbUser (enbApp->GetS1apSapEnb());
  m_mmeApp->AddEnb (cellId, mme_enbAddress); // TODO consider if this can be removed
  // add the interface to the S1AP endpoint on the MME
  Ptr<EpcS1apMme> s1apMme = m_mmeNode->GetObject<EpcS1apMme> ();
  s1apMme->AddS1apInterface (cellId, mme_enbAddress);

  m_sgwPgwApp->AddEnb (cellId, enbAddress, sgwAddress);
}

void
DaliEmuOneInstanceEpcHelper::AddX2Interface (Ptr<Node> enb1, Ptr<Node> enb2)
{
  NS_LOG_FUNCTION (this << enb1 << enb2);

  Ptr<LteEnbNetDevice> enb1LteDev = enb1->GetDevice (0)->GetObject<LteEnbNetDevice> ();
  uint16_t enb1CellId = 0;
  enb1CellId = enb1LteDev->GetCellId ();
  NS_LOG_INFO ("LteEnbNetDevice #1 = " << enb1LteDev << " - CellId = " << enb1CellId);

  Ptr<LteEnbNetDevice> enb2LteDev = enb2->GetDevice (0)->GetObject<LteEnbNetDevice> ();
  uint16_t enb2CellId = 0;
  enb2CellId = enb2LteDev->GetCellId ();
  NS_LOG_INFO ("LteEnbNetDevice #2 = " << enb2LteDev << " - CellId = " << enb2CellId);
  enb2LteDev->GetRrc ()->AddX2Neighbour (enb1CellId);

  ++m_enbMacAddress5thByteX2; //Ensure difference between X2 links MAC Addresses
  NetDeviceContainer enbX2Devices;

  // Create EmuFdNetDevice for the eNB1 to connect with eNB2
  EmuFdNetDeviceHelper emu_enb1;
  NS_LOG_LOGIC ("eNB1 device: " << m_enbX2DeviceName);
  emu_enb1.SetDeviceName (m_enbX2DeviceName);
  enbX2Devices.Add(emu_enb1.Install (enb1));

  std::ostringstream enb1MacAddressX2;
  enb1MacAddressX2 << m_enbMacAddressBaseX2 << ":" << std::hex << std::setfill ('0') << std::setw (2) << m_enbMacAddress5thByteX2 << ":" << std::hex << std::setfill ('0') << std::setw (2) << enb1CellId;
  NS_LOG_LOGIC ("MAC address of eNB1: " << enb1MacAddressX2.str ());
  Ptr<NetDevice> enb1X2Dev = enbX2Devices.Get (0);
  enb1X2Dev->SetAttribute ("Address", Mac48AddressValue (enb1MacAddressX2.str ().c_str ()));

  //emu_enb1.EnablePcap ("enb1X2Device", enb1X2Dev);

  // Create EmuFdNetDevice for the eNB2 to connect with eNB1
  EmuFdNetDeviceHelper emu_enb2;
  NS_LOG_LOGIC ("eNB2 device: " << m_enbX2DeviceName);
  emu_enb2.SetDeviceName (m_enbX2DeviceName);
  enbX2Devices.Add(emu_enb2.Install (enb2));

  std::ostringstream enb2MacAddressX2;
  enb2MacAddressX2 << m_enbMacAddressBaseX2 << ":" << std::hex << std::setfill ('0') << std::setw (2) << m_enbMacAddress5thByteX2 << ":" << std::hex << std::setfill ('0') << std::setw (2) << enb2CellId;
  NS_LOG_LOGIC ("MAC address of eNB2: " << enb2MacAddressX2.str ());
  Ptr<NetDevice> enb2X2Dev = enbX2Devices.Get (1);
  enb2X2Dev->SetAttribute ("Address", Mac48AddressValue (enb2MacAddressX2.str ().c_str ()));

  //emu_enb2.EnablePcap ("enb2X2Device", enb2X2Dev);


  m_x2Ipv4AddressHelper.NewNetwork ();
  Ipv4InterfaceContainer enbIpIfaces = m_x2Ipv4AddressHelper.Assign (enbX2Devices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #1 after assigning Ipv4 addr to X2 dev: " << enb1->GetObject<Ipv4> ()->GetNInterfaces ());
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #2 after assigning Ipv4 addr to X2 dev: " << enb2->GetObject<Ipv4> ()->GetNInterfaces ());

  Ipv4Address enb1X2Address = enbIpIfaces.GetAddress (0);
  Ipv4Address enb2X2Address = enbIpIfaces.GetAddress (1);

  // Add X2 interface to both eNBs' X2 entities
  Ptr<EpcX2> enb1X2 = enb1->GetObject<EpcX2> ();
  Ptr<EpcX2> enb2X2 = enb2->GetObject<EpcX2> ();

  enb1X2->AddX2Interface (enb1CellId, enb1X2Address, enb2CellId, enb2X2Address);
  enb2X2->AddX2Interface (enb2CellId, enb2X2Address, enb1CellId, enb1X2Address);

  enb1LteDev->GetRrc ()->AddX2Neighbour (enb2CellId);

  // if((enb1CellId == 1 && enb2CellId == 2) || (enb1CellId == 2 && enb2CellId == 1))
  // {
  //   //AsciiTraceHelper ascii;
  //   p2ph.EnablePcapAll("x2.pcap");
  // }
}


void
DaliEmuOneInstanceEpcHelper::AddUe (Ptr<NetDevice> ueDevice, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi << ueDevice );

  m_mmeApp->AddUe (imsi);
  m_sgwPgwApp->AddUe (imsi);
}

void
DaliEmuOneInstanceEpcHelper::AddDaliUe (Ptr<Node> ue, Ptr<NetDevice> ueDevice, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi << ueDevice );

  m_mmeApp->AddUe (imsi);
  m_sgwPgwApp->AddUe (imsi);

  NS_LOG_INFO ("Create UeDcx entity");
  Ptr<DaliUeDcx> dcx = CreateObject<DaliUeDcx> ();
  ue->AggregateObject (dcx);
}

void
DaliEmuOneInstanceEpcHelper::AddDcxInterface (Ptr<Node> ue1, Ptr<Node> ue2)
{
  NS_LOG_FUNCTION (this << ue1 << ue2);

  Ptr<LteUeNetDevice> ue1LteDev = ue1->GetDevice (0)->GetObject<LteUeNetDevice> ();
  uint64_t ue1Imsi = 0;
  ue1Imsi = ue1LteDev->GetImsi ();
  NS_LOG_INFO ("LteUeNetDevice #1 = " << ue1LteDev << " - Imsi = " << ue1Imsi);

  Ptr<LteUeNetDevice> ue2LteDev = ue2->GetDevice (0)->GetObject<LteUeNetDevice> ();
  uint64_t ue2Imsi = 0;
  ue2Imsi = ue2LteDev->GetImsi ();
  NS_LOG_INFO ("LteUeNetDevice #2 = " << ue2LteDev << " - Imsi = " << ue2Imsi);

  ++m_ueMacAddress5thByteDcx; //Ensure difference between DCX links MAC Addresses
  NetDeviceContainer ueDcxDevices;

  // Create EmuFdNetDevice for the UE1 to connect with UE2
  EmuFdNetDeviceHelper emu_ue1;
  NS_LOG_LOGIC ("UE1 device: " << m_ueDcxDeviceName);
  emu_ue1.SetDeviceName (m_ueDcxDeviceName);
  ueDcxDevices.Add(emu_ue1.Install (ue1));

  std::ostringstream ue1MacAddressDcx;
  ue1MacAddressDcx << m_ueMacAddressBaseDcx << ":" << std::hex << std::setfill ('0') << std::setw (2) << m_ueMacAddress5thByteDcx << ":" << std::hex << std::setfill ('0') << std::setw (2) << ue1Imsi;
  NS_LOG_LOGIC ("MAC address of UE1: " << ue1MacAddressDcx.str ());
  Ptr<NetDevice> ue1DcxDev = ueDcxDevices.Get (0);
  ue1DcxDev->SetAttribute ("Address", Mac48AddressValue (ue1MacAddressDcx.str ().c_str ()));

  //emu_ue1.EnablePcap ("ue1DcxDevice", ue1DcxDev);

  // Create EmuFdNetDevice for the UE2 to connect with UE1
  EmuFdNetDeviceHelper emu_ue2;
  NS_LOG_LOGIC ("UE2 device: " << m_ueDcxDeviceName);
  emu_ue2.SetDeviceName (m_ueDcxDeviceName);
  ueDcxDevices.Add(emu_ue2.Install (ue2));

  std::ostringstream ue2MacAddressDcx;
  ue2MacAddressDcx << m_ueMacAddressBaseDcx << ":" << std::hex << std::setfill ('0') << std::setw (2) << m_ueMacAddress5thByteDcx << ":" << std::hex << std::setfill ('0') << std::setw (2) <<ue2Imsi;
  NS_LOG_LOGIC ("MAC address of UE2: " << ue2MacAddressDcx.str ());
  Ptr<NetDevice> ue2DcxDev = ueDcxDevices.Get (1);
  ue2DcxDev->SetAttribute ("Address", Mac48AddressValue (ue2MacAddressDcx.str ().c_str ()));

  //emu_ue2.EnablePcap ("ue2DcxDevice", ue2DcxDev);


  m_dcxIpv4AddressHelper.NewNetwork ();
  Ipv4InterfaceContainer ueIpIfaces = m_dcxIpv4AddressHelper.Assign (ueDcxDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the UE #1 after assigning Ipv4 addr to DCX dev: " << ue1->GetObject<Ipv4> ()->GetNInterfaces ());
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the UE #2 after assigning Ipv4 addr to DCX dev: " << ue2->GetObject<Ipv4> ()->GetNInterfaces ());

  Ipv4Address ue1DcxAddress = ueIpIfaces.GetAddress (0);
  Ipv4Address ue2DcxAddress = ueIpIfaces.GetAddress (1);

  // Add DCX interface to both UEs' DCX entities
  Ptr<DaliUeDcx> ue1Dcx = ue1->GetObject<DaliUeDcx> ();
  Ptr<DaliUeDcx> ue2Dcx = ue2->GetObject<DaliUeDcx> ();

  ue1Dcx->AddDcxInterface (ue1Imsi, ue1DcxAddress, ue2Imsi, ue2DcxAddress);
  ue2Dcx->AddDcxInterface (ue2Imsi, ue2DcxAddress, ue1Imsi, ue1DcxAddress);

}

uint8_t
DaliEmuOneInstanceEpcHelper::ActivateEpsBearer (Ptr<NetDevice> ueDevice, uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice << imsi);

  // we now retrieve the IPv4 address of the UE and notify it to the SGW;
  // we couldn't do it before since address assignment is triggered by
  // the user simulation program, rather than done by the EPC
  Ptr<Node> ueNode = ueDevice->GetNode ();
  Ptr<Ipv4> ueIpv4 = ueNode->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ueIpv4 != 0, "UEs need to have IPv4 installed before EPS bearers can be activated");

  if (ueIpv4)
    {
      int32_t interface =  ueIpv4->GetInterfaceForDevice (ueDevice);
      if (interface >= 0 && ueIpv4->GetNAddresses (interface) == 1)
        {
          Ipv4Address ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();
          NS_LOG_LOGIC (" UE IPv4 address: " << ueAddr);
          m_sgwPgwApp->SetUeAddress (imsi, ueAddr);
        }
    }

  uint8_t bearerId = m_mmeApp->AddBearer (imsi, tft, bearer);
  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice> ();
  if (ueLteDevice)
    {
      Simulator::ScheduleNow (&EpcUeNas::ActivateEpsBearer, ueLteDevice->GetNas (), bearer, tft);
    }
  return bearerId;
}

uint8_t
DaliEmuOneInstanceEpcHelper::ActivateEpsBearer (Ptr<NetDevice> ueDevice, Ptr<EpcUeNas> ueNas, uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice << imsi);

  // we now retrieve the IPv4 address of the UE and notify it to the SGW;
  // we couldn't do it before since address assignment is triggered by
  // the user simulation program, rather than done by the EPC
  Ptr<Node> ueNode = ueDevice->GetNode ();
  Ptr<Ipv4> ueIpv4 = ueNode->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ueIpv4 != 0, "UEs need to have IPv4 installed before EPS bearers can be activated");

  if (ueIpv4)
    {
      int32_t interface =  ueIpv4->GetInterfaceForDevice (ueDevice);
      if (interface >= 0 && ueIpv4->GetNAddresses (interface) == 1)
        {
          Ipv4Address ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();
          NS_LOG_LOGIC (" UE IPv4 address: " << ueAddr);
          m_sgwPgwApp->SetUeAddress (imsi, ueAddr);
        }
    }

  uint8_t bearerId = m_mmeApp->AddBearer (imsi, tft, bearer);
  Simulator::ScheduleNow (&EpcUeNas::ActivateEpsBearer, ueNas, bearer, tft);
  return bearerId;
}


Ptr<Node>
DaliEmuOneInstanceEpcHelper::GetPgwNode ()
{
  return m_sgwPgw;
}


Ipv4InterfaceContainer
DaliEmuOneInstanceEpcHelper::AssignUeIpv4Address (NetDeviceContainer ueDevices)
{
  return m_uePgwAddressHelper.Assign (ueDevices);
}

Ipv4Address
DaliEmuOneInstanceEpcHelper::GetUeDefaultGatewayAddress ()
{
  // return the address of the tun device
  return m_sgwPgw->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
}


} // namespace ns3
