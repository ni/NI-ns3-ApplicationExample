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

#pragma once

#include <cstdint>       // integer types
#include "ni-l1-l2-api-lte.h"

namespace ns3 {
//--------------------------------------------------------------------------------------

// these low-level functions are not available outside of this module, hence static

static int32_t SerializeMessageHeader(
  GenMsgHdr* p_genMsgHdr,
  LteSubMsgHdr* p_subMsgHdr,
  uint8_t*   p_buffer,
  uint32_t*  p_bufferOffset
);

static int32_t DeserializeMessageHeader(
  GenMsgHdr* p_genMsgHdr,
  LteSubMsgHdr* p_subMsgHdr,
  uint8_t*   p_buffer,
  uint32_t*  p_bufferOffset
);

static int32_t SerializeParameterSet(
  uint32_t  msgType,
  uint32_t  subMsgType,
  ParSetHdr* p_parSetHdr,
  uint32_t* p_msgBody,      // Note: Type is uint32_t* in order to support generic parameter sets
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
);

static int32_t DeserializeParameterSet(
  uint32_t  msgType,
  uint32_t  subMsgType,
  ParSetHdr* p_parSetHdr,
  uint32_t* p_msgBody,      // Note: Type is uint32_t* in order to support generic parameter sets
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
);

static int32_t SerializePayload(
  uint8_t*  p_payloadBuffer,
  uint32_t  num_el,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
);

static int32_t DeserializePayload(
  uint8_t*  p_payloadBuffer,
  uint32_t  num_el,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
);

//--------------------------------------------------------------------------------------

// Add serialization/deserialization fucntions for each NIAPI message as needed

int32_t SerializePhyDlTxConfigReq(
  PhyDlTxConfigReq* p_phyDlTxConfigReq,
  uint8_t*          p_buffer,
  uint32_t*         p_bufferOffset
);

int32_t SerializePhyDlTxPayloadReq(
  PhyDlTxPayloadReq* p_phyDlTxPayloadReq,
  uint8_t*           p_buffer,
  uint32_t*          p_bufferOffset
);

int32_t DeserializePhyTimingInd(
  PhyTimingInd* p_phyTimingInd,
  uint8_t*      p_buffer,
  uint32_t*     p_bufferOffset
);

int32_t SerializePhyUlTxPayloadReq(
  PhyUlTxPayloadReq* p_phyUlTxPayloadReq,
  uint8_t*           p_buffer,
  uint32_t*          p_bufferOffset
);

int32_t DeserializePhyDlschRxInd(
  PhyDlschRxInd* p_phyDlschRxInd,
  uint8_t*       p_buffer,
  uint32_t*      p_bufferOffset
);

int32_t DeserializePhyCellMeasurementInd(
  PhyCellMeasInd* p_phyCellMeasInd,
  uint8_t*        p_buffer,
  uint32_t*       p_bufferOffset
);

int32_t DeserializePhyUlschRxInd(
  PhyUlschRxInd* p_phyUlschRxInd,
  uint8_t*       p_buffer,
  uint32_t*      p_bufferOffset
);

int32_t DeserializePhyCnf(
  PhyCnf*   p_phyCnf,
  uint8_t*  p_buffer,
  uint32_t* p_bufferOffset
);

//======================================================================================
//======================================================================================
} //namespace ns3
