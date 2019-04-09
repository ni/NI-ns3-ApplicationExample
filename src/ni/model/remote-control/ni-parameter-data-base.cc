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

#include "ni-parameter-data-base.h"
#include "iostream"



//
// 1st demo parameter access methods (2 xactual type (int), 2x string)
//
void ParameterDataBase::setParameterInt1(int p1){
    parameterInt1 = p1;
    return;
}
void ParameterDataBase::setParameterInt1(std::string p1str){
    parameterInt1 = std::stoi(p1str);
    return;
}
int ParameterDataBase::getParameterInt1(){
    return parameterInt1;
}
std::string ParameterDataBase::getStringParamterInt1(){
    return std::to_string(parameterInt1);
}

//
//2nd demo paramter methods (2x string)
//
void ParameterDataBase::setParameterString1(std::string str1){
    parameterString1 = str1;
    return;
}
std::string ParameterDataBase::getParameterString1(){
    //todo check returning string, what exactly happens, copy, ref type thing?;
    return parameterString1;
}

//
//ns3 Example Parameter num_PhyTimingInd (transport.cc) (do not set from testman --> no set by string method needed)
//
void ParameterDataBase::setParameterNumPhyTimingInd(int pti){
    parameterNum_PhyTimingInd = pti;
    //std::cout << "TEST:SetParam: " << pti << "/" << parameterNum_PhyTimingInd << std::endl;
}
int ParameterDataBase::getParameterNumPhyTimingInd(){
    //std::cout << "TEST:GetParam: " << parameterNum_PhyTimingInd << std::endl;
    return parameterNum_PhyTimingInd;
}
std::string ParameterDataBase::getStringParameterNumPhyTimingInd(){
    //std::cout << "TEST:GetParam: " << parameterNum_PhyTimingInd << " / " << std::to_string(parameterNum_PhyTimingInd) << std::endl;
    return std::to_string(parameterNum_PhyTimingInd);
}

//
//ns3 Example Parameter for activating Loggin of phy timing indication
//
void ParameterDataBase::setParameterLogPhyTimingInd(std::string lptiStr){
    if (lptiStr == "true") {
        parameterLog_PhyTimingInd = true;
    } else {
        parameterLog_PhyTimingInd = false;
    }
}
bool ParameterDataBase::getParameterLogPhyTimingInd(){
    //std::cout << "TEST:GetParam: " << parameterLog_PhyTimingInd << std::endl;
    return parameterLog_PhyTimingInd;
}
std::string ParameterDataBase::getStringParameterLogPhyTimingInd(){
    if (parameterLog_PhyTimingInd) {
        return "true";
    } else {
        return "false";
    }
}

//ns3 example for implementing lwa/lwip decision varaibles --LWA
void ParameterDataBase::setParameterLwaDecVariable(uint32_t p1) {
  // 0, 1, 2
  // otherwise throw error message
  if (p1 <= 2)
    {
      lwaDecisionVariable = p1;
    }
  else
    {
      std::cout << "ParameterDataBase::setParameterLwaDecVariable: ERR:LWA value not in range" << std::endl;
    }
}

std::string ParameterDataBase::setParameterLwaDecVariable(std::string p1str) {
  // 0, 1, 2
  // otherwise throw error message
  std::string ret = p1str;
  if (p1str == "1") {
      lwaDecisionVariable = 1;
  } else if (p1str == "2") {
      lwaDecisionVariable = 2;
  } else if (p1str == "0") {
      lwaDecisionVariable = 0;
  } else {
      std::cout << "ParameterDataBase::setParameterLwaDecVariable: ERR:LWA value not in range" << std::endl;
      ret = "ERR:LWA value not in range";
  }
  return ret;
}

uint32_t ParameterDataBase::getParameterLwaDecVariable(){
 	return lwaDecisionVariable;
}

std::string ParameterDataBase::getStringParameterLwaDecVariable(){
  return std::to_string(lwaDecisionVariable);
}

//ns3 example for implementing lwa/lwip decision varaibles --LWIP
void ParameterDataBase::setParameterLwipDecVariable(uint32_t p2) {
  // 0, 1
  // otherwise throw error message
  if (p2 <= 1)
    {
      lwipDecisionVariable = p2;
    }
  else
    {
      std::cout << "ParameterDataBase::setParameterLwipDecVariable: ERR:LWIP value not in range" << std::endl;
    }
}

std::string ParameterDataBase::setParameterLwipDecVariable(std::string p2str) {
  // 0, 1
  // otherwise throw error message
  std::string ret = p2str;
  if (p2str == "1") {
      lwipDecisionVariable = 1;
  } else if (p2str == "0") {
      lwipDecisionVariable = 0;
  } else {
      ret= "ERR:LWIP value not in range";
  }
  return ret;
}

