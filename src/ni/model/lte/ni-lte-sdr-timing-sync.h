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

#ifndef NI_LTE_SDR_TIMING_SYNC_H_
#define NI_LTE_SDR_TIMING_SYNC_H_

#include <iostream>
#include <ns3/object.h>

namespace ns3 {

  class NiLteSdrTimingSync : public Object
  {

  public:
    NiLteSdrTimingSync();
    virtual ~NiLteSdrTimingSync();

    // interface to wall clock synchronizer
    static void AttachWallClock (void);

    // interface to lte-phy-interface class
    void CalcPhyTimeDiff(int64_t currentDiff);
    void ReCalcSfnSfOffset(bool firstRun, uint32_t nrFrames, uint32_t nrSubFrames,
                           uint16_t timingIndSfn, uint8_t timingIndTti,
                           uint64_t ueToEnbSfoffset, int64_t &sfnSfOffset);
    int64_t GetAlignmentOffset(void);
    int64_t GetGlobalTimingdiffNano(void);

  private:
    // interface to wall clock synchronizer
    static uint64_t CalcNormalizedRealtime (uint64_t realtimeNano, uint64_t realtimeOriginNano);

    static int64_t AlignDiff(uint64_t wallClockDelta);

    const uint64_t m_phyTimeUsAccuSize = 8;

  };

} // namespace ns3


#endif /* NI_LTE_HELPER_H_ */
