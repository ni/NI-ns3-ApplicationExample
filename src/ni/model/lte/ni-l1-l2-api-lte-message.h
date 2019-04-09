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

#include <cstdint>
#include "ni-l1-l2-api-lte.h"
//#include <boost/date_time/posix_time/posix_time.hpp>  // timestamps


namespace ns3 {

//=============================================================================================================================
//=============================================================================================================================

  // manipulate buffer
  int32_t InsertU64( uint8_t* p_buffer, uint32_t offset, uint64_t value );
  int32_t InsertU32( uint8_t* p_buffer, uint32_t offset, uint32_t value );
  int32_t ExtractU64( uint8_t* p_buffer, uint32_t offset, uint64_t* p_value );
  int32_t ExtractU32( uint8_t* p_buffer, uint32_t offset, uint32_t* p_value );
  int32_t GetMsgType( uint32_t* p_msgType, uint8_t* p_buffer, uint32_t* p_bufferOffset );
  int32_t GetBodyLength( uint32_t* p_bodyLength, uint8_t* p_buffer, uint32_t* p_bufferOffset );

  // print
  int32_t PrintBuffer( uint8_t* p_buffer, uint32_t bufferOffset, uint32_t length );
  int32_t PrintMsgType( uint32_t msgType );
  int32_t PrintCnfStatus( uint32_t cnfStatus );
  int32_t PrintHeader( GenMsgHdr* p_genMsgHdr, LteSubMsgHdr* p_subMsgHdr );
  int32_t PrintPhyTimingInd( PhyTimingInd* p_phyTimingInd );
  int32_t PrintPhyCnf( PhyCnf* p_phyCnf );
  int32_t PrintPhyDlTxConfigReq( PhyDlTxConfigReq* p_phyDlTxConfigReq );
  int32_t PrintPhyDlTxPayloadReq( PhyDlTxPayloadReq* p_phyDlTxPayloadReq );
  int32_t PrintPhyDlschRxInd( PhyDlschRxInd* p_phyDlschRxInd );

  // initialize
  int32_t InitializePhyDlTxConfigReq( PhyDlTxConfigReq* p_phyDlTxConfigReq );
  int32_t InitializePhyDlTxPayloadReq( PhyDlTxPayloadReq* p_phyDlTxPayloadReq );
  int32_t InitializePhyUlTxPayloadReq( PhyUlTxPayloadReq* p_phyUlTxPayloadReq );

//=============================================================================================================================
//=============================================================================================================================
} //namespace ns3

