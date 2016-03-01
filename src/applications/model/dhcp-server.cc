/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2011 UPB
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

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "dhcp-server.h"
#include "dhcp-header.h"
#include <ns3/ipv4.h>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DhcpServer");
NS_OBJECT_ENSURE_REGISTERED (DhcpServer);

TypeId
DhcpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DhcpServer")
    .SetParent<Application> ()
    .AddConstructor<DhcpServer> ()
    .AddAttribute ("LeaseTime",
                   "Lease for which address will be leased.",
                   TimeValue (Seconds (30)),
                   MakeTimeAccessor (&DhcpServer::m_lease),
                   MakeTimeChecker ())
    .AddAttribute ("RenewTime",
                   "Time after which client should renew.",
                   TimeValue (Seconds (15)),
                   MakeTimeAccessor (&DhcpServer::m_renew),
                   MakeTimeChecker ())
    .AddAttribute ("RebindTime",
                   "Time after which client should rebind.",
                   TimeValue (Seconds (25)),
                   MakeTimeAccessor (&DhcpServer::m_rebind),
                   MakeTimeChecker ())
    .AddAttribute ("PoolAddresses",
                   "Pool of addresses to provide on request.",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&DhcpServer::m_poolAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("FirstAddress",
                   "The First valid address that can be given.",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&DhcpServer::m_minAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("LastAddress",
                   "The Last valid address that can be given.",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&DhcpServer::m_maxAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("PoolMask",
                   "Mask of the pool of addresses.",
                   Ipv4MaskValue (),
                   MakeIpv4MaskAccessor (&DhcpServer::m_poolMask),
                   MakeIpv4MaskChecker ())
    .AddAttribute ("DHCPServer",
                   "DHCP Server Address",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&DhcpServer::m_server),
                   MakeIpv4AddressChecker ())
  ;
  return tid;
}

DhcpServer::DhcpServer ()
{
  NS_LOG_FUNCTION (this);
  m_nextAddressSeq = 1;
}

DhcpServer::~DhcpServer ()
{
  NS_LOG_FUNCTION (this);
}

void
DhcpServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void DhcpServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_minAddress < m_maxAddress,"Invalid Address range");
  m_occurange = 0;
  if (m_socket == 0)
    {
      uint32_t addrIndex;
      int32_t ifIndex;

      //add the DHCP local address to the leased addresses list, if it is defined!
      Ptr<Ipv4> ipv4 = GetNode ()->GetObject<Ipv4> ();
      ifIndex = ipv4->GetInterfaceForPrefix (m_poolAddress, m_poolMask);
      if (ifIndex >= 0)
        {
          for (addrIndex = 0; addrIndex < ipv4->GetNAddresses (ifIndex); addrIndex++)
            {
              if ((ipv4->GetAddress (ifIndex, addrIndex).GetLocal ().Get () & m_poolMask.Get ()) == m_poolAddress.Get () && ipv4->GetAddress (ifIndex, addrIndex).GetLocal ().Get () >= m_minAddress.Get () && ipv4->GetAddress (ifIndex, addrIndex).GetLocal ().Get () <= m_maxAddress.Get ())
                {
                  uint32_t id = ipv4->GetAddress (ifIndex, addrIndex).GetLocal ().Get () - m_minAddress.Get ();
                  m_occurange++;
                  m_leasedAddresses.insert (std::make_pair (std::make_pair (Mac48Address::ConvertFrom (GetNode ()->GetDevice (ifIndex)->GetAddress ()), id), 0xffffffff)); // set infinite GRANTED_LEASED_TIME for my address
                  break;
                }
            }
        }

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), PORT);
      m_socket->SetAllowBroadcast (true);
      m_socket->Bind (local);
    }

  m_socket->SetRecvCallback (MakeCallback (&DhcpServer::NetHandler, this));
  m_expiredEvent = Simulator::Schedule (Seconds (1), &DhcpServer::TimerHandler, this);
}

void DhcpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }

  m_leasedAddresses.clear ();
  Simulator::Remove (m_expiredEvent);
}

void DhcpServer::TimerHandler ()
{
  // Set up timeout events and release of unsolicited addresses from the list
  uint32_t a = 1;
  std::map<std::pair<Mac48Address, uint32_t>, uint32_t>::iterator i;
  for (i = m_leasedAddresses.begin (); i != m_leasedAddresses.end (); i++, a++)
    {
      // update the address state
      if (i->second == 1)
        {
          i->second = 0;
          m_occurange--;
          NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Address removed!" << a);
          NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Address leased state expired!");
          continue;
        }
      if (i->second != 0xffffffff || i->second != 0)
        {
          i->second--;
        }
    }
  m_expiredEvent = Simulator::Schedule (Seconds (1), &DhcpServer::TimerHandler, this);
}

void DhcpServer::NetHandler (Ptr<Socket> socket)
{
  DhcpHeader header;
  Ptr<Packet> packet = 0;
  Address from;
  packet = m_socket->RecvFrom (from);
  if (packet->RemoveHeader (header) == 0)
    {
      return;
    }
  if (header.GetType () == DhcpHeader::DHCPDISCOVER)
    {
      SendOffer (header,from);
    }
  if (header.GetType () == DhcpHeader::DHCPREQ && (header.GetReq ()).Get () >= m_minAddress.Get () && (header.GetReq ()).Get () <= m_maxAddress.Get ())
    {
      SendAck (header,from);
    }
}

