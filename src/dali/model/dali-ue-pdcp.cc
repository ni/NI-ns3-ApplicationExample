/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 * Extension of DALI Dual Connectivity Configuration and Split Bearer between LTE eNBs
 */

#include <ns3/log.h>
#include <ns3/simulator.h>

#include "ns3/dali-ue-pdcp.h"
#include <ns3/lte-pdcp-header.h>
#include <ns3/lte-pdcp-sap.h>
#include <ns3/lte-pdcp-tag.h>
#include "ns3/dali-ue-dcx-sap.h"

#include <ns3/ni-logging.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DaliUePdcp");

class DaliUePdcpSpecificLteRlcSapUser : public LteRlcSapUser
{
public:
  DaliUePdcpSpecificLteRlcSapUser (DaliUePdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from LteRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);

private:
  DaliUePdcpSpecificLteRlcSapUser ();
  DaliUePdcp* m_pdcp;
};

DaliUePdcpSpecificLteRlcSapUser::DaliUePdcpSpecificLteRlcSapUser (DaliUePdcp* pdcp)
  : m_pdcp (pdcp)
{
}

DaliUePdcpSpecificLteRlcSapUser::DaliUePdcpSpecificLteRlcSapUser ()
{
}

void
DaliUePdcpSpecificLteRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdu (p);
}

///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (DaliUePdcp);

DaliUePdcp::DaliUePdcp ()
  : m_pdcpSapUser (0),
    m_rlcSapProvider (0),
    m_rnti (0),
    m_lcid (0),
	m_sEnbRnti (0),
    m_ueDcxPdcpProvider (0),
    m_txSequenceNumber (0),
    m_rxSequenceNumber (0),
	m_useDualConnectivity (false),
	m_sendOverSecondary (false),
	m_useInSequenceDelivery (false),
	m_sequenceLost (false)
{
  NS_LOG_FUNCTION (this);
  m_pdcpSapProvider = new LtePdcpSpecificLtePdcpSapProvider<DaliUePdcp> (this);
  m_rlcSapUser = new DaliUePdcpSpecificLteRlcSapUser (this);
  m_ueDcxPdcpUser = new UeDcxPdcpSpecificUser<DaliUePdcp> (this);
}

DaliUePdcp::~DaliUePdcp ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DaliUePdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DaliUePdcp")
    .SetParent<Object> ()
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the RLC.",
                     MakeTraceSourceAccessor (&DaliUePdcp::m_txPdu),
                     "ns3::DaliUePdcp::PduTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&DaliUePdcp::m_rxPdu),
                     "ns3::DaliUePdcp::PduRxTracedCallback")
    ;
  return tid;
}

void
DaliUePdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_pdcpSapProvider);
  delete (m_rlcSapUser);
  delete (m_ueDcxPdcpUser);
  m_pdcpPduMap.clear ();
}

void
DaliUePdcp::SetUeDcxPdcpProvider (UeDcxPdcpProvider * s)
{
  NS_LOG_FUNCTION(this);
  m_ueDcxPdcpProvider = s;
}


UeDcxPdcpUser*
DaliUePdcp::GetUeDcxPdcpUser ()
{
  NS_LOG_FUNCTION(this);
  return m_ueDcxPdcpUser;
}

void
DaliUePdcp::SetSenbRnti (uint16_t rnti)
{
  m_sEnbRnti = rnti;
}

void
DaliUePdcp::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
DaliUePdcp::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
DaliUePdcp::SetLtePdcpSapUser (LtePdcpSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_pdcpSapUser = s;
}

LtePdcpSapProvider*
DaliUePdcp::GetLtePdcpSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_pdcpSapProvider;
}

void
DaliUePdcp::SetLteRlcSapProvider (LteRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  NS_LOG_INFO("Change LteRlcSapProvider");
  m_rlcSapProvider = s;
}

LteRlcSapUser*
DaliUePdcp::GetLteRlcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapUser;
}

DaliUePdcp::Status
DaliUePdcp::GetStatus ()
{
  Status s;
  s.txSn = m_txSequenceNumber;
  s.rxSn = m_rxSequenceNumber;
  return s;
}

void
DaliUePdcp::SetStatus (Status s)
{
  m_txSequenceNumber = s.txSn;
  m_rxSequenceNumber = s.rxSn;
}

void
DaliUePdcp::SetUeDataParams(DaliUeDcxSap::UeDataParams params)
{
  m_ueDataParams = params;
}


////////////////////////////////////////

