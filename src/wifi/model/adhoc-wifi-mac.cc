/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 */

#include "adhoc-wifi-mac.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "mac-low.h"
#include "dcf-manager.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "msdu-aggregator.h"
#include "amsdu-subframe-header.h"
#include "mgt-headers.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AdhocWifiMac");

NS_OBJECT_ENSURE_REGISTERED (AdhocWifiMac);

TypeId
AdhocWifiMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AdhocWifiMac")
    .SetParent<RegularWifiMac> ()
    .SetGroupName ("Wifi")
    .AddConstructor<AdhocWifiMac> ()
  ;
  return tid;
}

AdhocWifiMac::AdhocWifiMac ()
{
  NS_LOG_FUNCTION (this);
  //Let the lower layers know that we are acting in an IBSS
  SetTypeOfStation (ADHOC_STA);

  // NI API CHANGE
  NI_LOG_DEBUG ("Create AdhocWifiMac");
  // create the NiWifiMacInterface object
  m_NiWifiAdMacInterface = CreateObject <NiWifiMacInterface> (NS3_ADHOC);
  // create callbacks from ni wifi sublayer tx interface
  m_NiWifiAdMacInterface->SetNiApWifiRxDataEndOkCallback (MakeCallback (&AdhocWifiMac::Receive, this));
}

AdhocWifiMac::~AdhocWifiMac ()
{
  NS_LOG_FUNCTION (this);
}

void
AdhocWifiMac::SetAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this << address);
  //In an IBSS, the BSSID is supposed to be generated per Section
  //11.1.3 of IEEE 802.11. We don't currently do this - instead we
  //make an IBSS STA a bit like an AP, with the BSSID for frames
  //transmitted by each STA set to that STA's address.
  //
  //This is why we're overriding this method.
  RegularWifiMac::SetAddress (address);
  RegularWifiMac::SetBssid (address);
}

// This function, which originated from the standard ns-3 AdhocWifiMac::Enqueue function, was modified to generate
// the TX Request messages in a certain format, that is expected by the 802.11 AFW.
// When the NIAPI is enabled, the ns-3 WifiMacHeader and ns-3 Packet are stored in the MSDU data of the TX Payload Request.
void
AdhocWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NI_LOG_DEBUG ("AdhocWifiMac::Enqueue: serialized packet size = " << packet->GetSerializedSize());

  NS_LOG_FUNCTION (this << packet << to);
  if (m_stationManager->IsBrandNew (to))
    {
      NI_LOG_DEBUG ("AdhocWifiMac::Enqueue: m_stationManager->IsBrandNew (to) = " << to);

      //In ad hoc mode, we assume that every destination supports all
      //the rates we support.
      if (m_htSupported || m_vhtSupported)
        {
          m_stationManager->AddAllSupportedMcs (to);
          m_stationManager->AddStationHtCapabilities (to, GetHtCapabilities());
        }
      if (m_vhtSupported)
        {
          m_stationManager->AddStationVhtCapabilities (to, GetVhtCapabilities());
        }
      m_stationManager->AddAllSupportedModes (to);
      m_stationManager->RecordDisassociated (to);
    }

  WifiMacHeader hdr;

  //If we are not a QoS STA then we definitely want to use AC_BE to
  //transmit the packet. A TID of zero will map to AC_BE (through \c
  //QosUtilsMapTidToAc()), so we use that as our default here.
  uint8_t tid = 0;

  //For now, a STA that supports QoS does not support non-QoS
  //associations, and vice versa. In future the STA model should fall
  //back to non-QoS if talking to a peer that is also non-QoS. At
  //that point there will need to be per-station QoS state maintained
  //by the association state machine, and consulted here.
  if (m_qosSupported)
    {
      NI_LOG_DEBUG("AdhocWifiMac::Enqueue: m_qosSupported");

      hdr.SetType (WIFI_MAC_QOSDATA);
      hdr.SetQosAckPolicy (WifiMacHeader::NORMAL_ACK);
      hdr.SetQosNoEosp ();
      hdr.SetQosNoAmsdu ();
      //Transmission of multiple frames in the same TXOP is not
      //supported for now
      hdr.SetQosTxopLimit (0);

      //Fill in the QoS control field in the MAC header
      tid = QosUtilsGetTidForPacket (packet);
      //Any value greater than 7 is invalid and likely indicates that
      //the packet had no QoS tag, so we revert to zero, which will
      //mean that AC_BE is used.
      if (tid > 7)
        {
          tid = 0;
        }
      hdr.SetQosTid (tid);
    }
  else
    {
      hdr.SetTypeData ();
    }

  if (m_htSupported || m_vhtSupported)
    {
      NI_LOG_DEBUG("AdhocWifiMac::Enqueue: m_htSupported = " << m_htSupported << ", m_vhtSupported = " << m_vhtSupported);

      hdr.SetNoOrder ();
    }
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (m_low->GetAddress ());
  hdr.SetAddr3 (GetBssid ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();

  if (m_NiWifiAdMacInterface->GetNiApiEnable() )
    {
      m_NiWifiAdMacInterface->NiStartTxCtrlDataFrame(packet, hdr);

      NI_LOG_DEBUG ("AdhocWifiMac::Enqueue: packet incl hdr sent to NI WiFi");
    }
  else
    {
      if (m_qosSupported)
        {
          //Sanity check that the TID is valid
          NS_ASSERT (tid < 8);
          m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
        }

      else
        {
          m_dca->Queue (packet, hdr);
        }
    }
}

