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

#ifndef __NIAPIMSGHELPER_H__
#define __NIAPIMSGHELPER_H__

#include "ns3/ni-wifi-api-msg-types.h"   // NIAPI message definitions

uint32_t CompareMsg(
    NiapiCommonHeader*  p_msgHdr1,
    uint32_t*           p_msgBody1,
    NiapiCommonHeader*  p_msgHdr2,
    uint32_t*           p_msgBody2
);

uint32_t PrintBufferU8(
    uint8_t*  p_buffer,
    uint32_t* p_bufferOffset,
    uint8_t colorCode
);

uint32_t PrintMessage(
    NiapiCommonHeader*  p_msgHdr,
    uint32_t*           p_msgBody
);

uint32_t PrintMessageHeader(
    NiapiCommonHeader*  p_msgHdr
);

uint32_t PrintMessageBody(
    uint32_t*           p_msgBody,
    uint32_t			subMsgType
);

#endif
