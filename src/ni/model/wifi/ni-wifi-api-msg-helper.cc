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

#include "ni-wifi-api-msg-helper.h"
#include "ni-wifi-api-msg-types.h"

#include "stdio.h"
#include "unistd.h"

uint32_t CompareMsg(
    NiapiCommonHeader*  p_msgHdr1,
    uint32_t*           p_msgBody1,
    NiapiCommonHeader*  p_msgHdr2,
    uint32_t*           p_msgBody2
)
{
  uint8_t pass = 1; 	// pass variable that is being written during every check

  // message type from General Message Header to differentiate between messages
  uint32_t msgType = p_msgHdr1->genMsgHdr.msgType;

  // Compare message headers
  if ((p_msgHdr1->genMsgHdr.msgType)     == (p_msgHdr2->genMsgHdr.msgType)    &&
      (p_msgHdr1->genMsgHdr.refId)       == (p_msgHdr2->genMsgHdr.refId)	    &&
      (p_msgHdr1->genMsgHdr.instId)      == (p_msgHdr2->genMsgHdr.instId)	    &&
      (p_msgHdr1->genMsgHdr.bodyLength)  == (p_msgHdr2->genMsgHdr.bodyLength) &&
      (p_msgHdr1->subMsgHdr.resv)        == (p_msgHdr2->subMsgHdr.resv)       &&
      (p_msgHdr1->subMsgHdr.timestmp)    == (p_msgHdr2->subMsgHdr.timestmp)   &&
      (p_msgHdr1->subMsgHdr.numSubMsg)   == (p_msgHdr2->subMsgHdr.numSubMsg)  &&
      (p_msgHdr1->subMsgHdr.cnfMode)     == (p_msgHdr2->subMsgHdr.cnfMode)) 			pass = 1;
  else 																				pass = 0;

  // compare message bodies (parameter set header + parameter set)
  // in every case iterate over the message bodies as a whole, compare each parameter value and update pass
  switch (msgType)
  {
    case (TX_CONFIG_REQ):
      for (uint32_t i = 0; i < (msduTxParamsSpec.numEl + phyTxParamsSpec.numEl); i++)
        {
          if (*(p_msgBody1+i) == *(p_msgBody2+i))	pass &= 1;
          else
            pass = 0;

        }
      break;

    case (TX_PAYLOAD_REQ):
    case (RX_PAYLOAD_IND):
      // first check if MSDU lengths are equals
      if (*(p_msgBody1 + msduTxPayloadSpec.numEl - 1) == *(p_msgBody2 + msduTxPayloadSpec.numEl - 1))
        {
          for (uint8_t i = 0; i < (msduTxPayloadSpec.numEl + *(p_msgBody1 + msduTxPayloadSpec.numEl - 1)); i++)
            {
              if (*(p_msgBody1+i) == *(p_msgBody2+i))	pass &= 1;
              else 									pass = 0;
              //printf("*(p_msgBody1+i) = %u, *(p_msgBody2+i) = %u\n", *(p_msgBody1+i), *(p_msgBody2+i));
              //printf("For i = %u: pass = %u\n", i, pass);
            }
        }
      else
        pass = 0;
      break;

    case (RX_CONFIG_IND):
      for (uint8_t i = 0; i < (msduTxParamsSpec.numEl + phyTxParamsSpec.numEl + macRxStatusSpec.numEl + addMsduRxParamsSpec.numEl); i++)
        {
          if (*(p_msgBody1+i) == *(p_msgBody2+i))
            pass &= 1;
          else
            pass = 0;
        }
      break;
  }

  if (pass == 1)	printf(" ---> PASS: The messages are equal! <---\n\n");
  else			printf(" ---> FAIL: The messages are not equal! <---\n\n");

  return 0;
}

