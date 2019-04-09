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

#include "ns3/ni-wifi-api-msg-handler.h"
#include "stdio.h"
#include "unistd.h"


// generic function for serializing a byte structure
int32_t SerializeStructU8(
    uint32_t* p_struct,
    uint32_t  num_el,
    uint8_t*  p_byte_width,
    uint8_t*  p_buffer,
    uint32_t* p_buffer_offset
)
{
  uint8_t  tmp_buffer = 0;  // temporary buffer variable
  uint32_t truncSeq  = 0;   // sequence to be truncated into bytes which will be sent to buffer

  // for each parameter in the header
  for ( uint32_t i=0; i<num_el; i++ )
    {
      // if current parameter is already 1 byte large, write it directly into the buffer
      // and increase the buffer offset
      if (*(p_byte_width+i) == 1)
        {
          p_buffer[(*p_buffer_offset)] = p_struct[i];
          (*p_buffer_offset)++;
        }

      else
        {
           // write current parameter into temporary variable truncSeq
          truncSeq = p_struct[i];

          for (uint32_t j = *(p_byte_width+i); j > 0; j--)
            {
              tmp_buffer = truncSeq >> 8*(j-1);          // truncate variable to 1 byte by shifting, starting with MSB
              p_buffer[(*p_buffer_offset)] = tmp_buffer; // write this byte into the buffer

              (*p_buffer_offset)++;                      // continue with next U8 buffer line
            }
        }
    }

  return 0;
}

// generic function for deserializing a byte structure
int32_t DeserializeStructU8(
    uint32_t* p_struct,
    uint32_t  num_el,
    uint8_t*  p_byte_width,
    uint8_t*  p_buffer,
    uint32_t* p_buffer_offset
)
{
  uint32_t concatSeq = 0; // temporary variable for concatenation of buffer contents

  // for each parameter in the header
  for (uint32_t i=0; i<num_el; i++)
    {
      // if current parameter is already 1 byte large, write buffer directly into the structure
      // and increase the buffer offset
      if (*(p_byte_width+i) == 1)
        {
          p_struct[i] = p_buffer[(*p_buffer_offset)];
          (*p_buffer_offset)++;
        }

      else
        {
          // write current buffer content into temporary variable
          concatSeq = p_buffer[(*p_buffer_offset)];

          for (uint32_t j=1; j < *(p_byte_width+i); j++)
            {
              // increase buffer offset and concatenate buffer contents
              (*p_buffer_offset)++;
              concatSeq = (concatSeq << 8) | p_buffer[(*p_buffer_offset)];
            }

          // write concatenated value to corresponding structure and continue with next line in buffer
          p_struct[i] = concatSeq;
          (*p_buffer_offset)++;
        }
    }

  return 0;
}

// function for serializing an ni api message header
int32_t SerializeMessageHeader(
    NiapiCommonHeader* p_msgHdr,
    uint8_t*           p_buffer,
    uint32_t*          p_bufferOffset
)
{
  // serialize general message header.
  SerializeStructU8(
      (uint32_t*) &((*p_msgHdr).genMsgHdr),
      genMsgHdrSpec.numEl,
      (uint8_t*) &(genMsgHdrSpec.byteWidth),
      p_buffer,
      p_bufferOffset
  );

  // serialize sub message header.
  SerializeStructU8(
      (uint32_t*) &((*p_msgHdr).subMsgHdr),
      subMsgHdrSpec.numEl,
      (uint8_t*) &(subMsgHdrSpec.byteWidth),
      p_buffer,
      p_bufferOffset
  );

  return 0;
}

// function for deserializing an ni api message header
int32_t DeserializeMessageHeader(
    NiapiCommonHeader* p_msgHdr,
    uint8_t*           p_buffer,
    uint32_t*          p_bufferOffset
)
{
  // deserialize general message header.
  DeserializeStructU8(
      (uint32_t*) &((*p_msgHdr).genMsgHdr),
      genMsgHdrSpec.numEl,
      (uint8_t*) &(genMsgHdrSpec.byteWidth),
      p_buffer,
      p_bufferOffset
  );

  // deserialize sub message header.
  DeserializeStructU8(
      (uint32_t*) &((*p_msgHdr).subMsgHdr),
      subMsgHdrSpec.numEl,
      (uint8_t*) &(subMsgHdrSpec.byteWidth),
      p_buffer,
      p_bufferOffset
  );

  return 0;
}