void DhcpServer::SendOffer (DhcpHeader header, Address from)
{
  DhcpHeader new_header;
  Ipv4Address address;
  Mac48Address source;
  uint32_t tran;
  uint32_t addr;
  Ptr<Packet> packet = 0;
  source = header.GetChaddr48 ();
  NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Trace RX: DHCP DISCOVER from: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " source port: " <<  InetSocketAddress::ConvertFrom (from).GetPort ());
  tran = header.GetTran ();
  bool found = false;
  uint32_t found_addr;
  std::map<std::pair<Mac48Address, uint32_t>, uint32_t>::iterator i;
  for (i = m_leasedAddresses.begin (); i != m_leasedAddresses.end (); i++)
    {
      if (i->first.first == source)
        {
          found_addr = i->first.second;
          found = true;
          m_leasedAddresses.erase (i--);
          break;
        }
    }
  if (!found)
    {
      if ((header.GetReq ()).Get () >= m_minAddress.Get () && (header.GetReq ()).Get () <= m_maxAddress.Get ())
        {
          for (i = m_leasedAddresses.begin (); i != m_leasedAddresses.end (); i++)
            {
              if (i->first.second == (header.GetReq ()).Get () - m_minAddress.Get ())
                {
                  break;
                }
            }
          if (i == m_leasedAddresses.end ())
            {
              found_addr = (header.GetReq ()).Get () - m_minAddress.Get ();
              found = true;
            }
        }
    }
  if (found == false)
    {
      // find a new address to be leased
      if (m_occurange <= m_maxAddress.Get () - m_minAddress.Get ())
        {
          for (i = m_leasedAddresses.begin (); i != m_leasedAddresses.end (); i++)
            {
              // check whether the address is occupied
              if (i->first.second == m_nextAddressSeq && i->second != 0)
                {
                  m_nextAddressSeq = m_nextAddressSeq % (m_maxAddress.Get () - m_minAddress.Get ()) + 1;
                  i = m_leasedAddresses.begin ();
                }
              if (i->first.second == m_nextAddressSeq && i->second == 0)
                {
                  m_leasedAddresses.erase (i--);
                }
            }
          if (i == m_leasedAddresses.end ())
            {
              found_addr = m_nextAddressSeq;
              m_nextAddressSeq = m_nextAddressSeq % (m_maxAddress.Get () - m_minAddress.Get ()) + 1;
              found = true;
            }
        }
    }
  if (found)
    {
      addr = m_minAddress.Get () + found_addr;
      m_occurange++;
      m_leasedAddresses.insert (std::make_pair (std::make_pair (source, found_addr), m_lease.GetSeconds ()));
      address.Set (addr);

      packet = Create<Packet> ();
      new_header.ResetOpt ();
      new_header.SetType (DhcpHeader::DHCPOFFER);
      new_header.SetChaddr48 (source);
      new_header.SetYiaddr (address);
      new_header.SetDhcps (m_server);
      new_header.SetMask (m_poolMask.Get ());
      new_header.SetTran (tran);
      new_header.SetLease (m_lease.GetSeconds ());
      new_header.SetRenew (m_renew.GetSeconds ());
      new_header.SetRebind (m_rebind.GetSeconds ());
      new_header.SetTime ();
      packet->AddHeader (new_header);

      if ((m_socket->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), InetSocketAddress::ConvertFrom (from).GetPort ()))) >= 0)
        {
          NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Trace TX: DHCP OFFER");
        }
      else
        {
          NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Error while sending DHCP OFFER");
        }
    }
}

void DhcpServer::SendAck (DhcpHeader header, Address from)
{
  DhcpHeader new_header;
  Mac48Address source;
  uint32_t tran;
  uint32_t addr;
  Ptr<Packet> packet = 0;
  Ipv4Address address;
  address = header.GetReq ();
  addr = address.Get ();

  NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Trace RX: DHCP REQUEST from: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
               " source port: " <<  InetSocketAddress::ConvertFrom (from).GetPort () << " refreshed addr is =" << address);

  source = header.GetChaddr48 ();
  tran = header.GetTran ();
  std::map<std::pair<Mac48Address, uint32_t>, uint32_t>::iterator i;
  for (i = m_leasedAddresses.begin (); i != m_leasedAddresses.end (); i++)
    {
      // update the lease time of this address
      if ((m_minAddress.Get () + i->first.second) == addr)
        {
          (i->second) += m_lease.GetSeconds ();
          packet = Create<Packet> ();
          new_header.ResetOpt ();
          new_header.SetType (DhcpHeader::DHCPACK);
          new_header.SetChaddr48 (source);
          new_header.SetYiaddr (address);
          new_header.SetTran (tran);
          new_header.SetTime ();
          packet->AddHeader (new_header);
          m_peer = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
          if (m_peer != address)
            {
              m_socket->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), InetSocketAddress::ConvertFrom (from).GetPort ()));
            }
          else
            {
              m_socket->SendTo (packet, 0, InetSocketAddress (m_peer, InetSocketAddress::ConvertFrom (from).GetPort ()));
            }
          break;
        }
    }
  if (i == m_leasedAddresses.end ())
    {
      packet = Create<Packet> ();
      new_header.ResetOpt ();
      new_header.SetType (DhcpHeader::DHCPNACK);
      new_header.SetChaddr48 (source);
      new_header.SetYiaddr (address);
      new_header.SetTran (tran);
      new_header.SetTime ();
      packet->AddHeader (new_header);
      m_peer = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
      if (m_peer != address)
        {
          m_socket->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), InetSocketAddress::ConvertFrom (from).GetPort ()));
        }
      else
        {
          m_socket->SendTo (packet, 0, InetSocketAddress (m_peer, InetSocketAddress::ConvertFrom (from).GetPort ()));
        }
      NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "This IP addr does not exists or released!");
    }
}

} // Namespace ns3
