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
 * Author: Daniel Swist
 *         Vincent Kotzsch <vincent.kotzsch@ni.com>
 *         Clemens Felber <clemens.felber@ni.com>
 */

#ifndef PARAMETER_DATA_BASE_H
#define PARAMETER_DATA_BASE_H

#include <string>

class ParameterDataBase
{
private:
    int parameterInt1;
    std::string parameterString1;
    int parameterNum_PhyTimingInd;
    bool parameterLog_PhyTimingInd;
    uint32_t lwaDecisionVariable;
    uint32_t lwipDecisionVariable;
    uint32_t dcDecisionVariable;
    bool dcLaunchEnable;
    bool manualLteUeChannelSinrEnable;
    double lteUeChannelSinr;
    uint32_t gfdmDlScs;
    uint32_t gfdmUlScs;


public:
  ParameterDataBase ();
  ~ParameterDataBase ();

	void setParameterInt1(int p1);
	void setParameterInt1(std::string p1str);
	int getParameterInt1();
	std::string getStringParamterInt1();

	void setParameterString1(std::string str1);
	std::string getParameterString1();

	void setParameterNumPhyTimingInd(int pti);
	int getParameterNumPhyTimingInd();
	std::string getStringParameterNumPhyTimingInd();

	void setParameterLogPhyTimingInd(std::string lptiStr);
	bool getParameterLogPhyTimingInd();
	std::string getStringParameterLogPhyTimingInd();

	std::string getParameterByName(std::string pname);
	std::string setParameterByName(std::string pname, std::string pvalue);

	void setParameterLwaDecVariable(uint32_t p1);
	std::string setParameterLwaDecVariable(std::string p1str);
	uint32_t getParameterLwaDecVariable();
	std::string getStringParameterLwaDecVariable();

	void setParameterDcDecVariable(uint32_t p1);
	std::string setParameterDcDecVariable(std::string p1str);
	uint32_t getParameterDcDecVariable();
	std::string getStringParameterDcDecVariable();

	void setParameterDcLaunchEnable(bool enable);
	std::string setParameterDcLaunchEnable(std::string enableStr);
	bool getParameterDcLaunchEnable(void);
	std::string getStringParameterDcLaunchEnable(void);

	void setParameterLwipDecVariable(uint32_t p2);
	std::string setParameterLwipDecVariable(std::string p2str);
	uint32_t getParameterLwipDecVariable();
	std::string getStringParameterLwipDecVariable();

  void setParameterManualLteUeChannelSinrEnable(bool enable);
  std::string setParameterManualLteUeChannelSinrEnable(std::string enableStr);
  bool getParameterManualLteUeChannelSinrEnable(void);
  std::string getStringParameterManualLteUeChannelSinrEnable(void);

	void setParameterLteUeChannelSinr(double sinr);
	std::string setParameterLteUeChannelSinr(std::string sinrStr);
	double getParameterLteUeChannelSinr();
	std::string getStringParameterLteUeChannelSinr();

	void setParameterGfdmDlScs(uint32_t dlScs);
	std::string setParameterGfdmDlScs(std::string dlScs);
	uint32_t getParameterGfdmDlScs(void);
	std::string getStringParameterGfdmDlScs(void);

	void setParameterGfdmUlScs(uint32_t ulScs);
	std::string setParameterGfdmUlScs(std::string ulScs);
	uint32_t getParameterGfdmUlScs(void);
	std::string getStringParameterGfdmUlScs(void);
};

#endif
