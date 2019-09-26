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

#include <mutex> // std::mutex
#include "ns3/wall-clock-synchronizer.h"
#include "ni-lte-constants.h"
#include "ni-lte-sdr-timing-sync.h"
#include "../common/ni-utils.h"
#include "../common/ni-logging.h"
#include "../../model/common/ni-pipe-transport.h"


namespace ns3 {

  // module global variables, in order to exchange values between static and non-static member functions
  // TODO-NI: get rid of static member functions and module glpbal variables
  uint64_t g_phytimeUsIdx = 0;
  int64_t g_phytimeUsAccu = 0;
  int64_t g_globalTimingdiffNano = 0;
  uint64_t g_lastNormalizedRealtime = 0;
  int64_t g_alignmentOffset = 0;
  std::mutex g_timingSyncMutex;

  NiLteSdrTimingSync::NiLteSdrTimingSync()
  {
    // default constructor
  }

  NiLteSdrTimingSync::~NiLteSdrTimingSync()
  {
    // default destructor
  }

  // attach callback to wall clock synchronizer
  void
  NiLteSdrTimingSync::AttachWallClock (void) {
    // hand over callback function to wall clock synchronizer
    WallClockSynchronizerSetCalcNormalizedRealtimeCb(CalcNormalizedRealtime);
  }

  // callback function called from wall clock synchronizer
  uint64_t
  NiLteSdrTimingSync::CalcNormalizedRealtime (uint64_t realtimeNano, uint64_t realtimeOriginNano) {

    g_timingSyncMutex.lock();

    uint64_t normalizedRealtime = 0;
    const bool normalizeToNiSdrTiming = true;
    /** Conversion constant between &mu;s and ns. */
    static const uint64_t US_PER_NS = (uint64_t)1000;
    /** Conversion constant between ns and s. */
    static const uint64_t NS_PER_SEC = (uint64_t)1000000000;
    struct timeval tv;
    gettimeofday (&tv, NULL);
    uint64_t realtimeNanoNow = tv.tv_sec * NS_PER_SEC + tv.tv_usec * US_PER_NS;

    if (realtimeNanoNow != realtimeNano)
      {
        NI_LOG_NONE("realtimeNano adjusted by " << realtimeNanoNow-realtimeNano);
        realtimeNano = realtimeNanoNow;
      }

    if (normalizeToNiSdrTiming)
      {
        //pthread_t threadId = pthread_self();
        //std::string threadName = NiUtils::GetThreadName(threadId);
        //NI_LOG_DEBUG(thisPtr << ", " << threadName << ", " << realtimeNano << ", " << realtimeOriginNano << ", " << g_alignmentOffset << ", " << g_lastNormalizedRealtime);
        //if (threadName == "unknown") NiUtils::Backtrace();

        normalizedRealtime = realtimeNano + g_alignmentOffset - realtimeOriginNano;
        int64_t currentAlignment = 0;
        // skip first iteration
        if (g_lastNormalizedRealtime > 0)
          {
            // calculate  delta to last call
            uint64_t wallClockDelta = normalizedRealtime - g_lastNormalizedRealtime;
            currentAlignment = AlignDiff(wallClockDelta);
            // adjust normalized real time
            normalizedRealtime += currentAlignment;
            NI_LOG_NONE("wallClockDelta: " + std::to_string(wallClockDelta) +
                        ", currentAlignment: " + std::to_string(currentAlignment) +
                        ", normalizedRealtime: " + std::to_string(normalizedRealtime));
            NI_LOG_NONE("[Trace#2],wallClockDelta,currentAlignment,normalizedRealtime," <<
                        wallClockDelta << "," << currentAlignment << "," << normalizedRealtime);
          }
        if (normalizedRealtime < g_lastNormalizedRealtime) {
            NI_LOG_FATAL("ERROR: normalizedRealtime < g_lastNormalizedRealtime (" <<
                         normalizedRealtime << ", " <<  g_lastNormalizedRealtime << ", " <<
                         realtimeNano << ", " << g_alignmentOffset << ", " << realtimeOriginNano << ")");
        }
        // update overall alignment offset
        g_alignmentOffset += currentAlignment;
        // save normalized real time for next iteration
        g_lastNormalizedRealtime = normalizedRealtime;
      }
    else
      {
        // original NS-3 calculation
        normalizedRealtime = realtimeNano - realtimeOriginNano;
      }

    g_timingSyncMutex.unlock();

    return normalizedRealtime;
  }

