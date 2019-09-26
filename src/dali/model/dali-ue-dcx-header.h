/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2019, Universitat Politecnica de Catalunya
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
 * Author: Daniel Maldonado-Hurtado <daniel.maldonado.hurtado@gmail.com>
 * Inspired by epc-x2-header
 */

#ifndef DALI_UE_DCX_HEADER_H
#define DALI_UE_DCX_HEADER_H

#include "ns3/dali-ue-dcx-sap.h"
#include "ns3/header.h"

#include <vector>


namespace ns3 {


class DaliUeDcxHeader : public Header
{
public:
  DaliUeDcxHeader ();
  virtual ~DaliUeDcxHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  /**
   * Get message type function
   * \returns the message type
   */
  uint8_t GetMessageType () const;
  /**
   * Set message type function
   * \param messageType the message type
   */
  void SetMessageType (uint8_t messageType);

  /**
   * Get procedure code function
   * \returns the procedure code
   */
  uint8_t GetProcedureCode () const;
  /**
   * Set procedure code function
   * \param procedureCode the procedure code
   */
  void SetProcedureCode (uint8_t procedureCode);

  /**
   * Set length of IEs function
   * \param lengthOfIes the length of IEs
   */
  void SetLengthOfIes (uint32_t lengthOfIes);
  /**
   * Set number of IEs function
   * \param numberOfIes the number of IEs
   */
  void SetNumberOfIes (uint32_t numberOfIes);


  /// Procedure code enumeration
  enum ProcedureCode_t {
	RrcDcConnectionReconfiguration            = 0,
	RrcDcConnectionReconfigurationCompleted   = 1,
    NotifyDcConnection                        = 2
  };

  /// Type of message enumeration
  enum TypeOfMessage_t {
    InitiatingMessage       = 0, //Standard TypeOfMessage left for possible future use
    SuccessfulOutcome       = 1,
    UnsuccessfulOutcome     = 2,
    DcForwardDownlinkData   = 3,
    DcForwardUplinkData     = 4
  };

private:
  uint8_t m_messageType; ///< message type
  uint8_t m_procedureCode; ///< procedure code

  uint32_t m_lengthOfIes; ///< length of IEs
  uint32_t m_numberOfIes; ///< number of IEs
};


} // namespace ns3

#endif // DALI_UE_DCX_HEADER_H
