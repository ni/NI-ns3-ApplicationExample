/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab.
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
 * Extension to DC devices by Michele Polese <michele.polese@gmail.com>
 *
 * Modified by: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 * Extension of DALI Dual Connectivity Configuration and Split Bearer between LTE eNBs
 */

#include <ns3/log.h>
#include <ns3/simulator.h>

#include "ns3/dali-enb-pdcp.h"
#include <ns3/lte-pdcp-header.h>
#include <ns3/lte-pdcp-sap.h>
#include <ns3/lte-pdcp-tag.h>
#include <ns3/epc-x2-sap.h>

#include "ns3/lwa-tag.h"
#include "ns3/lwip-tag.h"
#include "ns3/pdcp-lcid.h"

#include "ns3/ni-module.h"

#include <ns3/ni-logging.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DaliEnbPdcp");

class DaliEnbPdcpSpecificLteRlcSapUser : public LteRlcSapUser
{
public:
  DaliEnbPdcpSpecificLteRlcSapUser (DaliEnbPdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from LteRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);

private:
  DaliEnbPdcpSpecificLteRlcSapUser ();
  DaliEnbPdcp* m_pdcp;
};

DaliEnbPdcpSpecificLteRlcSapUser::DaliEnbPdcpSpecificLteRlcSapUser (DaliEnbPdcp* pdcp)
  : m_pdcp (pdcp)
{
}

DaliEnbPdcpSpecificLteRlcSapUser::DaliEnbPdcpSpecificLteRlcSapUser ()
{
}

void
DaliEnbPdcpSpecificLteRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdu (p);
}

///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (DaliEnbPdcp);

DaliEnbPdcp::DaliEnbPdcp ()
  : m_pdcpSapUser (0),
    m_rlcSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    m_sEnbRnti (0),
    m_epcX2PdcpProvider (0),
    m_txSequenceNumber (0),
    m_rxSequenceNumber (0),
    m_useDualConnectivity (false),
    m_sendOverSecondary (false),
    m_useInSequenceDelivery (false),
    m_sequenceLost (false),
    m_packetCounter(0),
    pdcp_decisionlwa(0),
    pdcp_decisionlwip(0),
    m_dcLwaLwipSwitch(0),
    pdcp_decisionDc(0)
{
  NS_LOG_FUNCTION (this);
  m_pdcpSapProvider = new LtePdcpSpecificLtePdcpSapProvider<DaliEnbPdcp> (this);
  m_rlcSapUser = new DaliEnbPdcpSpecificLteRlcSapUser (this);
  m_epcX2PdcpUser = new EpcX2PdcpSpecificUser<DaliEnbPdcp> (this);
}

DaliEnbPdcp::~DaliEnbPdcp ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DaliEnbPdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DaliEnbPdcp")
    .SetParent<Object> ()
	.AddConstructor<DaliEnbPdcp> ()
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the RLC.",
                     MakeTraceSourceAccessor (&DaliEnbPdcp::m_txPdu),
                     "ns3::DaliEnbPdcp::PduTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&DaliEnbPdcp::m_rxPdu),
                     "ns3::DaliEnbPdcp::PduRxTracedCallback")
    .AddTraceSource ("TxPDUtrace",//modifications for trace
                     "PDU to be transmit.",//modifications for trace
                     MakeTraceSourceAccessor (&DaliEnbPdcp::m_pdcptxtrace),//modifications for trace
                     "ns3::Packet::TracedCallback")//modifications for trace
    .AddAttribute ("PDCPDecLwa",
                   "PDCP LWA or LWIP decision variable",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DaliEnbPdcp::pdcp_decisionlwa),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("PDCPDecLwip",
                   "PDCP LWIP decision variable",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DaliEnbPdcp::pdcp_decisionlwip),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("DcLwaLwipSwitch",
                   "Switch to allow LWA/LWIP when DC is enabled",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DaliEnbPdcp::m_dcLwaLwipSwitch),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("PDCPDecDc",
                   "PDCP DC decision variable",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DaliEnbPdcp::pdcp_decisionDc),
                   MakeUintegerChecker<uint32_t>())

    ;
  return tid;
}

void
DaliEnbPdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_pdcpSapProvider);
  delete (m_rlcSapUser);
  delete (m_epcX2PdcpUser);
  m_pdcpPduMap.clear ();
}

void
DaliEnbPdcp::SetEpcX2PdcpProvider (EpcX2PdcpProvider * s)
{
  NS_LOG_FUNCTION(this);
  m_epcX2PdcpProvider = s;
}


EpcX2PdcpUser*
DaliEnbPdcp::GetEpcX2PdcpUser ()
{
  NS_LOG_FUNCTION(this);
  return m_epcX2PdcpUser;
}