  // align timing difference between wall clock and FPGA timing indication
  int64_t
  NiLteSdrTimingSync::AlignDiff(uint64_t wallClockDelta)
  {
    int64_t alignment = 0;
    if (llabs(g_globalTimingdiffNano) >= wallClockDelta)
      {
        // compensate timing difference with a fraction of the wall clock call delta

        // align with wallClockDelta / 2 to prevent clock stalling
        uint64_t wallClockDeltaHalf = ((wallClockDelta / 1000) / 2 ) * 1000; // round to millisecond
        if (g_globalTimingdiffNano >= 0)
          {
            // timing indication delayed to CPU time
            g_globalTimingdiffNano -= wallClockDeltaHalf;
            alignment = wallClockDeltaHalf;
          }
        else // g_globalTimingdiffNano < 0
          {
            // timing indication ahead CPU time
            g_globalTimingdiffNano += wallClockDeltaHalf;
            alignment = -wallClockDeltaHalf;
          }
      }
    else // llabs(g_globalTimingdiffNano) < wallClockDelta
      {
        if (g_globalTimingdiffNano > 0 || g_globalTimingdiffNano < 0)
          {
            // compensate remaining timing difference
            alignment = g_globalTimingdiffNano;
            g_globalTimingdiffNano = 0;
          }
        else // g_globalTimingdiffNano == 0
          {
            // do nothing
            alignment = 0;
          }
      }
    if (alignment > 0) {
        NI_LOG_NONE ("g_globalTimingdiffNano: " + std::to_string(g_globalTimingdiffNano) +
                     ", alignment: " + std::to_string(alignment));
    }
    return alignment;
  }

  // calculate timing difference between simulator start subframe and PHY timing indication
  void
  NiLteSdrTimingSync::CalcPhyTimeDiff(int64_t currentDiff)
  {
    g_timingSyncMutex.lock();

    int64_t mean = 0;
    g_phytimeUsAccu += currentDiff;
    NI_LOG_NONE ("g_phytimeUsAccu: " + std::to_string(g_phytimeUsAccu) +
                 ", currentDiff: " + std::to_string(currentDiff) +
                 ", g_globalTimingdiffNano: " + std::to_string(g_globalTimingdiffNano));
    if (g_phytimeUsIdx == (m_phyTimeUsAccuSize-1))
      {
        mean = g_phytimeUsAccu / m_phyTimeUsAccuSize; // calc mean
        g_globalTimingdiffNano += (mean * 1000);      // convert diff to nano seconds
        NI_LOG_DEBUG ("accuUs: " + std::to_string(g_phytimeUsAccu) +
                      ", meanDiffTimIndUs: " + std::to_string(mean) +
                      ", g_globalTimingdiffNano: " + std::to_string(g_globalTimingdiffNano));
        g_phytimeUsAccu = 0;
      }
    g_phytimeUsIdx = (g_phytimeUsIdx + 1) % m_phyTimeUsAccuSize;

    g_timingSyncMutex.unlock();
  }

