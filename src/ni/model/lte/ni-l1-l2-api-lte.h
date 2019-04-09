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

#include <cstdint>      // integer types
#include "ns3/ni-l1-l2-api.h" // common API

namespace ns3 {

//======================================================================================
// NIAPI constants
//======================================================================================

// msgType used in GenMsgHdr

// common
#define PHY_TIMING_IND           0x4001   // 16385
#define PHY_CNF                  0x4202   // 16898

// eNB TX
#define PHY_DL_TX_CONFIG_REQ     0x4501   // 17665
#define PHY_DL_TX_PAYLOAD_REQ    0x4502   // 17666

// eNB RX
#define PHY_ULSCH_RX_IND         0x4401   // 17409

// UE TX
#define PHY_UL_TX_PAYLOAD_REQ    0x4902   // 18690

// UE RX
#define PHY_DLSCH_RX_IND         0x4801   // 18433
#define PHY_CELL_MEASUREMENT_IND 0x4803   // 18435

//--------------------------------------------------------------

// subMsgType used in ParSetHdr

#define DLSCH_TX_CONFIG         0x1
#define DCI_TX_CONFIG_DL_GRANT  0x2
#define DLSCH_MAC_PDU           0x0
#define ULSCH_MAC_PDU           0x0
#define CELL_MEASUREMENT_REPORT 0x0

//--------------------------------------------------------------

// confirmation status codes

#define CNF_SUCCESS                   0x0
#define CNF_UNKNOWN_MESSAGE           0x1
#define CNF_MESSAGE_NOT_SUPPORTED     0x2
#define CNF_UNKNOWN_PARAMETER_SET     0x3
#define CNF_MISSING_PARAMETER_SET     0x4
#define CNF_PARAMETER_SET_REPETITION  0x5
#define CNF_RANGE_VIOLATION           0x6
#define CNF_STATE_VIOLATION           0x7
#define CNF_TIMEOUT                   0x8
#define CNF_CONFIG_PAYLOAD_MISMATCH   0x9
#define CNF_LENGTH_MISMATCH           0xA
#define CNF_INPUT_BUFFER_FULL         0xB
#define CNF_INTERNAL_ERROR            0xC
#define CNF_INSTANCE_ID_MISMATCH      0xD
#define CNF_NUM_STATUS_CODES          (CNF_INSTANCE_ID_MISMATCH + 1)

// magic numbers
#define MAX_MAC_PDU_SIZE              9422    // tbs=9422 for prb=110 and mcs=28
#define MAX_NUM_SUBBAND_SINR          13

//======================================================================================
// NIAPI message headers
//======================================================================================

#define LTE_MAX_ELEMENTS_IN_STRUCT 10

// This struct holds information about how to pack the headers.
typedef struct sLteElementsSpec {
  uint32_t numEl;
  uint8_t  byteWidth[LTE_MAX_ELEMENTS_IN_STRUCT];
} LteElementsSpec;



//--------------------------------------------------------------------------------------
// general message header -- 8 bytes
//--------------------------------------------------------------------------------------
static const LteElementsSpec genMsgHdrSpec = {
  .numEl    = 4,
  .byteWidth = {2, 2, 1, 3},
};



//--------------------------------------------------------------------------------------
// sub message header -- 8 bytes
//--------------------------------------------------------------------------------------

typedef struct sLteSubMsgHdr {
  uint32_t sfn;           // system frame number
  uint32_t tti;           // tti or slot index
  uint32_t numSubMsg;     // number of sub messages
  uint32_t cnfMode;       // confirm mode
  uint32_t resField;      // reserved bits
} LteSubMsgHdr;

static const LteElementsSpec subMsgHdrSpec = {
  .numEl    = 5,
  .byteWidth = {2, 1, 1, 1, 3},
};



//--------------------------------------------------------------------------------------
// parameter set header -- 5 bytes
//--------------------------------------------------------------------------------------

typedef struct sParSetHdr {
  uint32_t subMsgType;
  uint32_t parSetId;
  uint32_t parSetBodyLength;
} ParSetHdr;

static const LteElementsSpec parSetHdrSpec = {
  .numEl    = 3,
  .byteWidth = { 1, 1, 3},
};

//======================================================================================
// NIAPI message bodies (= parameter sets)
//======================================================================================

//--------------------------------------------------------------------------------------
// PHY_CNF --> CNF -- 3 bytes
//--------------------------------------------------------------------------------------

typedef struct sCnfBody {
  uint32_t cnfStatus;
  uint32_t srcMsgType;

} CnfBody;

static const LteElementsSpec cnfBodySpec = {
  .numEl    = 2,
  .byteWidth = {1, 2},
};



//--------------------------------------------------------------------------------------
// PHY_DL_TX_CONFIG_REQ --> DLSCH_TX_CONFIG -- 8 bytes
//--------------------------------------------------------------------------------------

typedef struct sDlschTxConfigBody {
  uint32_t macPduIndex;
  uint32_t rnti;
  uint32_t prbAllocation;
  uint32_t mcs;
} DlschTxConfigBody;

static const LteElementsSpec dlschTxConfigBodySpec = {
  .numEl    = 4,
  .byteWidth = {1, 2, 4, 1},
};



//--------------------------------------------------------------------------------------
// PHY_DL_TX_CONFIG_REQ --> DCI_TX_CONFIG_DL_GRANT -- 9 bytes
//--------------------------------------------------------------------------------------

typedef struct sDciTxConfigDlGrantBody {
  uint32_t rnti;
  uint32_t cceOffset;
  uint32_t prbAllocation;
  uint32_t mcs;
  uint32_t tpc;
} DciTxConfigDlGrantBody;

static const LteElementsSpec dciTxConfigDlGrantBodySpec = {
  .numEl    = 5,
  .byteWidth = {2, 1, 4, 1, 1},
};



//--------------------------------------------------------------------------------------
// PHY_DL_TX_PAYLOAD_REQ --> DLSCH_MAC_PDU_TX -- (4+tbs) bytes
//--------------------------------------------------------------------------------------

typedef struct sDlschMacPduTxBody {
  uint32_t macPduIndex;
  uint32_t macPduSize;
  uint8_t  macPdu[MAX_MAC_PDU_SIZE];
} DlschMacPduTxBody;

static const LteElementsSpec dlschMacPduTxBodySpec = {
  .numEl    = 2,            // macPdu is not considered here, handle the uint8_t array separately
  .byteWidth = {1, 3},
};

//--------------------------------------------------------------------------------------
// PHY_UL_TX_PAYLOAD_REQ --> ULSCH_MAC_PDU_TX -- (4+tbs) bytes
//--------------------------------------------------------------------------------------

typedef struct sUlschMacPduTxBody {
  uint32_t macPduIndex;
  uint32_t macPduSize;
  uint8_t  macPdu[MAX_MAC_PDU_SIZE];
} UlschMacPduTxBody;

static const LteElementsSpec ulschMacPduTxBodySpec = {
  .numEl    = 2,            // macPdu is not considered here, handle the uint8_t array separately
  .byteWidth = {1, 3},
};



//--------------------------------------------------------------------------------------
// PHY_DLSCH_RX_IND --> DLSCH_MAC_PDU_RX -- (6+tbs) bytes
//--------------------------------------------------------------------------------------

typedef struct sDlschMacPduRxBody {
  uint32_t rnti;
  uint32_t crcResult;
  uint32_t macPduSize;
  uint8_t macPdu[MAX_MAC_PDU_SIZE];
} DlschMacPduRxBody;

static const LteElementsSpec dlschMacPduRxBodySpec = {
  .numEl    = 3,            // macPdu is not considered here, handle the uint8_t array separately
  .byteWidth = {2, 1, 3},
};

//--------------------------------------------------------------------------------------
// PHY_CELL_MEASUREMENT_IND --> CELL_MEASUREMENT_REPORT -- (4+MAX_NUM_SUBBAND_SINR) bytes
//--------------------------------------------------------------------------------------

typedef struct sCellMeasReportBody {
  uint32_t cellId;
  uint32_t widebandSinr;
  uint32_t numSubbandSinr;
  uint8_t subbandSinr[MAX_NUM_SUBBAND_SINR];
} CellMeasReportBody;

static const LteElementsSpec cellMeasReportBodySpec = {
  .numEl    = 3,            // numSubband array is not considered here, handle the uint8_t array separately
  .byteWidth = {2, 1, 1},
};


//--------------------------------------------------------------------------------------
// PHY_ULSCH_RX_IND --> ULSCH_MAC_PDU_RX -- (4+tbs) bytes
//--------------------------------------------------------------------------------------

typedef struct sUlschMacPduRxBody {
  uint32_t rnti;
  uint32_t crcResult;
  uint32_t macPduSize;
  uint8_t macPdu[MAX_MAC_PDU_SIZE];
} UlschMacPduRxBody;

static const LteElementsSpec ulschMacPduRxBodySpec = {
  .numEl    = 3,            // macPdu is not considered here, handle the uint8_t array separately
  .byteWidth = {2, 1, 3},
};



//======================================================================================
// Definition of complete NIAPI messages
//======================================================================================

typedef struct sPhyTimingInd {
  GenMsgHdr       genMsgHdr;
  LteSubMsgHdr    subMsgHdr;
} PhyTimingInd;

typedef struct sPhyCnf {
  GenMsgHdr    genMsgHdr;
  LteSubMsgHdr subMsgHdr;
  ParSetHdr    cnfHdr;
  CnfBody      cnfBody;
} PhyCnf;

typedef struct sPhyDlTxConfigReq {
  GenMsgHdr              genMsgHdr;
  LteSubMsgHdr           subMsgHdr;
  ParSetHdr              dlschTxConfigHdr;
  DlschTxConfigBody      dlschTxConfigBody;
  ParSetHdr              dciTxConfigDlGrantHdr;
  DciTxConfigDlGrantBody dciTxConfigDlGrantBody;
} PhyDlTxConfigReq;

typedef struct sPhyDlTxPayloadReq {
  GenMsgHdr         genMsgHdr;
  LteSubMsgHdr      subMsgHdr;
  ParSetHdr         dlschMacPduTxHdr;
  DlschMacPduTxBody dlschMacPduTxBody;
} PhyDlTxPayloadReq;

typedef struct sPhyUlTxPayloadReq {
  GenMsgHdr         genMsgHdr;
  LteSubMsgHdr      subMsgHdr;
  ParSetHdr         ulschMacPduTxHdr;
  UlschMacPduTxBody ulschMacPduTxBody;
} PhyUlTxPayloadReq;

typedef struct sPhyDlschRxInd {
  GenMsgHdr         genMsgHdr;
  LteSubMsgHdr      subMsgHdr;
  ParSetHdr         dlschMacPduRxHdr;
  DlschMacPduRxBody dlschMacPduRxBody;
} PhyDlschRxInd;

typedef struct sPhyCellMeasInd {
  GenMsgHdr          genMsgHdr;
  LteSubMsgHdr       subMsgHdr;
  ParSetHdr          cellMeasReportHdr;
  CellMeasReportBody cellMeasReportBody;
} PhyCellMeasInd;

typedef struct sPhyUlschRxInd {
  GenMsgHdr         genMsgHdr;
  LteSubMsgHdr      subMsgHdr;
  ParSetHdr         ulschMacPduRxHdr;
  UlschMacPduRxBody ulschMacPduRxBody;
} PhyUlschRxInd;
//======================================================================================
//======================================================================================

} //namespace ns3