void
AdhocWifiMac::SetLinkUpCallback (Callback<void> linkUp)
{
  NS_LOG_FUNCTION (this << &linkUp);
  RegularWifiMac::SetLinkUpCallback (linkUp);

  //The approach taken here is that, from the point of view of a STA
  //in IBSS mode, the link is always up, so we immediately invoke the
  //callback if one is set
  linkUp ();
}

void
AdhocWifiMac::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION (this << packet << hdr);
  NS_ASSERT (!hdr->IsCtl ());
  Mac48Address from = hdr->GetAddr2 ();
  Mac48Address to = hdr->GetAddr1 ();

  NI_LOG_DEBUG ("AdhocWifiMac::Receive: if control frame: packet = " << packet << ", header = " << *hdr << ", source address = " << from);

  if (m_stationManager->IsBrandNew (from))
    {
      NI_LOG_DEBUG ("AdhocWifiMac::Receive: checking StationManager");

      //In ad hoc mode, we assume that every destination supports all
      //the rates we support.
      if (m_htSupported || m_vhtSupported)
        {
          NI_LOG_DEBUG ("AdhocWifiMac::Receive: m_htSupported || m_vhtSupported");

          m_stationManager->AddAllSupportedMcs (from);
          m_stationManager->AddStationHtCapabilities (from, GetHtCapabilities());
        }
      if (m_vhtSupported)
        {
          NI_LOG_DEBUG ("AdhocWifiMac::Receive: m_vhtSupported");

          m_stationManager->AddStationVhtCapabilities (from, GetVhtCapabilities());
        }
      m_stationManager->AddAllSupportedModes (from);
      m_stationManager->RecordDisassociated (from);
    }
  if (hdr->IsData ())
    {
      if (hdr->IsQosData () && hdr->IsQosAmsdu ())
        {
          NI_LOG_DEBUG ("AdhocWifiMac::Receive: QoS Data - packet " << packet << " from " << from << " with header "<< hdr);

          NS_LOG_DEBUG ("Received A-MSDU from" << from);
          DeaggregateAmsduAndForward (packet, hdr);
        }
      else
        {
          NI_LOG_DEBUG ("AdhocWifiMac::Receive: Non QoS Data - packet " << packet << " forward up from " << from << " to " << to);

          ForwardUp (packet, from, to);
        }
      return;
    }

  //Invoke the receive handler of our parent class to deal with any
  //other frames. Specifically, this will handle Block Ack-related
  //Management Action frames.
  RegularWifiMac::Receive (packet, hdr);
}

} //namespace ns3
