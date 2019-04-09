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

#include <stdint.h>
#include "ni-l1-l2-api-lte-tables.h"


//=============================================================================================================================
// get the LTE transport block size based on MCS and number of allocated PRB
int32_t GetTbs(
  uint32_t  mcs,
  uint32_t  prbAllocationU25,  // 1 active bit = 4 PRB
  uint32_t* p_tbs              // return value in bytes
)
//=============================================================================================================================
{

  // use builtin function popcount to count number of active bits in prb mask
  (*p_tbs) = tbsTableBits[ __builtin_popcountll(prbAllocationU25)*4-1 ][ McsToItbs[mcs] ] / 8;

  return 0;
};
//=============================================================================================================================
//=============================================================================================================================