void
DaliEnbPdcp::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
DaliEnbPdcp::SetSenbRnti (uint16_t rnti)
{
  m_sEnbRnti = rnti;
}

void
DaliEnbPdcp::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
DaliEnbPdcp::SetLtePdcpSapUser (LtePdcpSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_pdcpSapUser = s;
}

LtePdcpSapProvider*
DaliEnbPdcp::GetLtePdcpSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_pdcpSapProvider;
}

void
DaliEnbPdcp::SetLteRlcSapProvider (LteRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  NS_LOG_INFO("Change LteRlcSapProvider");
  m_rlcSapProvider = s;
}

LteRlcSapUser*
DaliEnbPdcp::GetLteRlcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapUser;
}

DaliEnbPdcp::Status
DaliEnbPdcp::GetStatus ()
{
  Status s;
  s.txSn = m_txSequenceNumber;
  s.rxSn = m_rxSequenceNumber;
  return s;
}

void
DaliEnbPdcp::SetStatus (Status s)
{
  m_txSequenceNumber = s.txSn;
  m_rxSequenceNumber = s.rxSn;
}

void
DaliEnbPdcp::SetUeDataParams(EpcX2Sap::UeDataParams params)
{
  m_ueDataParams = params;
}


////////////////////////////////////////
void
DaliEnbPdcp::TransmitOverLwaLwip(LteRlcSapProvider::TransmitPdcpPduParameters params, Ptr<Packet> p)
{
  params.pdcpPdu = p;

  LwaTag LWATag;
  LwipTag LWIPTag;
  PdcpLcid lcidtag;

  // switch between the lwa/lwip modes
  if ((pdcp_decisionlwa>0)&&(pdcp_decisionlwip==0)&&(m_lcid>=3)) // use lwa
    {
      NI_LOG_DEBUG("DC not launched and lwa enabled");
      if(pdcp_decisionlwa==1)
        {
          // split packets between lwa und default lte link
          if((m_packetCounter%2)==0)
            {
              m_rlcSapProvider->TransmitPdcpPdu (params);
            }
          if((m_packetCounter%2)==1)
            {
              LWATag.Set(pdcp_decisionlwa);
              lcidtag.Set((uint32_t)m_lcid);
              p->AddPacketTag (LWATag);
              p->AddByteTag (lcidtag);
              m_pdcptxtrace(params.pdcpPdu);
            }
          m_packetCounter++;
        }
      else
        {
          // all packets routed over lwa (all drb)
          LWATag.Set(pdcp_decisionlwa);
          lcidtag.Set((uint32_t)m_lcid);
          p->AddPacketTag (LWATag);
          p->AddByteTag (lcidtag);
          m_pdcptxtrace(params.pdcpPdu);
        }
    }
  else if ((pdcp_decisionlwa==0)&&(pdcp_decisionlwip>0)&&(m_lcid>=3)) // use lwip
    {
      NI_LOG_DEBUG("DC not launched and lwip enabled");
      // all packets routed over lwip
      LWIPTag.Set(pdcp_decisionlwip);
      lcidtag.Set((uint32_t)m_lcid);
      p->AddPacketTag (LWIPTag);
      p->AddByteTag (lcidtag);
      m_pdcptxtrace(params.pdcpPdu);
    }
}

