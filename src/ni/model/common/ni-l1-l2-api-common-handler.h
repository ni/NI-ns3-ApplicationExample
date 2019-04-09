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

namespace ns3 {

//#include "NiapiTypes.h"   // NIAPI message definitions

int32_t SerializeStruct(uint32_t* p_struct, uint32_t  num_el, uint8_t*  p_byte_width, uint8_t*  p_buffer, uint32_t* p_buffer_offset);
int32_t DeserializeStruct( uint32_t* p_struct, uint32_t  num_el, uint8_t*  p_byte_width, uint8_t*  p_buffer, uint32_t* p_buffer_offset);

} // namespace ns3