// function for serializing an ni api message body
int32_t SerializeMessageBody(
    uint32_t* p_msgBody,
    uint32_t  subMsgType,
    uint8_t*  p_buffer,
    uint32_t* p_bufferOffset
)
{
  // depending on the message body, select the right specification of its elements.
  uint8_t  num_el;
  uint8_t* p_byte_width;

  switch ( subMsgType )
  {
    case (MSDU_TX_PARAMS):
      num_el       = msduTxParamsSpec.numEl;
      p_byte_width = (uint8_t*) &(msduTxParamsSpec.byteWidth);
      break;

    case (PHY_TX_PARAMS):
      num_el       = phyTxParamsSpec.numEl;
      p_byte_width = (uint8_t*) &(phyTxParamsSpec.byteWidth);
      break;

    case (MSDU_TX_PAYLOAD):
      num_el       = msduTxPayloadSpec.numEl;
      p_byte_width = (uint8_t*) &(msduTxPayloadSpec.byteWidth);
      break;

    case (MAC_RX_STATUS):
      num_el       = macRxStatusSpec.numEl;
      p_byte_width = (uint8_t*) &(macRxStatusSpec.byteWidth);
      break;

    case (ADD_MSDU_RX_PARAMS):
      num_el       = addMsduRxParamsSpec.numEl;
      p_byte_width = (uint8_t*) &(addMsduRxParamsSpec.byteWidth);
      break;

    default:
        return -1;
  }

  // serialize message body.
  SerializeStructU8(
      (uint32_t*) p_msgBody,
      num_el,
      p_byte_width,
      p_buffer,
      p_bufferOffset
  );

  // For payload messages the actual message body is separated (by definition in ni-wifi-api-msg-types.h)
  // into the MSDU index, reserved and MSDU length parameters on one hand (which are serialized
  // as the other message bodies above) and on the other hand into the MSDU data, which is serialized
  // and written additionally into the buffer.

  if (subMsgType == MSDU_TX_PAYLOAD)
    {
      // access the current MSDU length
      uint32_t msduLength = *(p_msgBody + msduTxPayloadSpec.numEl - 1);

      // temporary byte width array that contains msduLength 1s that are needed to serialize the U8 payload
      uint8_t payload_byte_width [msduLength];
      for (uint32_t i = 0; i < msduLength; i++) payload_byte_width[i] = 1;

      SerializeStructU8(
          (uint32_t*) p_msgBody + msduTxPayloadSpec.numEl,
          msduLength,
          payload_byte_width,
          p_buffer,
          p_bufferOffset
      );
    }

  return 0;
}

// function for deserializing an ni api message body
int32_t DeserializeMessageBody(
    uint32_t* p_msgBody,
    uint32_t  subMsgType,
    uint8_t*  p_buffer,
    uint32_t* p_bufferOffset
)
{
  // depending on the message body, select the right specification of its elements.
  uint8_t  num_el;
  uint8_t* p_byte_width;

  switch (subMsgType)
  {
    case (MSDU_TX_PARAMS):
      num_el      = msduTxParamsSpec.numEl;
      p_byte_width = (uint8_t*) &(msduTxParamsSpec.byteWidth);
      break;

    case (PHY_TX_PARAMS):
      num_el      = phyTxParamsSpec.numEl;
      p_byte_width = (uint8_t*) &(phyTxParamsSpec.byteWidth);
      break;

    case (MSDU_TX_PAYLOAD):
      num_el      = msduTxPayloadSpec.numEl;
      p_byte_width = (uint8_t*) &(msduTxPayloadSpec.byteWidth);
      break;

    case (MAC_RX_STATUS):
      num_el      = macRxStatusSpec.numEl;
      p_byte_width = (uint8_t*) &(macRxStatusSpec.byteWidth);
      break;

    case (ADD_MSDU_RX_PARAMS):
      num_el      = addMsduRxParamsSpec.numEl;
      p_byte_width = (uint8_t*) &(addMsduRxParamsSpec.byteWidth);
      break;

    case (TX_CNF_PARAMS):
      num_el            = TxCnfBodySpec.numEl;
      p_byte_width = (uint8_t*) &(TxCnfBodySpec.byteWidth);
      break;

    default:
        return -1;
  }

  // deserialize message body.
  DeserializeStructU8(
      (uint32_t*) p_msgBody,
      num_el,
      p_byte_width,
      p_buffer,
      p_bufferOffset
  );

  // For payload messages the buffer is deserialized differently than for other message types,
  // starting with the MSDU index, reserved and MSDU length parameters first (using the DeserializeStructU8 function).
  // Afterwards the MSDU data is deserialized as it is of variable size.

  if (subMsgType == MSDU_TX_PAYLOAD)
    {
      // access the current MSDU length
      uint32_t msduLength = *(p_msgBody + msduTxPayloadSpec.numEl - 1);

      // temporary byte width array that contains msduLength 1s that are needed for serializing the U8 payload
      uint8_t payload_byte_width [msduLength];
      for (uint32_t i = 0; i < msduLength; i++) payload_byte_width[i] = 1;

      DeserializeStructU8(
          (uint32_t*) p_msgBody + msduTxPayloadSpec.numEl,
          msduLength,
          payload_byte_width,
          p_buffer,
          p_bufferOffset
      );
    }

  return 0;
}

