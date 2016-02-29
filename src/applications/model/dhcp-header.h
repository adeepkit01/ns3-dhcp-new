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

#ifndef DHCP_HEADER_H
#define DHCP_HEADER_H

#include "ns3/header.h"
#include <ns3/mac48-address.h>
#include <ns3/mac64-address.h>

namespace ns3 {
/**
 * \ingroup dhcp
 * \class DhcpHeader
 * \brief Bootp header with dhcp messages supports option-Subnet Mask(1), Address Request(50), Refresh Lease Time(51), DHCP Message Type(53), DHCP Server ID(54), Renew Time(58), Rebind Time(59) and End(255) of bootp

  \verbatim
    0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
   +---------------+---------------+---------------+---------------+
   |                            xid (4)                            |
   +-------------------------------+-------------------------------+
   |           secs (2)            |           flags (2)           |
   +-------------------------------+-------------------------------+
   |                          ciaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          yiaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          siaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          giaddr  (4)                          |
   +---------------------------------------------------------------+
   |                                                               |
   |                          chaddr  (16)                         |
   |                                                               |
   |                                                               |
   +---------------------------------------------------------------+
   |                                                               |
   |                          sname   (64)                         |
   +---------------------------------------------------------------+
   |                                                               |
   |                          file    (128)                        |
   +---------------------------------------------------------------+
   |                                                               |
   |                          options (variable)                   |
   +---------------------------------------------------------------+
  \endverbatim

 */
class DhcpHeader : public Header
{
public:
  DhcpHeader ();

  enum options
  {
    OP_MASK = 1,        //!< BOOTP Option 1: Address Mask
    OP_ADDREQ = 50,     //!< BOOTP Option 50: Requested Address
    OP_LEASE = 51,      //!< BOOTP Option 51: Address Lease Time
    OP_MSGTYPE = 53,    //!< BOOTP Option 53: DHCP Message Type
    OP_SERVID = 54,     //!< BOOTP Option 54: Server Identifier
    OP_RENEW = 58,      //!< BOOTP Option 58: Address Renewal Time
    OP_REBIND = 59,     //!< BOOTP Option 59: Address Rebind Time
    OP_END = 255        //!< BOOTP Option 255: END
  };

  enum Messages
  {
    DHCPDISCOVER = 0,     //!< Code for DHCP Discover
    DHCPOFFER = 1,        //!< Code for DHCP Offer 
    DHCPREQ = 2,          //!< Code for DHCP Request 
    DHCPACK = 4,          //!< Code for DHCP ACK
    DHCPNACK = 5          //!< Code for DHCP NACK 
  };

  /**
   * \brief Set the type of BOOTP and DHCP messages
   * \param type The type of message
   */
  void SetType (uint8_t type);

  /**
   * \brief Return the type of DHCP message
   * \return The sequence number
   */
  uint8_t GetType (void) const;

  /**
   * \brief Set the hardware information
   * \param htype Hardware type 
   * \param hlen Hardware length
   */
  void SetHWType (uint8_t htype, uint8_t hlen);

  /**
   * \brief Set the transaction ID
   * \param tran The transaction number
   */
  void SetTran (uint32_t tran);

  /**
   * \brief Set the time when message is sent
   */
  void SetTime ();

  /**
   * \brief Get the transaction id
   * \return The transaction id
   */
  uint32_t GetTran (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Set the MAC address of the device
   * \param addr Mac48Address of the device
   */
  void SetChaddr48 (Mac48Address addr);

  /**
   * \brief Set the MAC address of the device
   * \param addr Mac64Address of the device
   */
  void SetChaddr64 (Mac64Address addr);

  /**
   * \brief Get the MAC address of the client
   * \return The MAC 48 address of the client
   */
  Mac48Address GetChaddr48 (void);

  /**
   * \brief Get the MAC address of the client
   * \return The MAC 64 address of the client
   */
  Mac64Address GetChaddr64 (void);

  /**
   * \brief Set the IP Address of the client
   * \param addr the client Ipv4Address
   */
  void SetYiaddr (Ipv4Address addr);

  /**
   * \brief Get the IP Address of the client
   * \return IP address of the client
   */
  Ipv4Address GetYiaddr (void) const;

  /**
   * \brief Set the dhcp server information
   * \param addr IP address of the server
   */
  void SetDhcps (Ipv4Address addr);

  /**
   * \brief set the requested address by the client
   * \param the Ipv4Address client is requesting for
   */
  void SetReq (Ipv4Address addr);

  /**
   * \brief Set the mask of the IPv4 address
   * \param addr 32 bit mask
   */
  void SetMask (uint32_t addr);

  /**
   * \brief Set the lease time of the address
   * \param time 32 bit time
   */
  void SetLease (uint32_t time);

  /**
   * \brief Set the Renewal time of the address
   * \param time 32 bit time
   */
  void SetRenew (uint32_t time);

  /**
   * \brief Set the Rebind time of the address
   * \param time 32 bit time
   */
  void SetRebind (uint32_t time);

  /**
   * \brief Get the information about the dhcp server
   * \return the DHCP server address
   */
  Ipv4Address GetDhcps (void) const;

  /**
   * \brief Get the address requested by the client
   * \return The address requested by the client
   */
  Ipv4Address GetReq (void) const;

  /**
   * \brief Return the mask of the network
   * \return 32 bit mask
   */
  uint32_t GetMask (void) const;

  /**
   * \brief Return the lease time of the address
   * \return 32 bit time
   */
  uint32_t GetLease (void) const;

  /**
   * \brief Return the Renewal time of the address
   * \return 32 bit time
   */
  uint32_t GetRenew (void) const;

  /**
   * \brief Return the Rebind time of the address
   * \return 32 bit time
   */
  uint32_t GetRebind (void) const;

  /**
   * \brief Reset the BOOTP options
   */
  void ResetOpt ();

private:
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint8_t m_op;                          //!< The DHCP Message type
  uint8_t m_bootp;                       //!< The BOOTP Message type
  uint8_t m_hType;                       //!< The hardware type
  uint8_t m_hLen;                        //!< The hardware length
  uint8_t m_hops;                        //!< The number of hops covered by the message
  uint32_t m_xid;                        //!< The transaction number
  uint32_t m_mask;                       //!< The mask of the network
  uint32_t m_len;                        //!< The length of the header
  uint16_t m_secs;                       //!< Seconds elapsed
  uint16_t m_flags;                      //!< BOOTP flags
  Mac48Address m_chAddr48;               //!< The MAC 48 Address of the client
  Mac64Address m_chAddr64;               //!< The MAC 64 Address of the client
  Ipv4Address m_yiAddr;                  //!< Your (client) IP address
  Ipv4Address m_ciAddr;                  //!< The IP address of the client
  Ipv4Address m_siAddr;                  //!< Next Server IP address
  Ipv4Address m_giAddr;                  //!< Relay Agent IP address
  Ipv4Address m_dhcps;                   //!< DHCP server IP address
  Ipv4Address m_req;                     //!< Requested Address
  uint8_t m_hwpad[10];                   //!< When entered address is 48 bit Mac Address, 10 bits padded
  uint8_t m_sname[64];                   //!< Server name (Padded for now)
  uint8_t m_file[128];                   //!< File name (Padded for now)
  uint8_t m_magic_cookie[4];             //!< DHCP Magic Cookie
  uint32_t m_lease;                      //!< The lease time of the address
  uint32_t m_renew;                     //!< The renewal time for the client
  uint32_t m_rebind;                     //!< The rebinding time for the client
  bool m_opt[255];                       //!< BOOTP option list
};

} // namespace ns3

#endif /* DHCP_HEADER_H */
