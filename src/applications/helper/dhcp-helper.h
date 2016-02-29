/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB
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
 * Author: Radu Lupu <rlupu@elcom.pub.ro> for previous patch of DHCP on ns-3.12
 *         Ankit Deepak <adadeepak8@gmail.com> and
 *         Deepti Rajagopal <deeptir96@gmail.com> for DHCP patch on ns-3.24
 *
 */
#ifndef DHCP_HELPER_H
#define DHCP_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

/**
* \brief create an dhcp service
*/
class DhcpClientHelper
{
public:
  /**
  * Create DhcpClientHelper
  *
  * \param port The port number of the dhcp server
  */
  DhcpClientHelper (uint32_t device);
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

class DhcpServerHelper
{
public:
  /**
  * Create DhcpServerHelper
  *
  * \param pool_addr The domain of the IP addr that will be assigned dynamically
  * \param pool_mask The mask of the domain
  * \param option_1 Here of type IPv4 address (e.g. IP addr of some network server)
  * \param port The port number of the dhcp server
  *
  */
  DhcpServerHelper (Ipv4Address pool_addr, Ipv4Mask pool_mask, Ipv4Address serv_addr, Ipv4Address min_addr, Ipv4Address max_addr);
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* DHCP_HELPER_H */