uint32_t PrintBufferU8(
    uint8_t*  p_buffer,
    uint32_t* p_bufferOffset,
    uint8_t colorCode
)
{
  switch (colorCode)
  {
    case(31):
			      printf("\033[0;31m");
    break;
    case(32):
  			      printf("\033[0;32m");
    break;
    case(33):
  		  	      printf("\033[0;33m");
    break;
    case(34):
  		  	      printf("\033[0;34m");
    break;
    case(35):
  		  	      printf("\033[0;35m");
    break;
    case(36):
  		  	      printf("\033[0;36m");
    break;
  }

  for (uint32_t i = 0; i < *p_bufferOffset; ++i)
    {
      if ((i!=0) && (i%16 == 0)) printf("\n");

      printf("%02x", p_buffer[i]);

      if ((i != *p_bufferOffset-1)  || (i == 0) || (i%14 != 0)) printf (" | ");
    }

  printf("\033[0m\n\n");

  return 0;
}

uint32_t PrintMessage(
    NiapiCommonHeader*  p_msgHdr,
    uint32_t*           p_msgBody
)
{

  PrintMessageHeader(
      p_msgHdr
  );

  // message type from General Message Header to differentiate between messages
  uint32_t msgType = p_msgHdr->genMsgHdr.msgType;

  // this parameter is needed to access the right parameter set inside the message body
  uint32_t msgBodyOffset;

  switch (msgType)
  {
    case (TX_CONFIG_REQ):
      PrintMessageBody(p_msgBody, MSDU_TX_PARAMS);
      msgBodyOffset = msduTxParamsSpec.numEl;	// number of elements in the previous parameter set
      PrintMessageBody(p_msgBody + msgBodyOffset, PHY_TX_PARAMS);
      break;

    case (TX_PAYLOAD_REQ):
    case (RX_PAYLOAD_IND):
      PrintMessageBody(p_msgBody, MSDU_TX_PAYLOAD);
      break;

    case (RX_CONFIG_IND):
      PrintMessageBody(p_msgBody, MSDU_TX_PARAMS);
      msgBodyOffset = msduTxParamsSpec.numEl;	// number of elements in the previous parameter set
      PrintMessageBody(p_msgBody + msgBodyOffset, PHY_TX_PARAMS);
      msgBodyOffset += phyTxParamsSpec.numEl;	// number of elements in the previous parameter sets
      PrintMessageBody(p_msgBody + msgBodyOffset, ADD_MSDU_RX_PARAMS);
      msgBodyOffset += addMsduRxParamsSpec.numEl;	// number of elements in the previous parameter sets
      PrintMessageBody(p_msgBody + msgBodyOffset, MAC_RX_STATUS);
      break;
  }

  return 0;
}

uint32_t PrintMessageHeader(
    NiapiCommonHeader*  p_msgHdr
)
{
  printf( "----------------------------------------------------------\n" );
  printf( "------------------- General Message Header ---------------\n" );
  printf( "----------------------------------------------------------\n" );
  printf( "genMsgHdr.msgType                       = 0x%x\n", (*p_msgHdr).genMsgHdr.msgType);
  printf( "genMsgHdr.refId                         = %i\n", (*p_msgHdr).genMsgHdr.refId);
  printf( "genMsgHdr.instId                        = %i\n", (*p_msgHdr).genMsgHdr.instId);
  printf( "genMsgHdr.bodyLength                    = %i\n", (*p_msgHdr).genMsgHdr.bodyLength);
  printf( "----------------------------------------------------------\n" );
  printf( "----------------------- SAP-Sub Header -------------------\n" );
  printf( "----------------------------------------------------------\n" );
  printf( "subMsgHdr.resv                          = %i\n", (*p_msgHdr).subMsgHdr.resv);
  printf( "subMsgHdr.timestmp                      = %#x\n", (*p_msgHdr).subMsgHdr.timestmp);
  printf( "subMsgHdr.numSubMsg                     = %i\n", (*p_msgHdr).subMsgHdr.numSubMsg);
  printf( "subMsgHdr.cnfMode                       = %i\n", (*p_msgHdr).subMsgHdr.cnfMode);

  return 0;
}

