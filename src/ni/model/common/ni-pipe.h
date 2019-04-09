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
#include <sys/select.h>

#ifndef NI_PIPE_H_
#define NI_PIPE_H_

namespace ns3
{

  class NiPipe
  {
  public:
    NiPipe ();
    virtual
    ~NiPipe ();

     static int32_t OpenFifo(char* fifoName);
     static int32_t CloseFifo(char* fifoName);
     static int32_t OpenPipeForTx(char* fifoName, int32_t* pFd);
     static int32_t OpenPipeForRx(char* fifoName, int32_t* pFd, fd_set* pReadFds, int32_t* pFdMax);
     static int32_t ClosePipe(int32_t* pFd);

     static int32_t PipeWrite(int32_t* pFd, uint8_t* pBufU8, uint16_t len);
     static int32_t PipeRead(int32_t* pFd, fd_set* pReadFds, int32_t* pFdMax, uint8_t* pBufU8, uint16_t max_len);
     static int32_t PipeReadOnce(int32_t* pFd, uint8_t* pBufU8, uint16_t max_len);
  private:

  };

} /* namespace ns3 */

#endif /* SRC_NI_MODEL_COMMON_NI_PIPE_H_ */
