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
 * \ingroup applications
 * \class DhcpHeader
 * \brief Bootp header with dhcp messages supports option-Subnet Mask(1), Address Request(50), Refresh Lease Time(51), DHCP Message Type(53), DHCP Server ID(54) and End(255) of bootp

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
    OP_MASK = 1,
    OP_ADDREQ = 50,
    OP_LEASE = 51,
    OP_MSGTYPE = 53,
    OP_SERVID = 54,
    OP_RENEW = 58,
    OP_REBIND = 59,
    OP_END = 255
  };

  enum Messages
  {
    DHCPDISCOVER = 0,     //!< Defining code for discover message
    DHCPOFFER = 1,        //!< Defining code for offer message
    DHCPREQ = 2,          //!< Defining code for request message
    DHCPACK = 4,          //!< Defining code for ack message
    DHCPNACK = 5          //!< Defining code for nack message
  };

  /**
   * \brief Set the type of bootp and dhcp messages
   * \param the type of message
   */
  void SetType (uint8_t type);

  /**
   * \brief return the type of dhcp message
   * \return the sequence number
   */
  uint8_t GetType (void) const;

  /**
   * \brief set the hardware information
   * \param hardware type and hardware length
   */
  void SetHWType (uint8_t htype, uint8_t hlen);

  /**
   * \brief set the transaction ID
   * \param the transaction number
   */
  void SetTran (uint32_t tran);

  /**
   * \brief set the time
   */
  void SetTime ();

  /**
   * \brief get the transaction id
   * \return the transaction id
   */
  uint32_t GetTran (void) const;

  /**
   * \brief TypeId function
   * \return the TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief set the MAC address of the device
   * \param Mac48Address of the client
   */
  void SetChaddr48 (Mac48Address addr);

  /**
   * \brief set the MAC address of the device
   * \param Mac64Address of the client
   */
  void SetChaddr64 (Mac64Address addr);

  /**
   * \brief get the MAC address of the client
   * \return the MAC 48 address of the client
   */
  Mac48Address GetChaddr48 (void);

  /**
   * \brief get the MAC address of the client
   * \return the MAC 64 address of the client
   */
  Mac64Address GetChaddr64 (void);

  /**
   * \brief set the IP Address of the client
   * \param the client Ipv4Address
   */
  void SetYiaddr (Ipv4Address addr);

  /**
   * \brief get the IP Address of the client
   * \return IP address of the client
   */
  Ipv4Address GetYiaddr (void) const;

  /**
   * \brief set the dhcp server information
   * \param IP address of the server
   */
  void SetDhcps (Ipv4Address addr);

  /**
   * \brief set the requested address by the client
   * \param the Ipv4Address client is requesting for
   */
  void SetReq (Ipv4Address addr);

  /**
   * \brief set the mask of the address
   * \param 32 bit mask
   */
  void SetMask (uint32_t addr);

  /**
   * \brief set the lease time of the address
   * \param 32 bit time
   */
  void SetLease (uint32_t time);

  /**
   * \brief set the Renewal time of the address
   * \param 32 bit time
   */
  void SetRenew (uint32_t time);

  /**
   * \brief set the Rebind time of the address
   * \param 32 bit time
   */
  void SetRebind (uint32_t time);

  /**
   * \brief get the information about the dhcp server
   * \return the dhcp server address
   */
  Ipv4Address GetDhcps (void) const;

  /**
   * \brief get the address requested by the client
   * \return the address requested by the client
   */
  Ipv4Address GetReq (void) const;

  /**
   * \brief return the mask of the network
   * \return 32 bit mask
   */
  uint32_t GetMask (void) const;

  /**
   * \brief return the lease time of the address
   * \return 32 bit time
   */
  uint32_t GetLease (void) const;

  /**
   * \brief return the Renewal time of the address
   * \return 32 bit time
   */
  uint32_t GetRenew (void) const;

  /**
   * \brief return the Rebind time of the address
   * \return 32 bit time
   */
  uint32_t GetRebind (void) const;

  /**
   * \brief reset the options of the bootp message
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
