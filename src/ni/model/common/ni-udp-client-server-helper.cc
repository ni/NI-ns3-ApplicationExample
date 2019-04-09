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

#include "ni-udp-client-server-helper.h"

#include "ns3/ni-udp-server.h"
#include "ns3/ni-udp-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

NiUdpServerHelper::NiUdpServerHelper ()
{
}

NiUdpServerHelper::NiUdpServerHelper (uint16_t port)
{
  m_factory.SetTypeId (NiUdpServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
NiUdpServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
NiUdpServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<NiUdpServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<NiUdpServer>
NiUdpServerHelper::GetServer (void)
{
  return m_server;
}

NiUdpClientHelper::NiUdpClientHelper ()
{
}

NiUdpClientHelper::NiUdpClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (NiUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

NiUdpClientHelper::NiUdpClientHelper (Address address)
{
  m_factory.SetTypeId (NiUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
NiUdpClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
NiUdpClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NiUdpClient> client = m_factory.Create<NiUdpClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

} // namespace ns3
