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

#include <stdio.h>      // printf
#include <stdint.h>     // integer types

#include <fcntl.h>      // for named pipe
#include <sys/stat.h>   // for named pipe
#include <sys/select.h> // for reading pipe with select
#include <unistd.h>
#include <errno.h>      // errno
#include <string.h>     // strerror
#include <time.h>

#include "ni-pipe.h"

namespace ns3
{

  NiPipe::NiPipe ()
  {
    // TODO Auto-generated constructor stub

  }

  NiPipe::~NiPipe ()
  {
    // TODO Auto-generated destructor stub
  }


  int32_t NiPipe::OpenFifo(char* fifoName)
  {
    return mkfifo(fifoName, 0664);
  }

  int32_t NiPipe::CloseFifo(char* fifoName)
  {
    return unlink(fifoName);
  }

  int32_t NiPipe::OpenPipeForTx(char* fifoName, int32_t* pFd)
  {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 250000L;  // loop wait time
    errno = 0;

    //printf( "Writer is waiting for reader to connect...\n" );

    while (1)
      {
        //(*pFd) = open(fifoName, O_WRONLY | O_NONBLOCK);
        (*pFd) = open(fifoName, O_WRONLY );
        if ( (*pFd) < 0 )
          {
            if (errno == 2)
              {
                // expected errno
                //printf( "INFO: %s not available (yet?), will try again. errno=%i: %s\n", fifoName, errno, strerror(errno) );
              }
            else
              {
                // unexpected errno, abort
                printf( "ERROR: open() writer for %s errno=%i: %s\n", fifoName, errno, strerror(errno) );
                return -1;
              }
          }
        else
          {
            // all done
            //printf( "SUCCESS: open() writer for %s fd=%i\n", fifoName, (*pFd) );
            break;
          }
        nanosleep(&ts, NULL);
      }

    return 0;
  }

  int32_t NiPipe::OpenPipeForRx(char* fifoName, int32_t* pFd, fd_set*  pReadFds, int32_t* pFdMax)
  {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 500000L;  // loop wait time
    errno = 0;

    //printf( "Reader is waiting for writer to connect...\n" );

    while (1)
      {
        (*pFd) = open(fifoName, O_RDONLY | O_NONBLOCK );
        //(*pFd) = open(fifoName, O_RDONLY );
        if ( (*pFd) < 0 )
          {
            if (errno == 2)
              {
                // expected errno
                //printf( "INFO: %s not available (yet?), will try again. errno=%i: %s\n", fifoName, errno, strerror(errno) );
              }
            else
              {
                // unexpected errno, abort
                printf( "ERROR: open() reader for %s errno=%i: %s\n", fifoName, errno, strerror(errno) );
                return -1;
              }
          }
        else
          {
            // all done
            //printf( "SUCCESS: open() reader for %s fd=%i\n", fifoName, (*pFd) );
            break;
          }
        nanosleep(&ts, NULL);
      }

    FD_ZERO(pReadFds);          // clear set of file descriptors
    FD_SET ((*pFd), pReadFds);  // add the sockfd to the m_readFds set
    (*pFdMax) = (*pFd)+1;

    return 0;
  }


  int32_t NiPipe::NiPipe::ClosePipe(int32_t* pFd)
  {
    return close((*pFd));
  }

  int32_t NiPipe::PipeWrite(int32_t* pFd, uint8_t* pBufU8, uint16_t len)
  {
    errno = 0;
    return write((*pFd), pBufU8, len);
  }

  // Poll named pipe for data
  int32_t NiPipe::PipeRead(int32_t*  pFd, fd_set*   pReadFds, int32_t*  pFdMax, uint8_t*  pBufU8, uint16_t  max_len)
  {
    errno = 0;
    int32_t nread = 0;

    // set timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10;

    // poll for data
    int32_t temp = 0;
    for(;;)
      {
        temp = select((*pFdMax), pReadFds, NULL, NULL, &tv);     // use select() so that we can specify a timeout
        if (temp < 0)      // error ----------------------------------------
          {
            printf( "ERROR: NiapiNamedPipeHelper.c -> PipeRead() -> select() errno=%i: %s\n", errno, strerror(errno) );
            return -1;
          }
        else if (temp == 0)  // no data --------------------------------------
          {
            if (!FD_ISSET((*pFd), pReadFds))
              {
                FD_CLR((*pFd), pReadFds);
                FD_SET((*pFd), pReadFds);
                if ( (*pFdMax) < ((*pFd)+1) )
                  {
                    (*pFdMax) = (*pFd)+1;
                  }
              }
            break;
          }
        else                 // the descriptor has data ----------------------
          {
            if (FD_ISSET ((*pFd), pReadFds))
              {
                nread = read((*pFd), pBufU8, max_len);
                if (nread < 0 )
                  {
                    printf( "ERROR: NiapiNamedPipeHelper.c -> PipeRead() -> read() errno=%i: %s\n", errno, strerror(errno) );
                    return -2;
                  }
                return nread;
                break;
              }
          }
      }

    return 0;
  }

  int32_t NiPipe::PipeReadOnce(int32_t* pFd, uint8_t* pBufU8, uint16_t max_len)
  {
    errno = 0;
    return read((*pFd), pBufU8, max_len);
  }

} /* namespace ns3 */