uint32_t PrintMessageBody(
    uint32_t*  p_msgBody,
    uint32_t   subMsgType
)
{
  switch (subMsgType)
  {
    case (MSDU_TX_PARAMS):
    {
      MsduTxParams* p_body = (MsduTxParams*)p_msgBody;
      printf( "----------------------------------------------------------\n" );
      printf( "--------------------- Sub-Message Header -----------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "msduTxParamsBody.subMsgType             = %i\n", (*p_body).subMsgType);
      printf( "msduTxParamsBody.paramSetId             = %i\n", (*p_body).paramSetId);
      printf( "msduTxParamsBody.parSetLength           = %i\n", (*p_body).parSetLength);
      printf( "----------------------------------------------------------\n" );
      printf( "---------------------- MSDU parameters -------------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "msduTxParamsBody.msduIndex              = %i\n", (*p_body).msduIndex);
      printf( "msduTxParamsBody.frameType              = %i\n", (*p_body).frameType);
      printf( "msduTxParamsBody.subType                = %i\n", (*p_body).subType);
      printf( "msduTxParamsBody.toDs                   = %i\n", (*p_body).toDs);
      printf( "msduTxParamsBody.fromDs                 = %i\n", (*p_body).fromDs);
      printf( "msduTxParamsBody.powerManag             = %i\n", (*p_body).powerManag);
      printf( "msduTxParamsBody.moreData               = %i\n", (*p_body).moreData);
      printf( "msduTxParamsBody.protecFrame            = %i\n", (*p_body).protecFrame);
      printf( "msduTxParamsBody.htc                    = %i\n", (*p_body).htc);
      printf( "msduTxParamsBody.sourceAddr[6]          = ");
      for (int i=0; i<6; i++)
        {
          printf("%02x", (*p_body).sourceAddr[i]);
          if (i<5) printf(":");
          else printf("\n");
        }
      printf( "msduTxParamsBody.destAddr[6]            = ");
      for (int i=0; i<6; i++)
        {
          printf("%02x", (*p_body).destAddr[i]);
          if (i<5) printf(":");
          else printf("\n");
        }
      printf( "msduTxParamsBody.bssid[6]               = ");
      for (int i=0; i<6; i++)
        {
          printf("%02x", (*p_body).bssid[i]);
          if (i<5) printf(":");
          else printf("\n");
        }
      printf( "msduTxParamsBody.recipAddr[6]           = ");
      for (int i=0; i<6; i++)
        {
          printf("%02x", (*p_body).recipAddr[i]);
          if (i<5) printf(":");
          else printf("\n");
        }
      printf( "msduTxParamsBody.transmAddr[6]          = ");
      for (int i=0; i<6; i++)
        {
          printf("%02x", (*p_body).transmAddr[i]);
          if (i<5) printf(":");
          else printf("\n");
        }
      printf( "msduTxParamsBody.msduLength             = %#x\n", (*p_body).msduLength);
      printf( "----------------------------------------------------------\n" );
      printf( "\n" );
      break;
    }

    case (PHY_TX_PARAMS):
    {
      PhyTxParams* p_body = (PhyTxParams*) p_msgBody;
      printf( "----------------------------------------------------------\n" );
      printf( "--------------------- Sub-Message Header -----------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "phyTxParamsBody.subMsgType              = %i\n", (*p_body).subMsgType);
      printf( "phyTxParamsBody.paramSetId              = %i\n", (*p_body).paramSetId);
      printf( "phyTxParamsBody.parSetLength            = %i\n", (*p_body).parSetLength);
      printf( "----------------------------------------------------------\n" );
      printf( "--------------------- PHY parameters ---------------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "phyTxParamsBody.msduIndex               = %i\n", (*p_body).msduIndex);
      printf( "phyTxParamsBody.format                  = %i\n", (*p_body).format);
      printf( "phyTxParamsBody.bandwidth               = %i\n", (*p_body).bandwidth);
      printf( "phyTxParamsBody.mcs                     = %i\n", (*p_body).mcs);
      printf( "----------------------------------------------------------\n" );
      printf( "\n" );
      break;
    }

    case (MSDU_TX_PAYLOAD):
    {
      MsduTxPayload* p_body = (MsduTxPayload*) p_msgBody;
      printf( "----------------------------------------------------------\n" );
      printf( "--------------------- Sub-Message Header -----------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "msduTxPayloadBody.subMsgType            = %i\n", (*p_body).subMsgType);
      printf( "msduTxPayloadBody.paramSetId            = %i\n", (*p_body).paramSetId);
      printf( "msduTxPayloadBody.parSetLength          = %i\n", (*p_body).parSetLength);
      printf( "----------------------------------------------------------\n" );
      printf( "---------------------- MSDU payload ----------------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "msduTxPayloadBody.msduIndex             = %i\n", (*p_body).msduIndex);
      printf( "msduTxPayloadBody.reserved              = %i\n", (*p_body).reserved);
      printf( "msduTxPayloadBody.msduLength            = %i\n", (*p_body).msduLength);
      printf( "msduTxPayloadBody.msduData[]            = ");
      for (uint32_t i = 1; i <= (*p_body).msduLength; i++)
        {
          printf("%04x ", (*p_body).msduData[i-1]);
          if (i%8 == 0) printf("\n                                          ");
          if (i == (*p_body).msduLength) printf("\n");
        }
      printf( "----------------------------------------------------------\n" );
      printf( "\n" );
      break;
    }

    case (MAC_RX_STATUS):
    {
      MacRxStatus* p_body = (MacRxStatus*) p_msgBody;
      printf( "----------------------------------------------------------\n" );
      printf( "--------------------- Sub-Message Header -----------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "macRxStatusBody.subMsgType              = %i\n", (*p_body).subMsgType);
      printf( "macRxStatusBody.paramSetId              = %i\n", (*p_body).paramSetId);
      printf( "macRxStatusBody.parSetLength            = %i\n", (*p_body).parSetLength);
      printf( "----------------------------------------------------------\n" );
      printf( "---------------------- MAC RX status ---------------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "macRxStatusBody.msduDupDetStatus        = %i\n", (*p_body).msduDupDetStatus);
      printf( "macRxStatusBody.msduDefragStatus        = %i\n", (*p_body).msduDefragStatus);
      printf( "macRxStatusBody.mpduFiltStatus          = %i\n", (*p_body).mpduFiltStatus);
      printf( "macRxStatusBody.mpduDisassStatus        = %i\n", (*p_body).mpduDisassStatus);
      printf( "macRxStatusBody.mpduLengthCheckStatus   = %i\n", (*p_body).mpduLengthCheckStatus);
      printf( "macRxStatusBody.mpduFcsCheckStatus      = %i\n", (*p_body).mpduFcsCheckStatus);
      printf( "macRxStatusBody.aMpduDeAggrStatus       = %i\n", (*p_body).aMpduDeAggrStatus);
      printf( "----------------------------------------------------------\n" );
      printf( "\n" );
      break;
    }

    case (ADD_MSDU_RX_PARAMS):
    {
      AddMsduRxParams* p_body = (AddMsduRxParams*) p_msgBody;
      printf( "----------------------------------------------------------\n" );
      printf( "--------------------- Sub-Message Header -----------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "addMsduRxParamsBody.subMsgType          = %i\n", (*p_body).subMsgType);
      printf( "addMsduRxParamsBody.paramSetId          = %i\n", (*p_body).paramSetId);
      printf( "addMsduRxParamsBody.parSetLength        = %i\n", (*p_body).parSetLength);
      printf( "----------------------------------------------------------\n" );
      printf( "-------------- Additional MSDU RX parameters -------------\n" );
      printf( "----------------------------------------------------------\n" );
      printf( "addMsduRxParamsBody.retryFlag           = %i\n", (*p_body).retryFlag);
      printf( "----------------------------------------------------------\n" );
      printf( "\n" );
      break;
    }

    default:
        return -1;

  }

  return 0;
};

