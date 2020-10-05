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

#ifndef SRC_NI_MODEL_LTE_NI_LTE_CONSTANTS_H_
#define SRC_NI_MODEL_LTE_NI_LTE_CONSTANTS_H_


#define NI_LTE_CONST_TTI_DURATION_US 1000
#define NI_LTE_CONST_MAX_NUM_TTI 10
#define NI_LTE_CONST_MAX_NUM_SFN 1024

#define NI_LTE_CONST_MAX_NS3_CTRL_MSG_SIZE 150  // maximum size of NS-3 control messages which are put in front of the MAC PDU
#define NI_LTE_CONST_MAX_MAC_PDU_SIZE      9422 // bytes @ MCS=28, PRB allocation b1111111111111111111111111
#define NI_LTE_CONST_MAX_NS3_MAC_PDU_SIZE  (NI_LTE_CONST_MAX_MAC_PDU_SIZE - NI_LTE_CONST_MAX_NS3_CTRL_MSG_SIZE)

#define NI_LTE_CONST_DEFAULT_DCI_PRB  255 // 6 RBGs --> For GFDM change to 255 = 8PRBs
#define NI_LTE_CONST_DEFAULT_DCI_RNTI 1
#define NI_LTE_CONST_DEFAULT_DCI_MCS  5

#endif /* SRC_NI_MODEL_LTE_NI_LTE_CONSTANTS_H_ */
