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
#include "ni-l1-l2-api-common-handler.h"

namespace ns3 {

//======================================================================================
// Serialize any NIAPI structure defined in NiapiTypes.h to uint8_t buffer.
int32_t SerializeStruct(
  uint32_t* p_struct,
  uint32_t  num_el,
  uint8_t*  p_byte_width,
  uint8_t*  p_buffer,
  uint32_t* p_buffer_offset
)
//======================================================================================
{

  uint8_t  tmp_buffer = 0; // temporary buffer variable
  uint32_t truncSeq   = 0; // sequence to be truncated into bytes which will be sent to buffer

  // for each parameter in the header
  for ( uint32_t i=0; i<num_el; i++ )
  {
    // if current parameter is already 1 byte large, write it directly into the buffer and increase the buffer offset
    if (*(p_byte_width+i) == 1)
    {
      p_buffer[(*p_buffer_offset)] = p_struct[i];
      (*p_buffer_offset)++;
    }
    else
    {
      // start with most significant byte
      truncSeq = p_struct[i];                       //write current parameter into temporary variable truncSeq
      for (uint32_t j = *(p_byte_width+i); j > 0; j--)
      {
        tmp_buffer = truncSeq >> 8*(j-1);           // truncate variable to 1 byte by shifting, starting with MSB
        p_buffer[(*p_buffer_offset)] = tmp_buffer;  // write this byte into the buffer

        (*p_buffer_offset)++;                       // continue with next U8 buffer line
      }
    }
  }

  return 0;
}
//======================================================================================
//======================================================================================



//======================================================================================
// Deserialize any NIAPI structure defined in NiapiTypes.h from uint8_t buffer.
int32_t DeserializeStruct(
  uint32_t* p_struct,
  uint32_t  num_el,
  uint8_t*  p_byte_width,
  uint8_t*  p_buffer,
  uint32_t* p_buffer_offset
)
//======================================================================================
{

  uint32_t concatSeq = 0; // temporary variable for concatenation of buffer contents

  // for each parameter in the header
  for (uint32_t i=0; i<num_el; i++)
  {
    // if current parameter is already 1 byte large, write buffer directly into the structure and increase the buffer offset
    if (*(p_byte_width+i) == 1)
    {
      p_struct[i] = p_buffer[(*p_buffer_offset)];
      (*p_buffer_offset)++;
    }
    else
    {
      // start with most significant byte
      concatSeq = p_buffer[(*p_buffer_offset)];       // write current buffer content into temporary variable
      for (uint32_t j=1; j < *(p_byte_width+i); j++)
      {
        (*p_buffer_offset)++;                                         // increase buffer offset
        concatSeq = (concatSeq << 8) | p_buffer[(*p_buffer_offset)];  // concatenate buffer contents
      }
      p_struct[i] = concatSeq;        // write concatenated value to corresponding structure
      (*p_buffer_offset)++;           // continue with next line in buffer
    }
  }

  return 0;
}

} // namespace ns3
