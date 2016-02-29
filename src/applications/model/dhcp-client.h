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
 *
 */

#ifndef DHCP_CLIENT_H
#define DHCP_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"
#include "dhcp-header.h"
#include <list>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup dhcpclientserver
 * \class DhcpClient
 * \brief A Dhcp client. It learns DHCP server and IP gw addresses from IP header.
 * In other words DHCP server must be located on the network gw node.
 *
 */
class DhcpClient : public Application
{
public:
  static TypeId
  GetTypeId (void);

  DhcpClient ();

  virtual ~DhcpClient ();


  /**
   * \brief return the current dhcp server
   * \return Ipv4Address of the server
   */
  Ipv4Address GetDhcpServer (void);


  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  virtual void DoDispose (void);

private:
  enum States
  {
    WAIT_OFFER = 1,       //!< Defining state of the client waiting offer
    REFRESH_LEASE = 2,    //!< Defining state of the client needing lease refresh
    WAIT_ACK = 9         //!< Defining state of the client waiting acknowledgement
  };


  /*
   * \brief function to start the dhcp client application in node
   */
  virtual void StartApplication (void);

  /*
   * \brief function to stop the dhcp client application in node
   */
  virtual void StopApplication (void);

  /*
   * \brief function to handle the LinkState
   */
  void LinkStateHandler (void);

  /*
   * \brief function to handle the network
   */
  void NetHandler (Ptr<Socket> socket);

  void Boot (void);
  void OfferHandler (DhcpHeader header);
  void AcceptAck (DhcpHeader header, Address from);
  void Select (void);
  void Request (void);
  /*
   * \brief function triggered in case of retransmission
   *
  void RtrsHandler (void);

   *
   * \brief function to run the main configuration process
   *
  void RunEfsm (void);*/


  uint8_t m_state;                       //!< State of the client
  uint32_t device;                       //!< device identifier
  Ptr<Socket> m_socket;                  //!< socket for remote communication
  Ipv4Address m_remoteAddress;           //!< Address of the peer
  Ipv4Address m_myAddress;               //!< Address assigned to the client
  Ipv4Address m_offeredAddress;          //!< Address offered to the client
  Ipv4Mask m_myMask;                     //!< Mask of the address assigned
  Ipv4Address m_server;                  //!< Address of the DHCP server
  EventId m_requestEvent;                //!< Address refresh event
  EventId m_discoverEvent;               //!< Message retransmission event
  EventId m_refreshEvent;                //!< Message refresh event
  EventId m_rebindEvent;                 //!< Message rebind event
  EventId m_nextOfferEvent;              //!< Message next offer event
  EventId m_timeout;                     //!< The timeout time
  Time m_lease;                          //!< Store the lease time of the address
  Time m_renew;                          //!< Store the renew time of the address
  Time m_rebind;                         //!< Store the rebind time of the address
  Time m_nextoffer;                      //!< Time to try the next offer (if request gets no reply)
  Ptr<RandomVariableStream> m_ran;       //!< Uniform random variable for transaction ID
  Time m_rtrs;                           //!< Defining the time for retransmission
  Time m_collect;                        //!< Time for which client should collect offers
  bool m_offered;                        //!< Specify if the client has got any offer
  std::list<DhcpHeader> m_offerList;     //!< Stores all the offers given to the client
  uint32_t m_tran;                       //!< Stores the current transaction number to be used
};

} // namespace ns3

#endif /* DHCP_CLIENT_H */
