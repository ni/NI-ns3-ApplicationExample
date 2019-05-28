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

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <semaphore.h>
#include <deque>
#include <sys/time.h>              // for CPU time measurement
#include <bitset>                  // for binary printouts

#include "ns3/system-thread.h"
#include "ns3/simulator.h"
#include "ns3/fatal-error.h"
#include "ns3/nstime.h"

#ifndef NI_LOGGIN_H
#define NI_LOGGIN_H

namespace ns3 {

/**
 *  Logging severity classes and levels.
 */
enum NiLogLevel {
  LOG__NONE           = 0x00000000, //!< No logging.

  LOG__FATAL          = 0x00000001, //!< Fatal error messages only. / Program stop
  LOG__LEVEL_FATAL    = 0x00000001, //!< NI_LOG__FATAL and above.

  LOG__ERROR          = 0x00000002, //!< Serious error messages only.
  LOG__LEVEL_ERROR    = 0x00000003, //!< NI_LOG__ERROR and above.

  LOG__WARN           = 0x00000004, //!< Warning messages.
  LOG__LEVEL_WARN     = 0x00000007, //!< NI_LOG__WARN and above.

  LOG__INFO           = 0x00000008, //!< Informational messages (e.g., banners).
  LOG__LEVEL_INFO     = 0x0000000f, //!< NI_LOG__INFO and above.

  LOG__DEBUG          = 0x00000010, //!< debug messages.
  LOG__LEVEL_DEBUG    = 0x0000001f, //!< NI_LOG__DEBUG and above.

  LOG__TRACE          = 0x00000020, //!< Function tracing.
  LOG__LEVEL_TRACE    = 0x0000003f, //!< LOG_FUNCTION and above.

  LOG__CONSOLE_DEBUG         = 0x00000040, //!< Console debug print outs. DO NOT USE under real-time conditions
  LOG__LEVEL_CONSOLE_DEBUG   = 0x0000007f, //!< NI_LOG__CONSOLE and above.

// currently not considered to filter out:
//  LOG__CONSOLE_INFO        = 0x00000080, //!< Console info print outs. DO NOT USE under real-time conditions
//  LOG__LEVEL_CONSOLE_INFO  = 0x000000ff, //!< NI_LOG__CONSOLE and above.

  LOG__ALL            = 0x0fffffff, //!< Print everything.
  LOG__LEVEL_ALL      = LOG__ALL,   //!< Print everything.