uint32_t ParameterDataBase::getParameterLwipDecVariable(){
	return lwipDecisionVariable;
}

std::string ParameterDataBase::getStringParameterLwipDecVariable(){
  return std::to_string(lwipDecisionVariable);
}


//ns3 enable manual UE Channel Sinr
void ParameterDataBase::setParameterManualLteUeChannelSinrEnable(bool enable) {
  manualLteUeChannelSinrEnable = enable;
}

std::string ParameterDataBase::setParameterManualLteUeChannelSinrEnable(std::string enableStr) {
  if (enableStr == "true") {
      manualLteUeChannelSinrEnable = true;
  } else {
      manualLteUeChannelSinrEnable = false;
  }
  return enableStr;
}

bool ParameterDataBase::getParameterManualLteUeChannelSinrEnable(void){
  return manualLteUeChannelSinrEnable;
}

std::string ParameterDataBase::getStringParameterManualLteUeChannelSinrEnable(void){
  if (manualLteUeChannelSinrEnable) {
      return "true";
  } else {
      return "false";
  }
}

//ns3 UE Channel Sinr
void ParameterDataBase::setParameterLteUeChannelSinr(double sinr) {
  // range check?
  lteUeChannelSinr = sinr;
}

std::string ParameterDataBase::setParameterLteUeChannelSinr(std::string sinrStr) {
  // range check?
  lteUeChannelSinr = std::stod(sinrStr);
  return sinrStr;
}

double ParameterDataBase::getParameterLteUeChannelSinr(){
  return lteUeChannelSinr;
}

std::string ParameterDataBase::getStringParameterLteUeChannelSinr(){
  return std::to_string(lteUeChannelSinr);
}

std::string ParameterDataBase::getParameterByName(std::string pname){

  std::string response = "";
  //std::cout << "TEMP CMD FEEDBACK: " << pname << std::endl;

  if (pname.empty()) {
      response = "ERR:EMPTY_VARIABLE_NAME";
  } else if (pname == "ParameterInt1") {
      response = getStringParamterInt1();
  } else if (pname == "ParameterString1") {
      response = getParameterString1();
  } else if (pname == "num_PhyTimingInd") {
      response = getStringParameterNumPhyTimingInd();
  } else if (pname == "log_PhyTimingInd") {
      response = getStringParameterLogPhyTimingInd();
  } else if (pname == "ParameterLwaDecVariable") {
      response = getStringParameterLwaDecVariable();
  } else if (pname == "ParameterLwipDecVariable") {
      response = getStringParameterLwipDecVariable();
  } else if (pname == "ParameterManualLteUeChannelSinrEnable") {
      response = getStringParameterManualLteUeChannelSinrEnable();
  } else if (pname == "ParameterLteUeChannelSinr") {
      response = getStringParameterLteUeChannelSinr();
  } else {
      response = "ERR:UNKOWN_PARAMETER";
  }
  return response;
}

std::string ParameterDataBase::setParameterByName(std::string pname, std::string pvalue) {
  //todo consider merging abfarge which param to once only
  //maybe: preprocessor makro: both makro definition lead to same function but with differen parameters (first compare param, then if read/write)
  std::string response = pvalue;

  if (pname.empty() || pvalue.empty() ) {
      response = "ERR:EMPTY_NAME_OR_VALUE";
  } else if (pname == "ParameterInt1") {
      setParameterInt1(pvalue); //todo for all setters need error checking!
  } else if (pname == "ParameterString1") {
      setParameterString1(pvalue);
  } else if (pname == "num_PhyTimingInd") {
      response = "ERR:ONLY_READ_ACCESIBLE";
  } else if (pname == "log_PhyTimingInd") {
      setParameterLogPhyTimingInd(pvalue);
  } else if (pname == "ParameterLwaDecVariable") {
      response = setParameterLwaDecVariable(pvalue);
  } else if (pname == "ParameterLwipDecVariable") {
      response = setParameterLwipDecVariable(pvalue);
  } else if (pname == "ParameterManualLteUeChannelSinrEnable") {
      response = setParameterManualLteUeChannelSinrEnable(pvalue);
  } else if (pname == "ParameterLteUeChannelSinr") {
      response = setParameterLteUeChannelSinr(pvalue);
  } else {
      response = "ERR_UNKOWN_PARAMETER"; //TODO-NI: change to ERR:UNKOWN_PARAMETER
  }

  return response;
}
