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

#ifndef __NIAPITYPES_H__
#define __NIAPITYPES_H__

#include "stdint.h"   // integer types
#include "ns3/ni-l1-l2-api.h"

//======================================================================================
// helpers
//======================================================================================

#define CEILING_POS_I32(X) ((X-(int32_t)(X)) > 0 ? (int32_t)(X+1) : (int32_t)(X))
//#define CEILING_NEG_I32(X) ((X-(int32_t)(X)) < 0 ? (int32_t)(X-1) : (int32_t)(X))
//#define CEILING_I32(X) ( ((X) > 0) ? CEILING_POS_I32(X) : CEILING_NEG_I32(X) )

//======================================================================================
// NIAPI constants
//======================================================================================

// msgType used in GenMsgHdr

// TX
#define TX_CONFIG_REQ         0x5101
#define TX_PAYLOAD_REQ        0x5102
#define TX_CNF	              0x5201

// RX
#define RX_CONFIG_IND         0x5081
#define RX_PAYLOAD_IND        0x5082

// macros used for differentiation between parameter sets
#define MSDU_TX_PARAMS        0x0
#define PHY_TX_PARAMS	      0x1
#define MAC_RX_STATUS         0x2
#define ADD_MSDU_RX_PARAMS    0x3
#define MSDU_TX_PAYLOAD       0x4
#define TX_CNF_PARAMS	      0x5

// confirmation status codes
#define WIFI_CNF_SUCCESS                   0x0
#define WIFI_CNF_UNKNOWN_MESSAGE           0x1
#define WIFI_CNF_MESSAGE_NOT_SUPPORTED     0x2
#define WIFI_CNF_UNKNOWN_PARAMETER_SET     0x3
#define WIFI_CNF_MISSING_PARAMETER_SET     0x4
#define WIFI_CNF_PARAMETER_SET_REPETITION  0x5
#define WIFI_CNF_RANGE_VIOLATION           0x6
#define WIFI_CNF_STATE_VIOLATION           0x7
#define WIFI_CNF_TIMEOUT                   0x8
#define WIFI_CNF_CONFIG_PAYLOAD_MISMATCH   0x9
#define WIFI_CNF_LENGTH_MISMATCH           0xA
#define WIFI_CNF_INPUT_BUFFER_FULL         0xB
#define WIFI_CNF_INTERNAL_ERROR            0xC
#define WIFI_CNF_INSTANCE_ID_MISMATCH      0xD

//======================================================================================
// NIAPI message headers
//======================================================================================

#define MAX_ELEMENTS_IN_STRUCT 43

// this struct holds additional information about how to pack the headers.
typedef struct sElementsSpec {
  uint8_t numEl;
  uint8_t byteWidth[MAX_ELEMENTS_IN_STRUCT];
} ElementsSpec;

static const ElementsSpec genMsgHdrSpec = {
  .numEl     = 4,
  .byteWidth = {2, 2, 1, 3},
};

// sub message header
typedef struct sSubMsgHdr {
  uint32_t resv = 0;	  		// reserved field
  uint32_t timestmp = 0;	  	// timestamp
  uint32_t numSubMsg = 0;	  	// number of sub-messages
  uint32_t cnfMode = 0;		  	// confirmation mode
} SubMsgHdr;

static const ElementsSpec subMsgHdrSpec = {
  .numEl     = 4,
  .byteWidth = {2, 4, 1, 1},
};

//======================================================================================
// NIAPI message bodies:
//  Notice that all parameter set header are included in the message bodies because
//  one message often contains multiple parameter sets.
//======================================================================================

// TX_CONFIG_REQ, RX_CONFIG_IND --> MSDU_TX_PARAMS
typedef struct sMsduTxParams {
  // parameter set header
  uint32_t subMsgType = 0;	// U8
  uint32_t paramSetId = 0;	// U8
  uint32_t parSetLength = 41;	// U16

  // parameter set
  uint32_t msduIndex = 0;	// U8
  uint32_t frameType = 0;	// U8
  uint32_t subType = 0;		// U8
  uint32_t toDs = 0;		// U8
  uint32_t fromDs = 0;		// U8
  uint32_t powerManag = 0;	// U8
  uint32_t moreData = 0;	// U8
  uint32_t protecFrame = 0;	// U8
  uint32_t htc = 0;		// U8

  // all MAC addresses are U48: write them as U8 arrays with 6 elements
  uint32_t sourceAddr[6] = {0, 0, 0, 0, 0, 0};		//U48
  uint32_t destAddr[6] = {0, 0, 0, 0, 0, 0};		//U48
  uint32_t bssid[6] = {0, 0, 0, 0, 0, 0};		//U48
  uint32_t recipAddr[6] = {0, 0, 0, 0, 0, 0};		//U48
  uint32_t transmAddr[6] = {0, 0, 0, 0, 0, 0};		//U48

  uint32_t msduLength = 0;		// U16
} MsduTxParams;				// = 45 bytes