  LOG__PREFIX_FUNC    = 0x80000000, //!< Prefix all trace prints with function.
  LOG__PREFIX_TIME    = 0x40000000, //!< Prefix all trace prints with simulation time.
  LOG__PREFIX_NODE    = 0x20000000, //!< Prefix all trace prints with simulation node.
  LOG__PREFIX_LEVEL   = 0x10000000, //!< Prefix all trace prints with log level (severity).
  LOG__PREFIX_ALL     = 0xf0000000  //!< All prefixes.
};

typedef struct sNiLogInfo {
  uint64_t simTimeUs;
  uint64_t sysTimeUs;
  NiLogLevel logLevel;
  std::string file;
  int line;
  std::string func;
  std::string buffer;
} niLogInfo;

// Logging Mode
#define NI_LOG__INSTANT_WRITE_ENABLE true    // write on the fly to the file
#define NI_LOG__INSTANT_WRITE_DISABLE false  // write to a circular buffer

#define NI_LOG__BUFFER_SIZE (1 << 17)

/**
 * Use \ref to output a message of level LOG__NONE.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_NONE(msg) \
    NiLoggingLog(LOG__NONE, msg)

/**
 * Use \ref  to output a message of level LOG__FATAL.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_FATAL(msg) \
    NiLoggingLog(LOG__FATAL, msg)

/**
 * Use \ref  to output a message of level LOG__ERROR.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_ERROR(msg) \
    NiLoggingLog(LOG__ERROR, msg)

/**
 * Use \ref to output a message of level LOG__WARN.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_WARN(msg) \
    NiLoggingLog(LOG__WARN, msg)

/**
 * Use \ref to output a message of level LOG__INFO.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_INFO(msg) \
    NiLoggingLog(LOG__INFO, msg)

/**
 * Use \ref to output a message of level LOG__DEBUG.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_DEBUG(msg) \
    NiLoggingLog(LOG__DEBUG, msg)

/**
 * Use \ref to output a message of level LOG__DEBUG.
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_TRACE(msg) \
    NiLoggingLog(LOG__TRACE, msg)

/**
 * Use \ref to output a message of level LOG__CONSOLE
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_CONSOLE_DEBUG(msg) \
    NiLoggingLog (LOG__CONSOLE_DEBUG, msg)

/**
 * Use \ref to output a message of level LOG__CONSOLE
 *
 * \param [in] msg The message to log.
 */
#define NI_LOG_CONSOLE_INFO(msg) \
    std::cout << msg << std::endl;\
    fflush( stdout );

#define NI_LOG_ENABLED ({bool retval; retval = g_NiLogging.IsEnable(); retval;})

/**
 * Use \ref Initialize NI logging entity
 *
 * \param [in] LogLevelMask The log level mask.
 * \param [in] LogFileName The file to log.
 * \param [in] SyncToFileInstant If true log messages will be instantaneous written to the log file
 * \param [in] NiLoggingPriority] priority of the logging thread
 */
#define NiLoggingInit(LogLevelMask, LogFileName, SyncToFileInstant, NiLoggingPriority) \
{\
  g_NiLogging.Initialize (LogLevelMask, LogFileName, NiLoggingPriority);\
  if (SyncToFileInstant)\
    {\
      g_NiLogging.EnableSyncToFileInstant();\
    }\
}\

/**
 * Use \ref to output a message according to LogLevel
 *
 * \param [in] LogLevel The log level.
 * \param [in] msg The message to log.
 */
#define NiLoggingLog(LogLevel, msg) \
{\
  if(g_NiLogging.IsEnable() ) \
    {\
      std::stringstream strStream;\
      strStream << msg;\
      g_NiLogging.Write (LogLevel, __FILE__, __LINE__, __func__, strStream.str());\
    }\
}\

#define NiLoggingDeInit() \
{\
  g_NiLogging.DeInitialize();\
}\


//This class is explicitly used to enable logging for multi-threading use case.
//We can't use std::cout, cerr as they introduce too much latency in the system!
class NiLogging
{
public:
  NiLogging(void);
  ~NiLogging(void);
  void Initialize (uint32_t logLevel, std::string fileName, int NiLoggingPriority);
  void DeInitialize (void);
  void Write (const enum NiLogLevel logLevel, const std::string file, const int line, const std::string func, const std::string &buffer);
  bool IsEnable(void);
  void EnableSyncToFileInstant (void);
private:
  void WriteToFile();
  void terminateWriteThread();
  void writeThread();
  void DisableFirstCall(void);
  const std::string PrintHeader(void);

  uint64_t m_curLogBufferEntry;
  std::stringstream m_logStringBuffer[NI_LOG__BUFFER_SIZE];
  std::string m_fileOut;
  pthread_mutex_t m_logMutex;
  bool m_logIsEnable;
  bool m_isFirstCall;
  struct timeval m_firstCallSysTime;; //Wall Clock time when this function is called for the first time
  bool m_syncToFileInstant ; //Boolean variable that tells the simulator to instantaneously synch contents of m_NIAPIStringStream to file
  std::ofstream m_filePtr;
  sem_t m_semMsgCount; //Counts how many messages are in queue
  std::deque<niLogInfo> m_msgQueue;
  bool m_flagStopWriteThread;
  Ptr<SystemThread> m_logThread;
  int m_logThreadPriority;
  uint32_t m_loglevelMask;
};

//global variable for class NiLogging
extern NiLogging g_NiLogging;

// global variables for tracing
extern uint64_t g_logTraceStartSubframeTime;

}
#endif /* NIAPI_DEBUG_H*/
