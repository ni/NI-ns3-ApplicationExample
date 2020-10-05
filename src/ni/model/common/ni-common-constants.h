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

#ifndef SRC_NI_MODEL_COMMON_NI_COMMON_CONSTANTS_H_
#define SRC_NI_MODEL_COMMON_NI_COMMON_CONSTANTS_H_

#include <cstdint>   // integer types

#define NI_MODULE_VERSION_MAJOR 3
#define NI_MODULE_VERSION_MINOR 0
#define NI_MODULE_VERSION_FIX   0

#define NI_MODULE_VERSION (std::to_string(NI_MODULE_VERSION_MAJOR) + "." + std::to_string(NI_MODULE_VERSION_MINOR) + "." + std::to_string(NI_MODULE_VERSION_FIX))

// default NI Application Framework version used for LTE and WIFI
#define NI_AFW_VERSION_MAJOR 2
#define NI_AFW_VERSION_MINOR 2

#define NI_AFW_VERSION (std::to_string(NI_AFW_VERSION_MAJOR) + "." + std::to_string(NI_AFW_VERSION_MINOR))

// default NI Application Framework version used for 5G-GFDM
#define NI_AFW_VERSION_MAJOR_5G_GFDM 2
#define NI_AFW_VERSION_MINOR_5G_GFDM 5

#define NI_AFW_VERSION_5G_GFDM (std::to_string(NI_AFW_VERSION_MAJOR_5G_GFDM) + "." + std::to_string(NI_AFW_VERSION_MINOR_5G_GFDM))


#define NI_COMMON_CONST_MAX_PAYLOAD_SIZE 10240 // bytes

// Key to identify ns-3 payloads
const uint8_t niNs3Key[4] = {0x6e, 0x73, 0x2d, 0x33}; // ASCII "ns-3" -> 6e 73 2d 33


#endif /* SRC_NI_MODEL_COMMON_NI_COMMON_CONSTANTS_H_ */
