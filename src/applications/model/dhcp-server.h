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

#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include <ns3/traced-value.h>
#include "dhcp-header.h"
#include <map>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup dhcpclientserver DhcpClientServer
 */

/**
 * \ingroup dhcpclientserver
 * \class DhcpServer
 * \brief A Dhcp server.
 */
class DhcpServer : public Application
{
public:
  static TypeId GetTypeId (void);
  DhcpServer ();
  virtual ~DhcpServer ();

protected:
  virtual void DoDispose (void);

private:
  static const int PORT = 67;                       //!< The port number

  /*
   * \brief function to run the main configuration process to be triggered when a packet is received
   * \param socket which is expected to receive the message
   */
  void RunEfsm (Ptr<Socket> socket);

  /*
   * \brief function to be triggered in case of timeout
   */
  void TimerHandler (void);
  /*
   * \brief function to start the dhcp server application in node
   */
  virtual void StartApplication (void);

  /*
   * \brief function to stop the dhcp server application in node
   */
  virtual void StopApplication (void);

  Ptr<Socket> m_socket;                  //!< The socket for the application
  Address m_local;                       //!< The local address
  Ipv4Address m_poolAddress;             //!< The network address available to the server
  Ipv4Address m_minAddress;              //!< The first address of the range
  Ipv4Address m_maxAddress;              //!< The last address of the range
  uint32_t m_occurange;                  //!< Number of occupied address in the server
  Ipv4Mask m_poolMask;                   //!< The network mask available to the server
  Ipv4Address m_server;                  //!< Address of the dhcp server
  Ipv4Address m_peer;                    //!< Address of the peer
  std::map<std::pair<Mac48Address, uint32_t>, uint32_t> m_leasedAddresses; //!< Leased address and their status (cache memory)
  uint32_t m_nextAddressSeq;             //!< The host identifier
  Time m_lease;                          //!< Defining the granted lease time
  Time m_renew;                          //!< Defining the renewal time for the client
  Time m_rebind;                         //!< Defining the rebinding time for the client
  EventId m_expiredEvent;                //!< The expired event identifier
};

} // namespace ns3

#endif /* DHCP_SERVER_H */