void
DaliEnbPdcp::DoTransmitPdcpSdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  NI_LOG_DEBUG("eNB send PDCP data for lcid=" << (uint32_t) m_lcid << ", m_rnti=" <<  m_rnti << ", packet size=" << p->GetSize ());

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

  //NI API CHANGE: read out values from remote control database and overwrite local decision variables
  if (pdcp_decisionlwa != g_RemoteControlEngine.GetPdb()->getParameterLwaDecVariable())
    {
      pdcp_decisionlwa = g_RemoteControlEngine.GetPdb()->getParameterLwaDecVariable();
      NI_LOG_CONSOLE_INFO("NI.RC:LWA value changed! LWA value is : " << pdcp_decisionlwa);
    }
  if (pdcp_decisionlwip != g_RemoteControlEngine.GetPdb()->getParameterLwipDecVariable())
    {
      pdcp_decisionlwip = g_RemoteControlEngine.GetPdb()->getParameterLwipDecVariable();
      NI_LOG_CONSOLE_INFO("NI.RC:LWIP value changed! LWIP value is : " << pdcp_decisionlwip);
    }
  if (pdcp_decisionDc != g_RemoteControlEngine.GetPdb()->getParameterDcDecVariable())
    {
      pdcp_decisionDc = g_RemoteControlEngine.GetPdb()->getParameterDcDecVariable();
      NI_LOG_CONSOLE_INFO("NI.RC:DC value changed! DC value is : " << pdcp_decisionDc);
    }

  NS_LOG_INFO(this << " DaliEnbPdcp: Tx packet to downlink local stack");

  if(m_epcX2PdcpProvider == 0 || (!m_useDualConnectivity))
    {
	  if ((pdcp_decisionlwa>0)||(pdcp_decisionlwip>0)) // transmit packet over LWA/LWIP
	    TransmitOverLwaLwip(params, p);
      else // use default functionality
        {
          params.pdcpPdu = p;
          NS_LOG_LOGIC("Params.rnti " << params.rnti);
          NS_LOG_LOGIC("Params.m_lcid " << params.lcid);
          NS_LOG_LOGIC("Params.pdcpPdu " << params.pdcpPdu);
          NI_LOG_DEBUG("DC not launched and lwa/lwip disabled, use default functionality");
          m_rlcSapProvider->TransmitPdcpPdu (params);
        }
    }
  else if (m_useDualConnectivity)
    {
	  if (((pdcp_decisionlwa>0)||(pdcp_decisionlwip>0))&&m_dcLwaLwipSwitch==1) // transmit packet over LWA/LWIP
	    TransmitOverLwaLwip(params, p);
	  else
	    {
        if (pdcp_decisionDc == 1)
          m_sendOverSecondary = !m_sendOverSecondary; // DALI: Alternate between Local and Remote stack
        else if (pdcp_decisionDc == 2)
          m_sendOverSecondary = true; // use only remote stack
        else
          m_sendOverSecondary = false; // use only local stack

	      if (!m_sendOverSecondary)
	        {
	          NI_LOG_NONE("LTE.ENB.PDCP.DC: Tx packet to downlink local stack");
	          //Send package over local LTE stack
	          params.pdcpPdu = p;
	          NS_LOG_LOGIC("Params.rnti " << params.rnti);
	          NS_LOG_LOGIC("Params.m_lcid " << params.lcid);
	          NS_LOG_LOGIC("Params.pdcpPdu " << params.pdcpPdu);
	          m_rlcSapProvider->TransmitPdcpPdu (params);
	        }
	      else
	        {
	          //Send package over epcX2 interface to RLC layer on remote eNB
	          NS_LOG_INFO(this << " DaliEnbPdcp: Tx packet to downlink stack on Secondary eNB " << m_ueDataParams.targetCellId);
	          NI_LOG_CONSOLE_DEBUG("LTE.ENB.PDCP.DC: Tx packet to downlink stack on Secondary eNB");
	          m_ueDataParams.ueData = p;
	          m_epcX2PdcpProvider->SendDcPdcpPdu (m_ueDataParams);
	        }
	    }
    }
  else
    {
      NS_FATAL_ERROR("Invalid combination");
    }

  NI_LOG_NONE(m_useDualConnectivity << " " << pdcp_decisionlwa << " " << pdcp_decisionlwip << " " <<
                       m_dcLwaLwipSwitch << " " << pdcp_decisionDc << " " << m_sendOverSecondary);
}

void
DaliEnbPdcp::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  NS_LOG_INFO(this << " DaliEnbPdcp received uplink Pdu");
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
DaliEnbPdcp::DoReceivePduSend (Ptr<Packet> p)
{
  if(p->GetSize() <= 20 + 8 + 12)
    {
      // #TODO clarify the need for this length check with DALI developers
      NI_LOG_WARN("DaliEnbPdcp length check failed!");
    }
  LtePdcpSapUser::ReceivePdcpSduParameters params;
  params.pdcpSdu = p;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  m_pdcpSapUser->ReceivePdcpSdu (params);
}

void
DaliEnbPdcp::RxSequenceNumberNext()
{
  m_rxSequenceNumber++;
  if (m_rxSequenceNumber > m_maxPdcpSn)
	m_rxSequenceNumber = 0;
}

void
DaliEnbPdcp::DoUnloadReceiveBuffer()
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
DaliEnbPdcp::DoReceiveDcPdcpPdu (EpcX2Sap::UeDataParams params)
{

    NS_LOG_FUNCTION(this << m_sEnbRnti << (uint16_t) m_sEnbRnti);

  DoReceivePdu(params.ueData);
}

void
DaliEnbPdcp::UseDualConnectivity (bool useDualConnectivity)
{
	m_useDualConnectivity = useDualConnectivity;
}

bool
DaliEnbPdcp::GetUseDualConnectivity() const
{
  return m_useDualConnectivity && (m_epcX2PdcpProvider != 0);
}

void
DaliEnbPdcp::UseInSequenceDelivery (bool useInSequenceDelivery)
{
	m_useInSequenceDelivery = useInSequenceDelivery;
}

bool
DaliEnbPdcp::GetUseInSequenceDelivery() const
{
  return m_useInSequenceDelivery;
}


} // namespace ns3