// function for serializing an ni api message
int32_t SerializeMessage(
    NiapiCommonHeader*  p_msgHdr,
    uint32_t*           p_msgBody,
    uint8_t*            p_buffer,
    uint32_t*           p_bufferOffset
)
{
  // serialize message header
  SerializeMessageHeader(p_msgHdr, p_buffer, p_bufferOffset);

  // message type from general message header to differentiate between messages
  uint32_t msgType = p_msgHdr->genMsgHdr.msgType;

  // this parameter is needed to access the right parameter set inside the message body
  uint32_t msgBodyOffset;

  // serialize message body dependent on message type
  switch (msgType)
  {
    case (TX_CONFIG_REQ):
      SerializeMessageBody(p_msgBody, MSDU_TX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset = msduTxParamsSpec.numEl; // number of elements in the previous parameter set
      SerializeMessageBody(p_msgBody + msgBodyOffset, PHY_TX_PARAMS, p_buffer, p_bufferOffset);
      break;

    case (TX_PAYLOAD_REQ):
      SerializeMessageBody(p_msgBody, MSDU_TX_PAYLOAD, p_buffer, p_bufferOffset);
      break;

    case (RX_CONFIG_IND):
      SerializeMessageBody(p_msgBody, MSDU_TX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset = msduTxParamsSpec.numEl; // number of elements in the previous parameter set
      SerializeMessageBody(p_msgBody + msgBodyOffset, PHY_TX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset += phyTxParamsSpec.numEl; // number of elements in the previous parameter sets
      SerializeMessageBody(p_msgBody + msgBodyOffset, ADD_MSDU_RX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset += addMsduRxParamsSpec.numEl; // number of elements in the previous parameter sets
      SerializeMessageBody(p_msgBody + msgBodyOffset, MAC_RX_STATUS, p_buffer, p_bufferOffset);
      break;

    case (RX_PAYLOAD_IND):
      SerializeMessageBody(p_msgBody, MSDU_TX_PAYLOAD, p_buffer, p_bufferOffset);
      break;

    default:
      return -1;
  }

  return 0;
}

// function for deserializing an ni api message
int32_t DeserializeMessage(
    NiapiCommonHeader*  p_msgHdr,
    uint32_t*           p_msgBody,
    uint8_t*            p_buffer,
    uint32_t*           p_bufferOffset
)
{
  DeserializeMessageHeader(
      p_msgHdr,
      p_buffer,
      p_bufferOffset
  );

  // message type from general message header to differentiate between messages
  uint32_t msgType = p_msgHdr->genMsgHdr.msgType;

  // this parameter is needed to access the right parameter set inside the message body
  uint32_t msgBodyOffset;

  switch (msgType)
  {
    case (TX_CONFIG_REQ):
      DeserializeMessageBody(p_msgBody, MSDU_TX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset = msduTxParamsSpec.numEl; // number of elements in the previous parameter set
      DeserializeMessageBody(p_msgBody + msgBodyOffset, PHY_TX_PARAMS, p_buffer, p_bufferOffset);
      break;

    case (TX_PAYLOAD_REQ):
      DeserializeMessageBody(p_msgBody, MSDU_TX_PAYLOAD, p_buffer, p_bufferOffset);
      break;

    case (RX_CONFIG_IND):
      DeserializeMessageBody(p_msgBody, MSDU_TX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset = msduTxParamsSpec.numEl; // number of elements in the previous parameter set
      DeserializeMessageBody(p_msgBody + msgBodyOffset, PHY_TX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset += phyTxParamsSpec.numEl; // number of elements in the previous parameter sets
      DeserializeMessageBody(p_msgBody + msgBodyOffset, ADD_MSDU_RX_PARAMS, p_buffer, p_bufferOffset);
      msgBodyOffset += addMsduRxParamsSpec.numEl; // number of elements in the previous parameter sets
      DeserializeMessageBody(p_msgBody + msgBodyOffset, MAC_RX_STATUS, p_buffer, p_bufferOffset);
      break;

    case (RX_PAYLOAD_IND):
      DeserializeMessageBody(p_msgBody, MSDU_TX_PAYLOAD, p_buffer, p_bufferOffset);
      break;

    case (TX_CNF):
      DeserializeMessageBody(p_msgBody, TX_CNF_PARAMS, p_buffer, p_bufferOffset);
      break;
  }

  return 0;
}