  // calc and track the TTI/SFN offset (m_sfnSfOffset) between PHY and Simulator timing
  void
  NiLteSdrTimingSync::ReCalcSfnSfOffset(bool firstRun, uint32_t nrFrames, uint32_t nrSubFrames,
                                        uint16_t timingIndSfn, uint8_t timingIndTti,
                                        uint64_t ueToEnbSfoffset, int64_t &sfnSfOffset)
  {
    g_timingSyncMutex.lock();

    const uint16_t maxNumSfn = NI_LTE_CONST_MAX_NUM_SFN;
    const uint8_t maxNumTti = NI_LTE_CONST_MAX_NUM_TTI;
    // correct NS-3 counting in order to calculate subsequently with modulo operator
    uint32_t correctNrFrames = (nrFrames - 1) % maxNumSfn;  // Note: NS-3 "Bug": nrFrames starts from 1 instead 0 and does not wrap as specified in 3GPP
    uint32_t correctNrSubFrames = nrSubFrames - 1;          // Note: NS-3 "Bug": nrSubFrames counts from 1..10 instead 0..9
    // combine SFN and TTI in one number
    int64_t ns3SfnSf = correctNrFrames * maxNumTti + correctNrSubFrames;
    int64_t niSfnSf = timingIndSfn * maxNumTti + timingIndTti;
    if (firstRun)
      {
        // calc inital offset between NS-3 and NI FPGA PHY
        sfnSfOffset = ns3SfnSf - niSfnSf;
        NI_LOG_INFO("Initial offset between NS-3 and NI FPGA PHY: " + std::to_string(sfnSfOffset));
      }
    else
      {
        // add possible offset from MIB update
        if (ueToEnbSfoffset != 0)
          {
            sfnSfOffset += ueToEnbSfoffset;
            NI_LOG_INFO("Offset adjusted from MIB with: " << ueToEnbSfoffset << " to sfnSfOffset: " << sfnSfOffset);
          }
        else // detect timing variations caused by FPGA PHY / LTE Application Framework
          {
            // calc assumed NI FPGA PHY timing based on NS-3 timing and initial offset
            int64_t assumedNiSfnSf = ns3SfnSf - sfnSfOffset;
            assumedNiSfnSf += maxNumSfn * maxNumTti; // needed to handle SFN wrap
            uint16_t assumedPhySfn = (assumedNiSfnSf / maxNumTti) % maxNumSfn;
            uint8_t assumedPhyTti = assumedNiSfnSf % maxNumTti;

            // compare actual NI FPGA PHY timing (timingInd) against assumed NI FPGA PHY timing
            if (timingIndSfn != assumedPhySfn ||
                timingIndTti != assumedPhyTti)
              {
                NI_LOG_WARN("LTE Timing not aligned!" <<
                            ", NS-3 SFN.TTI: " << std::to_string(correctNrFrames) << "." << std::to_string(correctNrSubFrames) <<
                            ", PhyTimingInd SFN.TTI: "  << std::to_string(timingIndSfn) << "." << std::to_string(timingIndTti) <<
                            ", assumed PhyTimingInd SFN.TTI: " << std::to_string(assumedPhySfn) << "." << std::to_string(assumedPhyTti) <<
                            ", sfnSfOffset: " << std::to_string(sfnSfOffset));

                // re-calc offset between NS-3 and NI FPGA PHY
                int64_t oldSfnSfOffset = sfnSfOffset;        // store old offset
                int64_t newSfnSfOffset = ns3SfnSf - niSfnSf; // calc actual offset between NS-3 and NI FPGA PHY
                newSfnSfOffset += ueToEnbSfoffset;           // add offset UE to eNB offset from MIB update
                int64_t sfnSfOffsetDiff = llabs(oldSfnSfOffset-newSfnSfOffset); // calc absolute offset diff in respect to last iteration

                uint64_t curNs3Timing = nrFrames * maxNumTti + nrSubFrames;

                // hysteresis for timing alignment to prevent TTI toggling
                if (curNs3Timing == m_lastNs3Timing + 1)
                  {
                    NI_LOG_NONE("hysInc");
                    //subsequent misalignments, increment counter
                    m_misalignedCount++;
                  }
                else if (curNs3Timing > m_lastNs3Timing + 1)
                  {
                    NI_LOG_NONE("hysRst");
                    // new (no subsequent) misalignment, start from beginning
                    m_misalignedCount = 1;
                  }

                NI_LOG_DEBUG("m_misalignedCount: " << std::to_string(m_misalignedCount) <<
                             ", sfnSfOffsetDiff: " << std::to_string(sfnSfOffsetDiff) <<
                             ", curNs3Timing: " << std::to_string(curNs3Timing) <<
                             ", m_lastNs3Timing: " << std::to_string(m_lastNs3Timing));

                if ((m_misalignedCount > 1) || // more than one subsequent misaligments
                    (sfnSfOffsetDiff > 1))     // more than one TTI FPGA-NS-3 offset change
                  {
                    sfnSfOffset = newSfnSfOffset; // apply new offset
                    m_misalignedCount = 0;        // reset counter for misaligned timing
                    std::string errorMsg("New offset between NS-3 and NI FPGA PHY: " + std::to_string(newSfnSfOffset) + " (" + std::to_string(oldSfnSfOffset) + ")");
                    NI_LOG_ERROR(errorMsg);
                    NI_LOG_CONSOLE_DEBUG(errorMsg);
                  }
                m_lastNs3Timing = curNs3Timing;
              }
            else // timing comparison OK, decrement if timing was misaligned
              {
                if (m_misalignedCount > 0)
                  {
                    NI_LOG_NONE("hysDec");
                    // decrement counter because FPGA has the timing self compensated in the meantime.
                    // this is the case when LTE AFW reads one time no timing indication and
                    // in the next run two of them (caused by jitter in AFW)
                    m_misalignedCount--;
                  }
              }
          }
      }

    NI_LOG_INFO ("StartSubFrame NS3 SFN.TTI: " + std::to_string(nrFrames) + "." + std::to_string(nrSubFrames) +
                 " , Corrected SFN.TTI: " + std::to_string(correctNrFrames) + "." + std::to_string(correctNrSubFrames) +
                 " , PhyTimingInd SFN.TTI: "  + std::to_string(timingIndSfn) + "." + std::to_string(timingIndTti));

    g_timingSyncMutex.unlock();
  }

  int64_t
  NiLteSdrTimingSync::GetAlignmentOffset(void)
  {
    return g_alignmentOffset;
  }
  int64_t
  NiLteSdrTimingSync::GetGlobalTimingdiffNano(void)
  {
    return g_globalTimingdiffNano;
  }

} // namespace ns3
