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

#ifndef SRC_NI_HELPER_COMMON_NI_UTILS_H_
#define SRC_NI_HELPER_COMMON_NI_UTILS_H_

#include <iostream>
#include <thread>
#include <vector>
#include <cstdint>

namespace ns3 {


typedef struct sThreadInfo {
  pthread_t id;
  std::string name;
} ThreadInfo;

class NiUtils
{
public:
  NiUtils ();
  static int GetThreadPrioriy(void);
  static void SetThreadPrioriy(int priority);
  static void AddThreadInfo (pthread_t threadId, std::string threadName);
  static void PrintThreadInfo(void);
  static std::string GetThreadName (pthread_t threadId);
  static void InstallSignalHandler(void);
  static void Backtrace(void);
  static uint64_t GetSysTime(void);
  static double ConvertFxpI8_6_2ToDouble(uint8_t fxp);
private:

  virtual
  ~NiUtils ();
};

} // namespace ns3

#endif /* SRC_NI_HELPER_COMMON_NI_UTILS_H_ */
