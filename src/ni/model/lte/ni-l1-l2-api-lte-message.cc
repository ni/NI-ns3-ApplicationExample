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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include "ni-l1-l2-api-lte.h"
#include "ni-l1-l2-api-lte-message.h"


namespace ns3 {

//=============================================================================================================================
//Inserts a uint64_t value into an array of bytes at given offset.
int32_t InsertU64( uint8_t* p_buffer, uint32_t offset, uint64_t value )
//=============================================================================================================================
{

  // re-interpret value as array of uint8_t
  uint8_t* p;
  p = (uint8_t*)&value;

  // put into buffer
  for(int i = 0; i < 8; i++)
  {
    p_buffer[offset+i] = p[i];
  }

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
//Inserts a uint32_t value into an array of bytes at given offset.
int32_t InsertU32( uint8_t* p_buffer, uint32_t offset, uint32_t value )
//=============================================================================================================================
{

  // re-interpret value as array of uint8_t
  uint8_t* p;
  p = (uint8_t *)&value;

  // put into buffer
  for(int i = 0; i < 4; i++)
  {
    p_buffer[offset+i] = p[i];
  }

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================




//=============================================================================================================================
//Extracts a uint64_t value from an array of bytes at given offset.
int32_t ExtractU64( uint8_t* p_buffer, uint32_t offset, uint64_t* p_value )
//=============================================================================================================================
{

  (*p_value) = 0;
  for (int i=7; i>=0; --i)
  {
    (*p_value)  = ((*p_value) << 8) | p_buffer[offset+i];
  }

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
//Extracts a uint32_t value from an array of bytes at given offset.
int32_t ExtractU32( uint8_t* p_buffer, uint32_t offset, uint32_t* p_value )
//=============================================================================================================================
{

  (*p_value)  = 0;
  for (int i=3; i>=0; --i)
  {
    (*p_value)  = ((*p_value)  << 8) | p_buffer[offset+i];
  }

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//======================================================================================
// Extract msgType (=bytes 0-1 of NIAPI message) from raw buffer
// bufferOffset should point at the beginning of the NIAPI message
int32_t GetMsgType(
  uint32_t* p_msgType,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  (*p_msgType) = 0;
  (*p_msgType) = (p_buffer[(*p_bufferOffset)] << 8 ) | p_buffer[(*p_bufferOffset)+1];

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
// Extract body length (=bytes 5-7 of NIAPI message) from raw buffer
// bufferOffset should point at the beginning of the NIAPI message
int32_t GetBodyLength(
  uint32_t* p_bodyLength,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  (*p_bodyLength) = 0;
  (*p_bodyLength) = (p_buffer[(*p_bufferOffset)+5] << 16 ) | (p_buffer[(*p_bufferOffset)+6] << 8 ) | p_buffer[(*p_bufferOffset)+7];

  return 0;
}
//======================================================================================
//======================================================================================



//=============================================================================================================================
// Print contents of a buffer to cmd, start at given offset and print given number of bytes
int32_t PrintBuffer( uint8_t* p_buffer, uint32_t bufferOffset, uint32_t length )
//=============================================================================================================================
{

  for (uint32_t i = bufferOffset; i < (bufferOffset+length); i++ )
  {

    // print offset at the beginning of each line
    if ((i%16) == 0)
    {
      printf( "%4i:    ", i );
    }

    // print bytes
    printf( "%02x ", p_buffer[i] );

    // print spacer every 4 bytes
    if ( ( (i%16) == 3 ) | ( (i%16) == 7 ) | ( (i%16) == 11 ) )
    {
      printf( "   " );
    }

    // line break every 16 bytes
    if ((i%16) == 15)
    {
      printf( "\n" );
    }
  }
  printf( "\n" );
  printf( "\n" );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
// Print string representation of given message type.
int32_t PrintMsgType( uint32_t msgType )
//=============================================================================================================================
{

  printf( "msgType = 0x%04x = %i = ", msgType, msgType );

  switch (msgType)
  {
    case 0x4001:
    {
      printf( "PHY_TIMING_IND" );
      break;
    };

    case 0x4202:
    {
      printf( "PHY_CNF" );
      break;
    };

    case 0x4501:
    {
      printf( "PHY_DL_TX_CONFIG_REQ" );
      break;
    };

    case 0x4502:
    {
      printf( "PHY_DL_TX_PAYLOAD_REQ" );
      break;
    };

    case 0x4801:
    {
      printf( "PHY_DLSCH_RX_IND" );
      break;
    };

    default:
    {
      printf( "UNKNOWN" );
    };

  }

  printf( "\n" );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
// Print string representation and explanation of given confirmation status.
int32_t PrintCnfStatus( uint32_t cnfStatus )
//=============================================================================================================================
{

  printf( "cnfStatus = 0x%02x = %i = ", cnfStatus, cnfStatus );

  switch (cnfStatus)
  {

    case 0x0:
    {
      printf( "CNF_SUCCESS\n" );
      printf( "Request accepted and sent to the subsequent module.\n" );
      printf( "No error has occurred during handling of the request.\n" );
      break;
    };

    case 0x1:
    {
      printf( "CNF_UNKNOWN_MESSAGE\n" );
      printf( "The message type is unknown or undefined.\n" );
      break;
    };

    case 0x2:
    {
      printf( "CNF_MESSAGE_NOT_SUPPORTED\n" );
      printf( "The message type is defined, but not supported.\n" );
      break;
    };

    case 0x3:
    {
      printf( "CNF_UNKNOWN_PARAMETER_SET\n" );
      printf( "One or more of the received parameters or parameter\n" );
      printf( "sets are not part of the message.\n");
      break;
    };

    case 0x4:
    {
      printf( "CNF_MISSING_PARAMETER_SET\n" );
      printf( "One or more mandatory parameters or parameter sets\n" );
      printf( "of the received message are missing\n" );
      break;
    };

    case 0x5:
    {
      printf( "CNF_PARAMETER_SET_REPETITION\n" );
      printf( "One or more of the received parameters or parameter\n" );
      printf( "sets have been set more than once\n" );
      break;
    };

    case 0x6:
    {
      printf( "CNF_RANGE_VIOLATION\n" );
      printf( "One or more parameters are out of the supported range\n" );
      break;
    };

    case 0x7:
    {
      printf( "CNF_STATE_VIOLATION\n" );
      printf( "The received request is not supported in the current state\n" );
      printf( "of operation. This includes:\n" );
      printf( "- Any violation of the allowed message sequence\n" );
      printf( "- Specific configurations that are not allowed due to\n" );
      printf( "  previously configured settings / configurations\n" );
      printf( "- etc.\n" );
      break;
    };

    case 0x8:
    {
      printf( "CNF_TIMEOUT\n" );
      printf( "An expected message has not been received in time.\n");
      break;
    };

    case 0x9:
    {
      printf( "CNF_CONFIG_PAYLOAD_MISMATCH\n" );
      printf( "The config part of a request does not fit to the payload part\n" );
      printf( "(e.g. for mismatch of lengths).\n" );
      break;
    };

    case 0xA:
    {
      printf( "CNF_LENGTH_MISMATCH\n" );
      printf( "The content of any \"message body length\" or \"parameter set body\n" );
      printf( "length\" indicator field does not match to the actual message body\n" );
      printf( "length or parameter set body length.\n" );
      break;
    };

    case 0xB:
    {
      printf( "CNF_INPUT_BUFFER_FULL\n" );
      printf( "Request rejected since the input buffer for incoming requests is full.\n" );
      printf( "The subsequent module needs some time to execute the previously stored requests.\n" );
      break;
    };

    case 0xC:
    {
      printf( "CNF_INTERNAL_ERROR\n" );
      printf( "Internal error.\n");
      break;
    };

    case 0xD:
    {
      printf( "CNF_INSTANCE_ID_MISMATCH\n" );
      printf( "The local instance ID is not like the decoded instance ID from API module.\n" );
      break;
    };

    default:
    {
      printf( "UNKNOWN\n" );
      printf( "\n");
    };

  }

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================




//=============================================================================================================================
// Print header (common for all NIAPI messages)
int32_t PrintHeader( GenMsgHdr* p_genMsgHdr, LteSubMsgHdr* p_subMsgHdr )
//=============================================================================================================================
{

  printf( "genMsgHdr.msgType    = 0x%04x\n", p_genMsgHdr->msgType );
  printf( "genMsgHdr.refId      = 0x%04x\n", p_genMsgHdr->refId );
  printf( "genMsgHdr.instId     = 0x%02x\n", p_genMsgHdr->instId );
  printf( "genMsgHdr.bodyLength = 0x%06x\n", p_genMsgHdr->bodyLength );
  printf( "\n" );
  printf( "subMsgHdr.sfn        = 0x%04x\n", p_subMsgHdr->sfn );
  printf( "subMsgHdr.tti        = 0x%02x\n", p_subMsgHdr->tti );
  printf( "subMsgHdr.numSubMsg  = 0x%02x\n", p_subMsgHdr->numSubMsg );
  printf( "subMsgHdr.cnfMode    = 0x%02x\n", p_subMsgHdr->cnfMode );
  printf( "subMsgHdr.resField   = 0x%06x\n", p_subMsgHdr->resField );
  printf( "\n" );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
int32_t PrintPhyTimingInd( PhyTimingInd* p_phyTimingInd )
//=============================================================================================================================
{

  printf( "phyTimingInd.genMsgHdr.msgType    = 0x%04x\n", p_phyTimingInd->genMsgHdr.msgType );
  printf( "phyTimingInd.genMsgHdr.refId      = 0x%04x\n", p_phyTimingInd->genMsgHdr.refId );
  printf( "phyTimingInd.genMsgHdr.instId     = 0x%02x\n", p_phyTimingInd->genMsgHdr.instId );
  printf( "phyTimingInd.genMsgHdr.bodyLength = 0x%06x\n", p_phyTimingInd->genMsgHdr.bodyLength );
  printf( "\n" );
  printf( "phyTimingInd.subMsgHdr.sfn        = 0x%04x\n", p_phyTimingInd->subMsgHdr.sfn );
  printf( "phyTimingInd.subMsgHdr.tti        = 0x%02x\n", p_phyTimingInd->subMsgHdr.tti );
  printf( "phyTimingInd.subMsgHdr.numSubMsg  = 0x%02x\n", p_phyTimingInd->subMsgHdr.numSubMsg );
  printf( "phyTimingInd.subMsgHdr.cnfMode    = 0x%02x\n", p_phyTimingInd->subMsgHdr.cnfMode );
  printf( "phyTimingInd.subMsgHdr.resField   = 0x%06x\n", p_phyTimingInd->subMsgHdr.resField );
  printf( "\n" );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
int32_t PrintPhyCnf( PhyCnf* p_phyCnf )
//=============================================================================================================================
{

  printf( "phyCnf.genMsgHdr.msgType    = 0x%04x\n", p_phyCnf->genMsgHdr.msgType );
  printf( "phyCnf.genMsgHdr.refId      = 0x%04x\n", p_phyCnf->genMsgHdr.refId );
  printf( "phyCnf.genMsgHdr.instId     = 0x%02x\n", p_phyCnf->genMsgHdr.instId );
  printf( "phyCnf.genMsgHdr.bodyLength = 0x%06x\n", p_phyCnf->genMsgHdr.bodyLength );
  printf( "\n" );
  printf( "phyCnf.subMsgHdr.sfn        = 0x%04x\n", p_phyCnf->subMsgHdr.sfn );
  printf( "phyCnf.subMsgHdr.tti        = 0x%02x\n", p_phyCnf->subMsgHdr.tti );
  printf( "phyCnf.subMsgHdr.numSubMsg  = 0x%02x\n", p_phyCnf->subMsgHdr.numSubMsg );
  printf( "phyCnf.subMsgHdr.cnfMode    = 0x%02x\n", p_phyCnf->subMsgHdr.cnfMode );
  printf( "phyCnf.subMsgHdr.resField   = 0x%06x\n", p_phyCnf->subMsgHdr.resField );
  printf( "\n" );
  printf( "phyCnf.cnfHdr.subMsgType       = 0x%02x\n", p_phyCnf->cnfHdr.subMsgType);
  printf( "phyCnf.cnfHdr.parSetId         = 0x%02x\n", p_phyCnf->cnfHdr.parSetId);
  printf( "phyCnf.cnfHdr.parSetBodyLength = 0x%06x\n", p_phyCnf->cnfHdr.parSetBodyLength);
  printf( "\n" );
  printf( "phyCnf.cnfBody.cnfStatus  = 0x%02x\n", p_phyCnf->cnfBody.cnfStatus);
  printf( "phyCnf.cnfBody.srcMsgType = 0x%04x\n", p_phyCnf->cnfBody.srcMsgType);
  printf( "\n" );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
// Print complete PHY_DL_TX_CONFIG_REQ
int32_t PrintPhyDlTxConfigReq( PhyDlTxConfigReq* p_phyDlTxConfigReq )
//=============================================================================================================================
{

  printf( "\n" );
  printf( "phyDlTxConfigReq.genMsgHdr.msgType    = 0x%04x\n", p_phyDlTxConfigReq->genMsgHdr.msgType );
  printf( "phyDlTxConfigReq.genMsgHdr.refId      = 0x%04x\n", p_phyDlTxConfigReq->genMsgHdr.refId );
  printf( "phyDlTxConfigReq.genMsgHdr.instId     = 0x%02x\n", p_phyDlTxConfigReq->genMsgHdr.instId );
  printf( "phyDlTxConfigReq.genMsgHdr.bodyLength = 0x%06x\n", p_phyDlTxConfigReq->genMsgHdr.bodyLength );
  printf( "\n" );
  printf( "phyDlTxConfigReq.subMsgHdr.sfn       = 0x%04x\n", p_phyDlTxConfigReq->subMsgHdr.sfn       );
  printf( "phyDlTxConfigReq.subMsgHdr.tti       = 0x%02x\n", p_phyDlTxConfigReq->subMsgHdr.tti       );
  printf( "phyDlTxConfigReq.subMsgHdr.numSubMsg = 0x%02x\n", p_phyDlTxConfigReq->subMsgHdr.numSubMsg );
  printf( "phyDlTxConfigReq.subMsgHdr.cnfMode   = 0x%02x\n", p_phyDlTxConfigReq->subMsgHdr.cnfMode   );
  printf( "phyDlTxConfigReq.subMsgHdr.resField  = 0x%06x\n", p_phyDlTxConfigReq->subMsgHdr.resField  );
  printf( "\n" );
  printf( "phyDlTxConfigReq.dlschTxConfigHdr.subMsgType       = 0x%02x\n", p_phyDlTxConfigReq->dlschTxConfigHdr.subMsgType);
  printf( "phyDlTxConfigReq.dlschTxConfigHdr.parSetId         = 0x%02x\n", p_phyDlTxConfigReq->dlschTxConfigHdr.parSetId);
  printf( "phyDlTxConfigReq.dlschTxConfigHdr.parSetBodyLength = 0x%06x\n", p_phyDlTxConfigReq->dlschTxConfigHdr.parSetBodyLength);
  printf( "\n" );
  printf( "phyDlTxConfigReq.dlschTxConfigBody.macPduIndex   = 0x%02x\n", p_phyDlTxConfigReq->dlschTxConfigBody.macPduIndex);
  printf( "phyDlTxConfigReq.dlschTxConfigBody.rnti          = 0x%04x\n", p_phyDlTxConfigReq->dlschTxConfigBody.rnti);
  printf( "phyDlTxConfigReq.dlschTxConfigBody.prbAllocation = 0x%08x\n", p_phyDlTxConfigReq->dlschTxConfigBody.prbAllocation);
  printf( "phyDlTxConfigReq.dlschTxConfigBody.mcs           = 0x%02x\n", p_phyDlTxConfigReq->dlschTxConfigBody.mcs);
  printf( "\n" );
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantHdr.subMsgType       = 0x%02x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantHdr.subMsgType);
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantHdr.parSetId         = 0x%02x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantHdr.parSetId);
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantHdr.parSetBodyLength = 0x%06x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantHdr.parSetBodyLength);
  printf( "\n" );
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantBody.rnti          = 0x%04x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantBody.rnti);
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantBody.cceOffset     = 0x%02x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantBody.cceOffset);
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantBody.prbAllocation = 0x%08x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantBody.prbAllocation);
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantBody.mcs           = 0x%02x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantBody.mcs);
  printf( "phyDlTxConfigReq.dciTxConfigDlGrantBody.tpc           = 0x%02x\n", p_phyDlTxConfigReq->dciTxConfigDlGrantBody.tpc);
  printf( "\n" );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
// Print complete PHY_DL_TX_PAYLOAD_REQ incl. payload
int32_t PrintPhyDlTxPayloadReq( PhyDlTxPayloadReq* p_phyDlTxPayloadReq )
//=============================================================================================================================
{

  printf( "\n" );
  printf( "phyDlTxPayloadReq.genMsgHdr.msgType    = 0x%04x\n", p_phyDlTxPayloadReq->genMsgHdr.msgType );
  printf( "phyDlTxPayloadReq.genMsgHdr.refId      = 0x%04x\n", p_phyDlTxPayloadReq->genMsgHdr.refId );
  printf( "phyDlTxPayloadReq.genMsgHdr.instId     = 0x%02x\n", p_phyDlTxPayloadReq->genMsgHdr.instId );
  printf( "phyDlTxPayloadReq.genMsgHdr.bodyLength = 0x%06x\n", p_phyDlTxPayloadReq->genMsgHdr.bodyLength );
  printf( "\n" );
  printf( "phyDlTxPayloadReq.subMsgHdr.sfn       = 0x%04x\n", p_phyDlTxPayloadReq->subMsgHdr.sfn       );
  printf( "phyDlTxPayloadReq.subMsgHdr.tti       = 0x%02x\n", p_phyDlTxPayloadReq->subMsgHdr.tti       );
  printf( "phyDlTxPayloadReq.subMsgHdr.numSubMsg = 0x%02x\n", p_phyDlTxPayloadReq->subMsgHdr.numSubMsg );
  printf( "phyDlTxPayloadReq.subMsgHdr.cnfMode   = 0x%02x\n", p_phyDlTxPayloadReq->subMsgHdr.cnfMode   );
  printf( "phyDlTxPayloadReq.subMsgHdr.resField  = 0x%06x\n", p_phyDlTxPayloadReq->subMsgHdr.resField  );
  printf( "\n" );
  printf( "phyDlTxPayloadReq.dlschMacPduTxHdr.subMsgType       = 0x%02x\n", p_phyDlTxPayloadReq->dlschMacPduTxHdr.subMsgType);
  printf( "phyDlTxPayloadReq.dlschMacPduTxHdr.parSetId         = 0x%02x\n", p_phyDlTxPayloadReq->dlschMacPduTxHdr.parSetId);
  printf( "phyDlTxPayloadReq.dlschMacPduTxHdr.parSetBodyLength = 0x%06x\n", p_phyDlTxPayloadReq->dlschMacPduTxHdr.parSetBodyLength);
  printf( "\n" );
  printf( "phyDlTxPayloadReq.dlschMacPduTxBody.macPduIndex   = 0x%02x\n", p_phyDlTxPayloadReq->dlschMacPduTxBody.macPduIndex);
  printf( "phyDlTxPayloadReq.dlschMacPduTxBody.macPduSize    = 0x%06x\n", p_phyDlTxPayloadReq->dlschMacPduTxBody.macPduSize);
  printf( "phyDlTxPayloadReq.dlschMacPduTxBody.macPdu = \n");
  printf( "\n" );
  PrintBuffer( (*p_phyDlTxPayloadReq).dlschMacPduTxBody.macPdu, 0, p_phyDlTxPayloadReq->dlschMacPduTxBody.macPduSize );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
// Print complete PHY_DL_DLSCH_RX_IND incl. payload
int32_t PrintPhyDlschRxInd( PhyDlschRxInd* p_phyDlschRxInd )
//=============================================================================================================================
{

  printf( "\n" );
  printf( "phyDlschRxInd.genMsgHdr.msgType    = 0x%04x\n", p_phyDlschRxInd->genMsgHdr.msgType );
  printf( "phyDlschRxInd.genMsgHdr.refId      = 0x%04x\n", p_phyDlschRxInd->genMsgHdr.refId );
  printf( "phyDlschRxInd.genMsgHdr.instId     = 0x%02x\n", p_phyDlschRxInd->genMsgHdr.instId );
  printf( "phyDlschRxInd.genMsgHdr.bodyLength = 0x%06x\n", p_phyDlschRxInd->genMsgHdr.bodyLength );
  printf( "\n" );
  printf( "phyDlschRxInd.subMsgHdr.sfn       = 0x%04x\n", p_phyDlschRxInd->subMsgHdr.sfn       );
  printf( "phyDlschRxInd.subMsgHdr.tti       = 0x%02x\n", p_phyDlschRxInd->subMsgHdr.tti       );
  printf( "phyDlschRxInd.subMsgHdr.numSubMsg = 0x%02x\n", p_phyDlschRxInd->subMsgHdr.numSubMsg );
  printf( "phyDlschRxInd.subMsgHdr.cnfMode   = 0x%02x\n", p_phyDlschRxInd->subMsgHdr.cnfMode   );
  printf( "phyDlschRxInd.subMsgHdr.resField  = 0x%06x\n", p_phyDlschRxInd->subMsgHdr.resField  );
  printf( "\n" );
  printf( "phyDlschRxInd.dlschMacPduTxHdr.subMsgType       = 0x%02x\n", p_phyDlschRxInd->dlschMacPduRxHdr.subMsgType);
  printf( "phyDlschRxInd.dlschMacPduTxHdr.parSetId         = 0x%02x\n", p_phyDlschRxInd->dlschMacPduRxHdr.parSetId);
  printf( "phyDlschRxInd.dlschMacPduTxHdr.parSetBodyLength = 0x%06x\n", p_phyDlschRxInd->dlschMacPduRxHdr.parSetBodyLength);
  printf( "\n" );
  printf( "phyDlschRxInd.dlschMacPduRxBody.rnti       = 0x%04x\n", p_phyDlschRxInd->dlschMacPduRxBody.rnti);
  printf( "phyDlschRxInd.dlschMacPduRxBody.crcResult  = 0x%02x\n", p_phyDlschRxInd->dlschMacPduRxBody.crcResult);
  printf( "phyDlschRxInd.dlschMacPduRxBody.macPduSize = 0x%06x\n", p_phyDlschRxInd->dlschMacPduRxBody.macPduSize);
  printf( "phyDlschRxInd.dlschMacPduRxBody.macPdu = \n");
  printf( "\n" );
  PrintBuffer( (*p_phyDlschRxInd).dlschMacPduRxBody.macPdu, 0, p_phyDlschRxInd->dlschMacPduRxBody.macPduSize );

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================



//=============================================================================================================================
// Initialize PHY_DL_TX_CONFIG_REQ with default values
int32_t InitializePhyDlTxConfigReq( PhyDlTxConfigReq* p_phyDlTxConfigReq )
//=============================================================================================================================
{

  (*p_phyDlTxConfigReq).genMsgHdr.msgType    = PHY_DL_TX_CONFIG_REQ;
  (*p_phyDlTxConfigReq).genMsgHdr.refId      = 0;   // update in loop
  (*p_phyDlTxConfigReq).genMsgHdr.instId     = 0;   // unused
  (*p_phyDlTxConfigReq).genMsgHdr.bodyLength = 35;  // in bytes, excl. genMsgHdr

  (*p_phyDlTxConfigReq).subMsgHdr.sfn        = 0;   // update in loop
  (*p_phyDlTxConfigReq).subMsgHdr.tti        = 0;   // update in loop
  (*p_phyDlTxConfigReq).subMsgHdr.numSubMsg  = 2;
  (*p_phyDlTxConfigReq).subMsgHdr.cnfMode    = 0;  // 0 = no confirmation
  (*p_phyDlTxConfigReq).subMsgHdr.resField   = 0;

  (*p_phyDlTxConfigReq).dlschTxConfigHdr.subMsgType       = DLSCH_TX_CONFIG;
  (*p_phyDlTxConfigReq).dlschTxConfigHdr.parSetId         = 0;
  (*p_phyDlTxConfigReq).dlschTxConfigHdr.parSetBodyLength = 8; // in bytes

  (*p_phyDlTxConfigReq).dlschTxConfigBody.macPduIndex     = 0;
  (*p_phyDlTxConfigReq).dlschTxConfigBody.rnti            = 0;  // update in loop
  (*p_phyDlTxConfigReq).dlschTxConfigBody.prbAllocation   = 0;  // update in loop
  (*p_phyDlTxConfigReq).dlschTxConfigBody.mcs             = 0;  // update in loop

  (*p_phyDlTxConfigReq).dciTxConfigDlGrantHdr.subMsgType       = DCI_TX_CONFIG_DL_GRANT;
  (*p_phyDlTxConfigReq).dciTxConfigDlGrantHdr.parSetId         = 0;
  (*p_phyDlTxConfigReq).dciTxConfigDlGrantHdr.parSetBodyLength = 9;  // in bytes

  (*p_phyDlTxConfigReq).dciTxConfigDlGrantBody.rnti            = 0;  // update in loop
  (*p_phyDlTxConfigReq).dciTxConfigDlGrantBody.cceOffset       = 0;
  (*p_phyDlTxConfigReq).dciTxConfigDlGrantBody.prbAllocation   = 0;  // update in loop
  (*p_phyDlTxConfigReq).dciTxConfigDlGrantBody.mcs             = 0;  // update in loop
  (*p_phyDlTxConfigReq).dciTxConfigDlGrantBody.tpc             = 0;

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================


//=============================================================================================================================
// Initialize PHY_DL_TX_PAYLOAD_REQ with default values.
int32_t InitializePhyDlTxPayloadReq( PhyDlTxPayloadReq* p_phyDlTxPayloadReq )
//=============================================================================================================================
{

  (*p_phyDlTxPayloadReq).genMsgHdr.msgType    = PHY_DL_TX_PAYLOAD_REQ;
  (*p_phyDlTxPayloadReq).genMsgHdr.refId      = 0;  // update in loop
  (*p_phyDlTxPayloadReq).genMsgHdr.instId     = 0;  // unused
  (*p_phyDlTxPayloadReq).genMsgHdr.bodyLength = 17; // =17+tbs, update in loop, value in bytes, excl. genMsgHdr

  (*p_phyDlTxPayloadReq).subMsgHdr.sfn        = 0;  // update in loop
  (*p_phyDlTxPayloadReq).subMsgHdr.tti        = 0;  // update in loop
  (*p_phyDlTxPayloadReq).subMsgHdr.numSubMsg  = 1;
  (*p_phyDlTxPayloadReq).subMsgHdr.cnfMode    = 0;  // 0 = no confirmation
  (*p_phyDlTxPayloadReq).subMsgHdr.resField   = 0;

  (*p_phyDlTxPayloadReq).dlschMacPduTxHdr.subMsgType       = DLSCH_MAC_PDU;
  (*p_phyDlTxPayloadReq).dlschMacPduTxHdr.parSetId         = 0;
  (*p_phyDlTxPayloadReq).dlschMacPduTxHdr.parSetBodyLength = 4;  // =4+tbs, update in loop

  (*p_phyDlTxPayloadReq).dlschMacPduTxBody.macPduIndex = 0;  // currently sending only one PDU per TTI
  (*p_phyDlTxPayloadReq).dlschMacPduTxBody.macPduSize  = 0;  // =tbs, update in loop

  memset((*p_phyDlTxPayloadReq).dlschMacPduTxBody.macPdu, 0, MAX_MAC_PDU_SIZE);

  return 0;
}

//=============================================================================================================================
// Initialize PHY_UL_TX_PAYLOAD_REQ with default values.
int32_t InitializePhyUlTxPayloadReq( PhyUlTxPayloadReq* p_phyUlTxPayloadReq )
//=============================================================================================================================
{

  (*p_phyUlTxPayloadReq).genMsgHdr.msgType    = PHY_UL_TX_PAYLOAD_REQ;
  (*p_phyUlTxPayloadReq).genMsgHdr.refId      = 0;  // update in loop
  (*p_phyUlTxPayloadReq).genMsgHdr.instId     = 0;  // unused
  (*p_phyUlTxPayloadReq).genMsgHdr.bodyLength = 17; // =17+tbs, update in loop, value in bytes, excl. genMsgHdr

  (*p_phyUlTxPayloadReq).subMsgHdr.sfn        = 0;  // update in loop
  (*p_phyUlTxPayloadReq).subMsgHdr.tti        = 0;  // update in loop
  (*p_phyUlTxPayloadReq).subMsgHdr.numSubMsg  = 1;
  (*p_phyUlTxPayloadReq).subMsgHdr.cnfMode    = 0;  // 0 = no confirmation
  (*p_phyUlTxPayloadReq).subMsgHdr.resField   = 0;

  (*p_phyUlTxPayloadReq).ulschMacPduTxHdr.subMsgType       = DLSCH_MAC_PDU;
  (*p_phyUlTxPayloadReq).ulschMacPduTxHdr.parSetId         = 0;
  (*p_phyUlTxPayloadReq).ulschMacPduTxHdr.parSetBodyLength = 4;  // =4+tbs, update in loop

  (*p_phyUlTxPayloadReq).ulschMacPduTxBody.macPduIndex = 0;  // currently sending only one PDU per TTI
  (*p_phyUlTxPayloadReq).ulschMacPduTxBody.macPduSize  = 0;  // =tbs, update in loop

  memset((*p_phyUlTxPayloadReq).ulschMacPduTxBody.macPdu, 0, MAX_MAC_PDU_SIZE);

  return 0;
}
//=============================================================================================================================
//=============================================================================================================================
} //namespace ns3