void
DaliUePdcp::DoTransmitPdcpSdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  LtePdcpHeader pdcpHeader;
  pdcpHeader.SetSequenceNumber (m_txSequenceNumber);

  m_txSequenceNumber++;
  if (m_txSequenceNumber > m_maxPdcpSn)
    {
      m_txSequenceNumber = 0;
    }

  pdcpHeader.SetDcBit (LtePdcpHeader::DATA_PDU);

  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
  p->AddHeader (pdcpHeader);

  LteRlcSapProvider::TransmitPdcpPduParameters params;
  params.rnti = m_rnti;
  params.lcid = m_lcid;

  // Sender timestamp. We will use this to measure the delay on top of RLC
  PdcpTag pdcpTag (Simulator::Now ());
  p->AddByteTag (pdcpTag);
  m_txPdu (m_rnti, m_lcid, p->GetSize ());

  if(m_ueDcxPdcpProvider == 0 || (!m_useDualConnectivity))
  {
    NS_LOG_INFO(this << " DaliUePdcp: Tx packet to Uplink local stack");
    params.pdcpPdu = p;

    NS_LOG_LOGIC("Params.rnti " << params.rnti);
    NS_LOG_LOGIC("Params.lcid " << params.lcid);
    NS_LOG_LOGIC("Params.pdcpPdu " << params.pdcpPdu);

    m_rlcSapProvider->TransmitPdcpPdu (params);
  }
  else if (m_useDualConnectivity)
  {
    if (!m_sendOverSecondary)
    {
      //Send package over local LTE stack
      params.pdcpPdu = p;

      NS_LOG_LOGIC("Params.rnti " << params.rnti);
      NS_LOG_LOGIC("Params.lcid " << params.lcid);
      NS_LOG_LOGIC("Params.pdcpPdu " << params.pdcpPdu);

      m_rlcSapProvider->TransmitPdcpPdu (params);
    }
    else
    {
      //Send package over DaliUeDcx interface to RLC layer on remote UE
      NS_LOG_INFO(this << " DaliUePdcp: Tx packet to Uplink LTE stack on Secondary UE " << m_ueDataParams.targetImsi);
      m_ueDataParams.ueData = p;
      m_ueDcxPdcpProvider->SendDcPdcpPdu (m_ueDataParams);
    }
    m_sendOverSecondary = !m_sendOverSecondary; // DALI: Alternate between Local and Remote stack
  }
  else
  {
    NS_FATAL_ERROR("Invalid combination");
  }
}

void
DaliUePdcp::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  NS_LOG_INFO(this << " DaliUePdcp received Downlink Pdu");
  // Receiver timestamp
  PdcpTag pdcpTag;
  Time delay;
  if (p->FindFirstMatchingByteTag (pdcpTag))
    {
      delay = Simulator::Now() - pdcpTag.GetSenderTimestamp ();
    }
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  p->RemoveAllByteTags();
  NS_LOG_LOGIC("ALL BYTE TAGS REMOVED. NetAmin and FlowMonitor won't work");

  LtePdcpHeader pdcpHeader;
  p->RemoveHeader (pdcpHeader);
  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);

  if (!m_useInSequenceDelivery)
  {
    m_rxSequenceNumber = pdcpHeader.GetSequenceNumber () + 1;
    if (m_rxSequenceNumber > m_maxPdcpSn)
    {
      m_rxSequenceNumber = 0;
    }
    DoReceivePduSend(p);
  }
  else
  {
	uint16_t rxSequenceNumber = pdcpHeader.GetSequenceNumber ();
	if (rxSequenceNumber == m_rxSequenceNumber)
	{
	  DoReceivePduSend(p);
  	  RxSequenceNumberNext();
  	  DoUnloadReceiveBuffer();
	}
	else
	{
      while((rxSequenceNumber - m_rxSequenceNumber) > m_maxPdcpRW ||
    		((m_rxSequenceNumber - rxSequenceNumber) > 0 && (m_rxSequenceNumber - rxSequenceNumber) < m_maxPdcpRW))
	  {
    	if (m_pdcpPduMap.empty()) {
    	  m_rxSequenceNumber = rxSequenceNumber;
    	  break;
    	}
	    std::map<uint16_t, Ptr<Packet> >::const_iterator it = m_pdcpPduMap.begin();
	    m_rxSequenceNumber = it->first;
	    DoUnloadReceiveBuffer();
	  }
      if (rxSequenceNumber == m_rxSequenceNumber)
      {
    	DoReceivePduSend(p);
    	RxSequenceNumberNext();
      }
      else
      {
        m_pdcpPduMap.insert(std::pair<uint16_t, Ptr<Packet> > (rxSequenceNumber, p));
        m_sequenceLost = true;
      }
	}
  }
}

void
DaliUePdcp::DoReceivePduSend (Ptr<Packet> p)
{
  if(p->GetSize() <= 20 + 8 + 12)
    {
      // #TODO clarify the need for this length check with DALI developers
      NI_LOG_WARN("DaliUePdcp length check failed!");
    }
  LtePdcpSapUser::ReceivePdcpSduParameters params;
  params.pdcpSdu = p;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  m_pdcpSapUser->ReceivePdcpSdu (params);
}

void
DaliUePdcp::RxSequenceNumberNext()
{
  m_rxSequenceNumber++;
  if (m_rxSequenceNumber > m_maxPdcpSn)
	m_rxSequenceNumber = 0;
}

void
DaliUePdcp::DoUnloadReceiveBuffer()
{
  if(m_sequenceLost)
  {
	std::map<uint16_t, Ptr<Packet> >::const_iterator it = m_pdcpPduMap.find (m_rxSequenceNumber);
	while (it != m_pdcpPduMap.end ())
	{
	  DoReceivePduSend(it->second);
	  m_pdcpPduMap.erase(it);
	  RxSequenceNumberNext();
	  it = m_pdcpPduMap.find (m_rxSequenceNumber);
	}
	if (m_pdcpPduMap.empty())
	  m_sequenceLost=false;
  }
}

void
DaliUePdcp::DoReceiveDcPdcpPdu (DaliUeDcxSap::UeDataParams params)
{
	NS_LOG_FUNCTION(this << m_sEnbRnti << (uint32_t) m_lcid);

  DoReceivePdu(params.ueData);
}

void
DaliUePdcp::UseDualConnectivity (bool useDualConnectivity)
{
	m_useDualConnectivity = useDualConnectivity;
}

bool
DaliUePdcp::GetUseDualConnectivity() const
{
  return m_useDualConnectivity && (m_ueDcxPdcpProvider != 0);
}

void
DaliUePdcp::UseInSequenceDelivery (bool useInSequenceDelivery)
{
	m_useInSequenceDelivery = useInSequenceDelivery;
}

bool
DaliUePdcp::GetUseInSequenceDelivery() const
{
  return m_useInSequenceDelivery;
}


} // namespace ns3
