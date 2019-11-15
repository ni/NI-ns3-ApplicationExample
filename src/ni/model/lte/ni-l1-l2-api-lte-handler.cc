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

#include <cstdint>       // integer types
#include "../common/ni-l1-l2-api-common-handler.h"
#include "ni-l1-l2-api-lte-handler.h"

namespace ns3 {

//======================================================================================
static int32_t SerializeMessageHeader(
  GenMsgHdr* p_genMsgHdr,
  LteSubMsgHdr* p_subMsgHdr,
  uint8_t*   p_buffer,
  uint32_t*  p_bufferOffset
)
//======================================================================================
{

  // Serialize general message header.
  SerializeStruct(
    (uint32_t*) p_genMsgHdr,
    genMsgHdrSpec.numEl,
    (uint8_t*) &(genMsgHdrSpec.byteWidth),
    p_buffer,
    p_bufferOffset
  );

  // Serialize sub message header.
  SerializeStruct(
    (uint32_t*) p_subMsgHdr,
    subMsgHdrSpec.numEl,
    (uint8_t*) &(subMsgHdrSpec.byteWidth),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
static int32_t DeserializeMessageHeader(
  GenMsgHdr* p_genMsgHdr,
  LteSubMsgHdr* p_subMsgHdr,
  uint8_t*   p_buffer,
  uint32_t*  p_bufferOffset
)
//======================================================================================
{

  // Deserialize general message header.
  DeserializeStruct(
    (uint32_t*) p_genMsgHdr,
    genMsgHdrSpec.numEl,
    (uint8_t*) &(genMsgHdrSpec.byteWidth),
    p_buffer,
    p_bufferOffset
  );

  // Deserialize sub message header.
  DeserializeStruct(
    (uint32_t*) p_subMsgHdr,
    subMsgHdrSpec.numEl,
    (uint8_t*) &(subMsgHdrSpec.byteWidth),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
static int32_t SerializeParameterSet(
  uint32_t   msgType,
  uint32_t   subMsgType,
  ParSetHdr* p_parSetHdr,
  uint32_t*  p_msgBody,      // Note: Type is uint32_t* in order to support generic parameter sets
  uint8_t*   p_buffer,
  uint32_t*  p_bufferOffset
)
//======================================================================================
{

  // Depending on the message body, select the right specification of its elements.
  uint8_t  num_el;
  uint8_t* p_byte_width;

  switch ( msgType )
  {

    case (PHY_DL_TX_CONFIG_REQ):
    {

      switch ( subMsgType )
      {
        case (DLSCH_TX_CONFIG):
        {
          num_el       = dlschTxConfigBodySpec.numEl;
          p_byte_width = (uint8_t*) &(dlschTxConfigBodySpec.byteWidth);
          break;
        }
        case (DCI_TX_CONFIG_DL_GRANT):
        {
          num_el       = dciTxConfigDlGrantBodySpec.numEl;
          p_byte_width = (uint8_t*) &(dciTxConfigDlGrantBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;

    }

    case (PHY_DL_TX_PAYLOAD_REQ):
    {

      switch ( subMsgType )
      {
        case (DLSCH_MAC_PDU):
        {
          num_el       = dlschMacPduTxBodySpec.numEl;
          p_byte_width = (uint8_t*) &(dlschMacPduTxBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;

    }

    case (PHY_UL_TX_PAYLOAD_REQ):
    {

      switch ( subMsgType )
      {
        case (ULSCH_MAC_PDU):
        {
          num_el       = ulschMacPduTxBodySpec.numEl;
          p_byte_width = (uint8_t*) &(ulschMacPduTxBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;

    }

//************ 5G Message Cases
    case (PHY_5G_DL_TX_CONFIG_REQ):
        {

          switch ( subMsgType )
          {
            case (FiveG_TX_CONFIG):
            {
              num_el       = FiveGTxConfigBodySpec.numEl;
              p_byte_width = (uint8_t*) &(FiveGTxConfigBodySpec.byteWidth);
              break;
            }
            default:
            {
              break;
            }
          }
          break;

        }

    case (PHY_5G_UL_TX_CONFIG_REQ):
            {

              switch ( subMsgType )
              {
                case (FiveG_TX_CONFIG):
                {
                  num_el       = FiveGTxConfigBodySpec.numEl;
                  p_byte_width = (uint8_t*) &(FiveGTxConfigBodySpec.byteWidth);
                  break;
                }
                default:
                {
                  break;
                }
              }
              break;

            }

    case (PHY_5G_DL_RX_CONFIG_REQ):
            {

              switch ( subMsgType )
              {
                case (FiveG_TX_CONFIG):
                {
                  num_el       = FiveGTxConfigBodySpec.numEl;
                  p_byte_width = (uint8_t*) &(FiveGTxConfigBodySpec.byteWidth);
                  break;
                }
                default:
                {
                  break;
                }
              }
              break;

            }

    case (PHY_5G_UL_RX_CONFIG_REQ):
                {

                  switch ( subMsgType )
                  {
                    case (FiveG_TX_CONFIG):
                    {
                      num_el       = FiveGTxConfigBodySpec.numEl;
                      p_byte_width = (uint8_t*) &(FiveGTxConfigBodySpec.byteWidth);
                      break;
                    }
                    default:
                    {
                      break;
                    }
                  }
                  break;

                }

    default:
    {
      break;
    }

  }

  //************ End of 5G Message Cases

  // Serialize parameter set header
  SerializeStruct(
    (uint32_t*) p_parSetHdr,
    parSetHdrSpec.numEl,
    (uint8_t*) &(parSetHdrSpec.byteWidth),
    p_buffer,
    p_bufferOffset
  );

  // Serialize parameter set body
  SerializeStruct(
    (uint32_t*) p_msgBody,
    num_el,
    p_byte_width,
    p_buffer,
    p_bufferOffset
  );

  // Serialize payload separately
  if ( ( msgType == PHY_DL_TX_PAYLOAD_REQ ) and ( subMsgType == DLSCH_MAC_PDU ) )
  {
    SerializePayload(
      (*(DlschMacPduTxBody*)p_msgBody).macPdu,
      (*(DlschMacPduTxBody*)p_msgBody).macPduSize,
      p_buffer,
      p_bufferOffset
    );
  }

  if ( ( msgType == PHY_UL_TX_PAYLOAD_REQ ) and ( subMsgType == ULSCH_MAC_PDU ) )
  {
    SerializePayload(
      (*(UlschMacPduTxBody*)p_msgBody).macPdu,
      (*(UlschMacPduTxBody*)p_msgBody).macPduSize,
      p_buffer,
      p_bufferOffset
    );
  }

  return 0;
}
//======================================================================================
//======================================================================================




//======================================================================================
static int32_t DeserializeParameterSet(
  uint32_t   msgType,
  uint32_t   subMsgType,
  ParSetHdr* p_parSetHdr,
  uint32_t*  p_msgBody,      // Note: Type is uint32_t* in order to support generic parameter sets
  uint8_t*   p_buffer,
  uint32_t*  p_bufferOffset
)
//======================================================================================
{

  // Depending on the message body, select the right specification of its elements.
  uint8_t  num_el;
  uint8_t* p_byte_width;

  switch ( msgType )
  {

    case (PHY_DLSCH_RX_IND):
    {

      switch ( subMsgType )
      {
        case (DLSCH_MAC_PDU):
        {
          num_el       = dlschMacPduRxBodySpec.numEl;
          p_byte_width = (uint8_t*) &(dlschMacPduRxBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }

    case (PHY_ULSCH_RX_IND):
    {

      switch ( subMsgType )
      {
        case (ULSCH_MAC_PDU):
        {
          num_el       = ulschMacPduRxBodySpec.numEl;
          p_byte_width = (uint8_t*) &(ulschMacPduRxBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }

    case (PHY_CELL_MEASUREMENT_IND):
    {
      switch ( subMsgType )
      {
        case (CELL_MEASUREMENT_REPORT):
        {
          num_el       = cellMeasReportBodySpec.numEl;
          p_byte_width = (uint8_t*) &(cellMeasReportBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }

    case (PHY_CNF):
    {

      switch ( subMsgType )
      {
        case (0):
        {
          num_el       = cnfBodySpec.numEl;
          p_byte_width = (uint8_t*) &(cnfBodySpec.byteWidth);
          break;
        }
        default:
        {
          break;
        }
      }
      break;

    }

    default:
    {
      break;
    }

  }

  // Serialize parameter set header
  DeserializeStruct(
    (uint32_t*) p_parSetHdr,
    parSetHdrSpec.numEl,
    (uint8_t*) &(parSetHdrSpec.byteWidth),
    p_buffer,
    p_bufferOffset
  );

  // Serialize parameter set body
  DeserializeStruct(
    (uint32_t*) p_msgBody,
    num_el,
    p_byte_width,
    p_buffer,
    p_bufferOffset
  );

  // Serialize payload separately
  if ( ( msgType == PHY_DLSCH_RX_IND ) and ( subMsgType == DLSCH_MAC_PDU ) )
  {
    DeserializePayload(
      ((DlschMacPduRxBody*)p_msgBody)->macPdu,
      ((DlschMacPduRxBody*)p_msgBody)->macPduSize,
      p_buffer,
      p_bufferOffset
    );
  }

  if ( ( msgType == PHY_ULSCH_RX_IND ) and ( subMsgType == ULSCH_MAC_PDU ) )
  {
    DeserializePayload(
      ((UlschMacPduRxBody*)p_msgBody)->macPdu,
      ((UlschMacPduRxBody*)p_msgBody)->macPduSize,
      p_buffer,
      p_bufferOffset
    );
  }

  if ( ( msgType == PHY_CELL_MEASUREMENT_IND ) and ( subMsgType == CELL_MEASUREMENT_REPORT ) )
  {
      // SINR array is of type uint8 -> here we can re-use the de-serialize function for payload
      DeserializePayload(
        ((CellMeasReportBody*)p_msgBody)->subbandSinr,
        ((CellMeasReportBody*)p_msgBody)->numSubbandSinr,
        p_buffer,
        p_bufferOffset
      );
  }

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
// Serialize the MAC PDU, i.e., copy it from the payload buffer to the NIAPI message buffer
static int32_t SerializePayload(
  uint8_t*  p_payloadBuffer,
  uint32_t  num_el,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  for (uint32_t i = 0; i < num_el; i++ )
  {
    p_buffer[(*p_bufferOffset)] = p_payloadBuffer[i];
    (*p_bufferOffset)++;
  }

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
// Deserialize the MAC PDU, i.e., copy it from the NIAPI message buffer to the payload buffer
static int32_t DeserializePayload(
  uint8_t*  p_payloadBuffer,
  uint32_t  num_el,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  for (uint32_t i = 0; i < num_el; i++ )
  {
    p_payloadBuffer[i] = p_buffer[(*p_bufferOffset)];
    (*p_bufferOffset)++;
  }

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
int32_t SerializePhyDlTxConfigReq(
  PhyDlTxConfigReq* p_phyDlTxConfigReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  SerializeMessageHeader(
    &(p_phyDlTxConfigReq->genMsgHdr),
    &(p_phyDlTxConfigReq->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  SerializeParameterSet(
    PHY_DL_TX_CONFIG_REQ,
    DLSCH_TX_CONFIG,
    &(p_phyDlTxConfigReq->dlschTxConfigHdr),
    (uint32_t*) &(p_phyDlTxConfigReq->dlschTxConfigBody),
    p_buffer,
    p_bufferOffset
  );

  SerializeParameterSet(
    PHY_DL_TX_CONFIG_REQ,
    DCI_TX_CONFIG_DL_GRANT,
    &(p_phyDlTxConfigReq->dciTxConfigDlGrantHdr),
    (uint32_t*) &(p_phyDlTxConfigReq->dciTxConfigDlGrantBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================


//*********************** Begin of 5G Messages ***********************

int32_t SerializePhyFiveGDlTxConfigReq(
  Phy5GDlTxConfigReq* p_phy5GDlTxConfigReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  SerializeMessageHeader(
    &(p_phy5GDlTxConfigReq->genMsgHdr),
    &(p_phy5GDlTxConfigReq->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  SerializeParameterSet(
    PHY_5G_DL_TX_CONFIG_REQ,
    FiveG_TX_CONFIG,
    &(p_phy5GDlTxConfigReq->FiveGDlTxConfigHdr),
    (uint32_t*) &(p_phy5GDlTxConfigReq->FiveGDlTxConfigBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================


int32_t SerializePhyFiveGDlRxConfigReq(
  Phy5GDlRxConfigReq* p_phy5GDlRxConfigReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  SerializeMessageHeader(
    &(p_phy5GDlRxConfigReq->genMsgHdr),
    &(p_phy5GDlRxConfigReq->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  SerializeParameterSet(
    PHY_5G_DL_RX_CONFIG_REQ,
    FiveG_TX_CONFIG,
    &(p_phy5GDlRxConfigReq->FiveGDlRxConfigHdr),
    (uint32_t*) &(p_phy5GDlRxConfigReq->FiveGDlRxConfigBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================


int32_t SerializePhyFiveGUlTxConfigReq(
  Phy5GUlTxConfigReq* p_phy5GUlTxConfigReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  SerializeMessageHeader(
    &(p_phy5GUlTxConfigReq->genMsgHdr),
    &(p_phy5GUlTxConfigReq->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  SerializeParameterSet(
    PHY_5G_DL_RX_CONFIG_REQ,
    FiveG_TX_CONFIG,
    &(p_phy5GUlTxConfigReq->FiveGUlTxConfigHdr),
    (uint32_t*) &(p_phy5GUlTxConfigReq->FiveGUlTxConfigBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================


int32_t SerializePhyFiveGUlRxConfigReq(
  Phy5GUlRxConfigReq* p_phy5GUlRxConfigReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  SerializeMessageHeader(
    &(p_phy5GUlRxConfigReq->genMsgHdr),
    &(p_phy5GUlRxConfigReq->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  SerializeParameterSet(
    PHY_5G_DL_RX_CONFIG_REQ,
    FiveG_TX_CONFIG,
    &(p_phy5GUlRxConfigReq->FiveGUlRxConfigHdr),
    (uint32_t*) &(p_phy5GUlRxConfigReq->FiveGUlRxConfigBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================

//*********************** End of 5G Messages ***********************



//======================================================================================
int32_t SerializePhyDlTxPayloadReq(
  PhyDlTxPayloadReq* p_phyDlTxPayloadReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

    SerializeMessageHeader(
      &(p_phyDlTxPayloadReq->genMsgHdr),
      &(p_phyDlTxPayloadReq->subMsgHdr),
      p_buffer,
      p_bufferOffset
    );

    SerializeParameterSet(
      PHY_DL_TX_PAYLOAD_REQ,
      DLSCH_MAC_PDU,
      &(p_phyDlTxPayloadReq->dlschMacPduTxHdr),
      (uint32_t*) &(p_phyDlTxPayloadReq->dlschMacPduTxBody),
      p_buffer,
      p_bufferOffset
    );

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
int32_t SerializePhyUlTxPayloadReq(
  PhyUlTxPayloadReq* p_phyUlTxPayloadReq,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

    SerializeMessageHeader(
      &(p_phyUlTxPayloadReq->genMsgHdr),
      &(p_phyUlTxPayloadReq->subMsgHdr),
      p_buffer,
      p_bufferOffset
    );

    SerializeParameterSet(
      PHY_UL_TX_PAYLOAD_REQ,
      ULSCH_MAC_PDU,
      &(p_phyUlTxPayloadReq->ulschMacPduTxHdr),
      (uint32_t*) &(p_phyUlTxPayloadReq->ulschMacPduTxBody),
      p_buffer,
      p_bufferOffset
    );

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
int32_t DeserializePhyTimingInd(
  PhyTimingInd* p_phyTimingInd,
  uint8_t*      p_buffer,
  uint32_t*     p_bufferOffset
)
//======================================================================================
{

  DeserializeMessageHeader(
    &(p_phyTimingInd->genMsgHdr),
    &(p_phyTimingInd->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================




//======================================================================================
int32_t DeserializePhyDlschRxInd(
  PhyDlschRxInd* p_phyDlschRxInd,
  uint8_t*       p_buffer,
  uint32_t*      p_bufferOffset
)
//======================================================================================
{

  DeserializeMessageHeader(
    &(p_phyDlschRxInd->genMsgHdr),
    &(p_phyDlschRxInd->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  DeserializeParameterSet(
    PHY_DLSCH_RX_IND,
    DLSCH_MAC_PDU,
    &(p_phyDlschRxInd->dlschMacPduRxHdr),
    (uint32_t*) &(p_phyDlschRxInd->dlschMacPduRxBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================


//======================================================================================
int32_t DeserializePhyCellMeasurementInd(
  PhyCellMeasInd* p_phyCellMeasInd,
  uint8_t*       p_buffer,
  uint32_t*      p_bufferOffset
)
//======================================================================================
{

  DeserializeMessageHeader(
    &(p_phyCellMeasInd->genMsgHdr),
    &(p_phyCellMeasInd->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  DeserializeParameterSet(
    PHY_CELL_MEASUREMENT_IND,
    CELL_MEASUREMENT_REPORT,
    &(p_phyCellMeasInd->cellMeasReportHdr),
    (uint32_t*) &(p_phyCellMeasInd->cellMeasReportBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================


//======================================================================================
int32_t DeserializePhyUlschRxInd(
  PhyUlschRxInd* p_phyUlschRxInd,
  uint8_t*       p_buffer,
  uint32_t*      p_bufferOffset
)
//======================================================================================
{

  DeserializeMessageHeader(
    &(p_phyUlschRxInd->genMsgHdr),
    &(p_phyUlschRxInd->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  DeserializeParameterSet(
    PHY_ULSCH_RX_IND,
    ULSCH_MAC_PDU,
    &(p_phyUlschRxInd->ulschMacPduRxHdr),
    (uint32_t*) &(p_phyUlschRxInd->ulschMacPduRxBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
int32_t DeserializePhyCnf(
  PhyCnf*   p_phyCnf,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
)
//======================================================================================
{

  DeserializeMessageHeader(
    &(p_phyCnf->genMsgHdr),
    &(p_phyCnf->subMsgHdr),
    p_buffer,
    p_bufferOffset
  );

  DeserializeParameterSet(
    PHY_CNF,
    0,
    &(p_phyCnf->cnfHdr),
    (uint32_t*) &(p_phyCnf->cnfBody),
    p_buffer,
    p_bufferOffset
  );

  return 0;
}
//======================================================================================
//======================================================================================

} //namespace ns3
