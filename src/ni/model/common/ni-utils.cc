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

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/prctl.h> // for setting thread names
#include "ns3/fatal-error.h"
#include "ni-utils.h"
#include "ni-logging.h"

namespace ns3 {

// local variables
static std::vector<ThreadInfo> m_threadInfo;
// local function prototypes
void NiUtilsHandleSignals (int sig);



NiUtils::NiUtils ()
{
  // TODO Auto-generated constructor stub

}

NiUtils::~NiUtils ()
{
  // TODO Auto-generated destructor stub
}

int NiUtils::GetThreadPrioriy (void)
{
  int policy;
  struct sched_param param;
  if(pthread_getschedparam(pthread_self(), &policy, &param) < 0)
    {
      NS_FATAL_ERROR("Error getting thread priority!");
    }
 return param.sched_priority;
}

void NiUtils::SetThreadPrioriy (int priority)
{

  int policy;
  struct sched_param param;
  if(pthread_getschedparam(pthread_self(), &policy, &param) < 0)
  {
    NS_FATAL_ERROR("Error getting thread priority!");
  }
  param.sched_priority = priority;
  if(pthread_setschedparam(pthread_self(), policy, &param) < 0)
  {
    NS_FATAL_ERROR("Error setting thread priority!");
  }
}

void NiUtils::AddThreadInfo (pthread_t threadId, std::string threadName)
{
  ThreadInfo threadInfo;
  threadInfo.id = threadId;
  threadInfo.name = threadName;
  m_threadInfo.push_back(threadInfo);
  //pthread_setname_np(threadId, threadName.c_str());
  prctl(PR_SET_NAME, threadName.c_str(),0,0,0);
}

void NiUtils::PrintThreadInfo(void)
{
  for(std::vector<ThreadInfo>::iterator it = m_threadInfo.begin(); it != m_threadInfo.end(); ++it) {
      std::cout << "threadName:" << it->name << ", threadId:" << it->id << std::endl;
  }
}

std::string NiUtils::GetThreadName (pthread_t threadId)
{
  std::string threadName = "unknown";
  for(std::vector<ThreadInfo>::iterator it = m_threadInfo.begin(); it != m_threadInfo.end(); ++it) {
      if (it->id == threadId)
        {
          threadName = it->name;
        }
  }
  return threadName;
}

void NiUtils::InstallSignalHandler (void)
{
  const int signals[] = {SIGILL, SIGIOT, SIGFPE, SIGSEGV, SIGBUS, SIGSYS, SIGPIPE};
  const int numSignals = sizeof(signals)/sizeof(int);
  for (int i; i < numSignals; i++)
    {
      // install NI signal handler which prints logs like the last functioned called in case of an error
      signal(signals[i], NiUtilsHandleSignals);
    }
}

// http://man7.org/linux/man-pages/man3/backtrace.3.html
void NiUtils::Backtrace(void)
{
#define BT_BUF_SIZE 100

  int j, nptrs;
  void *buffer[BT_BUF_SIZE];
  char **strings;

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  printf("backtrace() returned %d addresses\n", nptrs);

  /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
       would produce similar output to the following: */

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
      perror("backtrace_symbols");
      exit(EXIT_FAILURE);
  }

  for (j = 0; j < nptrs; j++)
    printf("%s\n", strings[j]);

  free(strings);
}

uint64_t NiUtils::GetSysTime(void)
{
  struct timeval systime;
  gettimeofday(&systime, NULL);
  return (systime.tv_sec * 1000000 + systime.tv_usec);
}


// converts FXP I8<6.2> to double value
double NiUtils::ConvertFxpI8_6_2ToDouble(uint8_t fxp)
{
  // check sign bit (bit 7)
  bool negative = (fxp & 0x80);

  // shift out decimal part
  uint8_t integerPart = fxp >> 2;
  if (negative)
    {
      // sign extension (two upper bits)
      integerPart |= 0xC0;
    }
  // convert to double
  double integerPartDbl = (double) ((int8_t) integerPart);

  // get decimal part
  uint8_t decimalPart = fxp & 0x03;
  // convert to double
  double decimalPartDbl = (double) decimalPart * (double) 0.25;

  return (integerPartDbl + decimalPartDbl);
}

// prints ip addresses of ns-3 nodes
void NiUtils::PrintNodeInfo(std::string nodeName, Ptr<Node> nodePtr)
{
  std::cout << "-- "<< nodeName << " ------------------------------------" << std::endl;
  for (int i = 0; i < nodePtr->GetObject<Ipv4> ()->GetNInterfaces(); ++i)
    {
      for (int j = 0; j < nodePtr->GetObject<Ipv4> ()->GetNAddresses(i); ++j)
        {
          std::cout << "Node(" << nodePtr->GetId() << "),Interface(" << i << "),Address(" << j << ") = " <<
              nodePtr->GetObject<Ipv4> ()->GetAddress (i,j).GetLocal () << std::endl;
        }
    }
}


// local C functions

void NiUtilsHandleSignals (int sig)
{
  pthread_t threadId = pthread_self();
  std::string threadName = NiUtils::GetThreadName(threadId);

  // print signal and thread id
  std::cout <<  std::endl << "ERROR: signal " << sig << " (" << strsignal(sig) << ")"
            << " caught within thread with THREAD_ID = "<< threadId << " (" << threadName << ")" << std::endl;
  // print backtrace
  NiUtils::Backtrace();
  // de-init logging and save log file in case of errors
  NiLoggingDeInit();

  exit(1);
}




} // namespace ns3