static const ElementsSpec msduTxParamsSpec = {
  .numEl     = 43,
  .byteWidth = { 1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1, \
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, \
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, \
		 1,  1,  1,  1,  1,  1,  2},
};

// TX_CONFIG_REQ, RX_CONFIG_IND --> PHY_TX_PARAMS
typedef struct sPhyTxParams {
  // parameter set header
  uint32_t subMsgType = 1;	// U8
  uint32_t paramSetId = 0;	// U8
  uint32_t parSetLength = 4;	// U16

  // parameter set
  uint32_t msduIndex = 0;	// U8
  uint32_t format = 0;		// U8
  uint32_t bandwidth = 0;	// U8
  uint32_t mcs = 0;		// U8
} PhyTxParams;			// = 8 bytes

static const ElementsSpec phyTxParamsSpec = {
  .numEl     = 7,
  .byteWidth = {1, 1, 2, 1, 1, 1, 1},
};

// TX_PAYLOAD_REQ, RX_PAYLOAD_IND --> MSDU_TX_PAYLOAD
typedef struct sMsduTxPayload {
  // parameter set header
  uint32_t subMsgType = 0;	// U8
  uint32_t paramSetId = 0;	// U8
  uint32_t parSetLength = 4;	// U16, actually MSDU length + 4 (to be defined when creating a message)

  // parameter set
  uint32_t msduIndex = 0;	// U8
  uint32_t reserved = 0;	// U8
  uint32_t msduLength = 0;	// U16

  uint32_t msduData[4065];	// msduLength (max. 4065) * U8
} MsduTxPayload;		// = min. 8 bytes, max. 4073 bytes

// Notice, that msduData is excluded from msduTxPayloadSpec as it of variable length
// and assigned, serialized and deserialized dynamically in NiapiMsgHandler.
static const ElementsSpec msduTxPayloadSpec = {
  .numEl     = 6,
  .byteWidth = {1, 1, 2, 1, 1, 2},
};

// RX_CONFIG_IND --> MAC_RX_STATUS
typedef struct sMacRxStatus {
  // parameter set header
  uint32_t subMsgType = 2;		// U8
  uint32_t paramSetId = 0;		// U8
  uint32_t parSetLength = 7;		// U16

  // parameter set
  uint32_t msduDupDetStatus = 0;	// U8
  uint32_t msduDefragStatus = 0;	// U8
  uint32_t mpduFiltStatus = 0;		// U8
  uint32_t mpduDisassStatus = 0;	// U8
  uint32_t mpduLengthCheckStatus = 0;	// U8
  uint32_t mpduFcsCheckStatus = 0;	// U8
  uint32_t aMpduDeAggrStatus = 0;	// U8
} MacRxStatus;				// = 11 bytes

static const ElementsSpec macRxStatusSpec = {
  .numEl     = 10,
  .byteWidth = {1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
};

// RX_CONFIG_IND --> ADD_MSDU_RX_PARAMS
typedef struct sAddMsduRxParams {
  // parameter set header
  uint32_t subMsgType = 3;		// U8
  uint32_t paramSetId = 0;		// U8
  uint32_t parSetLength = 1;		// U16

  // parameter set
  uint32_t retryFlag = 0;		// U8
} AddMsduRxParams;			// = 5 bytes

static const ElementsSpec addMsduRxParamsSpec = {
  .numEl     = 4,
  .byteWidth = {1, 1, 2, 1},
};

//======================================================================================
// NIAPI complete header
//======================================================================================

typedef struct sNiapiCommonHeader {
  GenMsgHdr       genMsgHdr;
  SubMsgHdr       subMsgHdr;
} NiapiCommonHeader;

//======================================================================================
// NIAPI complete body
//======================================================================================

// TX_CONFIG_REQ body
typedef struct sTxConfigReqBody {
  MsduTxParams   msduTxParams;
  PhyTxParams    phyTxParams;
} TxConfigReqBody;

// TX_PAYLOAD_REQ body
typedef struct sTxPayloadReqBody {
  MsduTxPayload   msduTxPayload;
} TxPayloadReqBody;

// RX_CONFIG_IND body
typedef struct sRxConfigIndBody {
  MsduTxParams    msduRxParams;		// identical parameter set body as defined for MSDU TX parameters, so reuse structure
  PhyTxParams     phyRxParams;		// identical parameter set body as defined for PHY TX parameters
  MacRxStatus     macRxStatus;
  AddMsduRxParams addMsduRxParams;
} RxConfigIndBody;

// RX_PAYLOAD_IND body
typedef struct sRxPayloadIndBody {
  MsduTxPayload   msduRxPayload;	// identical parameter set body as defined for MSDU TX payload
} RxPayloadIndBody;

// TX confirmation
typedef struct sTxCnfBody {

  uint32_t subMsgType = 0 ;	// U8
  uint32_t paramSetId = 0;	// U8
  uint32_t parSetLength = 1;	// U16
  uint32_t cnfStatus;           //U8
} TxCnfBody;

static const ElementsSpec TxCnfBodySpec = {
  .numEl     = 4,
  .byteWidth = {1, 1, 2, 1},
};

#endif
