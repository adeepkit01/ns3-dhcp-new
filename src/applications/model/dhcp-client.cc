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
#include <stdlib.h>
#include <stdio.h>
#include <ns3/ipv4.h>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <ns3/ipv4-static-routing-helper.h>
#include "ns3/random-variable-stream.h"
#include "dhcp-client.h"
#include "dhcp-header.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include <list>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DhcpClient");
NS_OBJECT_ENSURE_REGISTERED (DhcpClient);

static const int DHCP_PEER_PORT = 67; //!< Defining DHCP port

TypeId
DhcpClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DhcpClient")
    .SetParent<Application> ()
    .AddConstructor<DhcpClient> ()
    .AddAttribute ("NetDevice", "Index of netdevice of the node for DHCP",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DhcpClient::device),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RTRS", "Time for retransmission of Discover message",
                   TimeValue (Seconds (5)),
                   MakeTimeAccessor (&DhcpClient::m_rtrs),
                   MakeTimeChecker ())
    .AddAttribute ("Collect", "Time for which offer collection starts",
                   TimeValue (Seconds (5)),
                   MakeTimeAccessor (&DhcpClient::m_collect),
                   MakeTimeChecker ())
    .AddAttribute ("ReRequest", "Time after which request will be resent to next server",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&DhcpClient::m_nextoffer),
                   MakeTimeChecker ())
    .AddAttribute ("Transactions",
                   "The possible value of transaction numbers ",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000000.0]"),
                   MakePointerAccessor (&DhcpClient::m_ran),
                   MakePointerChecker<RandomVariableStream> ());
  return tid;
}

DhcpClient::DhcpClient () : m_server (Ipv4Address::GetAny ())
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_refreshEvent = EventId ();
  m_requestEvent = EventId ();
  m_discoverEvent = EventId ();
  m_rebindEvent = EventId ();
  m_nextOfferEvent = EventId ();
  m_timeout = EventId ();
}

DhcpClient::~DhcpClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}


Ipv4Address DhcpClient::GetDhcpServer (void)
{
  return m_server;
}

void
DhcpClient::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

int64_t
DhcpClient::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_ran->SetStream (stream);
  return 1;
}

void
DhcpClient::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_remoteAddress = Ipv4Address ("255.255.255.255");
  m_myAddress = Ipv4Address ("0.0.0.0");
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 68);
      m_socket->SetAllowBroadcast (true);
      m_socket->Bind (local);
      m_socket->BindToNetDevice (GetNode ()->GetDevice (device));
    }
  m_socket->SetRecvCallback (MakeCallback (&DhcpClient::NetHandler, this));

  GetNode ()->GetDevice (device)->AddLinkChangeCallback (MakeCallback (&DhcpClient::LinkStateHandler, this));
  Boot ();

}

void
DhcpClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Remove (m_discoverEvent);
  Simulator::Remove (m_requestEvent);
  Simulator::Remove (m_rebindEvent);
  Simulator::Remove (m_refreshEvent);
  Simulator::Remove (m_timeout);
  Ptr<Ipv4> ipv4 = GetNode ()->GetObject<Ipv4> ();
  int32_t ifIndex = ipv4->GetInterfaceForAddress (m_myAddress);
  ipv4->RemoveAddress (ifIndex, 0);
  ipv4->AddAddress (ifIndex, Ipv4InterfaceAddress (Ipv4Address ("0.0.0.0"),Ipv4Mask ("/0")));
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->Close ();
}


void DhcpClient::LinkStateHandler (void)
{
  if (GetNode ()->GetDevice (device)->IsLinkUp ())
    {
      NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "LINK UP!!!! at " << Simulator::Now ().GetSeconds ());
      m_socket->SetRecvCallback (MakeCallback (&DhcpClient::NetHandler, this));
      Boot ();
    }
  else
    {
      NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "LINK DOWN!!!! at " << Simulator::Now ().GetSeconds ()); //reinitialization
      Simulator::Remove (m_refreshEvent); //stop refresh timer!!!!
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());  //stop receiving on this socket !!!

      Ptr<Ipv4> ipv4MN = GetNode ()->GetObject<Ipv4> ();
      int32_t ifIndex = ipv4MN->GetInterfaceForDevice (GetNode ()->GetDevice (device));
      ipv4MN->RemoveAddress (ifIndex, 0);
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (ipv4MN);
      staticRouting->RemoveRoute (0);
      ipv4MN->AddAddress (ifIndex, Ipv4InterfaceAddress (Ipv4Address ("0.0.0.0"), Ipv4Mask ("/0")));
    }
}

void DhcpClient::NetHandler (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Address from;
  Ptr<Packet> packet = m_socket->RecvFrom (from);
  DhcpHeader header;
  if (packet->RemoveHeader (header) == 0)
    {
      return;
    }
  if (header.GetChaddr48 () != Mac48Address::ConvertFrom (GetNode ()->GetDevice (device)->GetAddress ()))
    {
      return;
    }
  if (m_state == WAIT_OFFER && header.GetType () == DhcpHeader::DHCPOFFER)
    {
      OfferHandler (header);
    }
  if (m_state == WAIT_ACK && header.GetType () == DhcpHeader::DHCPACK)
    {
      Simulator::Remove (m_nextOfferEvent);
      AcceptAck (header,from);
    }
  if (m_state == WAIT_ACK && header.GetType () == DhcpHeader::DHCPNACK)
    {
      Simulator::Remove (m_nextOfferEvent);
      Boot ();
    }
}

void DhcpClient::AcceptAck (DhcpHeader header, Address from)
{
  Simulator::Remove (m_rebindEvent);
  Simulator::Remove (m_refreshEvent);
  Simulator::Remove (m_timeout);
  NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Trace TX: DHCP ACK RECEIVED");
  Ptr<Ipv4> ipv4 = GetNode ()->GetObject<Ipv4> ();
  int32_t ifIndex = ipv4->GetInterfaceForAddress (m_myAddress);
  ipv4->RemoveAddress (ifIndex, 0);

  ipv4->AddAddress (ifIndex, Ipv4InterfaceAddress ((Ipv4Address)m_offeredAddress, m_myMask));
  ipv4->SetUp (ifIndex);
  NS_LOG_INFO ("here");
  InetSocketAddress remote = InetSocketAddress (InetSocketAddress::ConvertFrom (from).GetIpv4 (), DHCP_PEER_PORT);
  m_socket->Connect (remote);
  m_myAddress = m_offeredAddress;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (ipv4);
  staticRouting->SetDefaultRoute (InetSocketAddress::ConvertFrom (from).GetIpv4 (), ifIndex, 0);

  m_remoteAddress = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
  NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Current DHCP Server is =" << m_remoteAddress);

  m_offerList.clear ();
  m_refreshEvent = Simulator::Schedule (m_renew, &DhcpClient::Request, this);
  m_rebindEvent = Simulator::Schedule (m_rebind, &DhcpClient::Request, this);
  m_timeout =  Simulator::Schedule (m_lease, &DhcpClient::Boot, this);
  m_state = REFRESH_LEASE;
}

void DhcpClient::OfferHandler (DhcpHeader header)
{
  m_offerList.push_back (header);
  if (m_offered == false)
    {
      Simulator::Remove (m_discoverEvent);
      m_offered = true;
      Simulator::Schedule (m_collect, &DhcpClient::Select, this);
    }
}

void DhcpClient::Select (void)
{
  if (m_offerList.empty ())
    {
      Boot ();
    }
  DhcpHeader header = m_offerList.front ();
  m_offerList.pop_front ();
  m_lease = Time (Seconds (header.GetLease ()));
  m_renew = Time (Seconds (header.GetRenew ()));
  m_rebind = Time (Seconds (header.GetRebind ()));
  m_offeredAddress = header.GetYiaddr ();
  m_myMask = Ipv4Mask (header.GetMask ());
  m_server = header.GetDhcps ();
  Request ();
}

void DhcpClient::Request (void)
{
  DhcpHeader header;
  Ptr<Packet> packet;
  if (m_state != REFRESH_LEASE)
    {
      packet = Create<Packet> ();
      header.ResetOpt ();
      header.SetType (DhcpHeader::DHCPREQ); //Send DHCP Request
      header.SetTime ();
      header.SetTran (m_tran);
      header.SetReq (m_offeredAddress);
      header.SetChaddr48 (Mac48Address::ConvertFrom (GetNode ()->GetDevice (device)->GetAddress ()));
      packet->AddHeader (header);
      m_socket->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), DHCP_PEER_PORT));
      m_state = WAIT_ACK;
      m_nextOfferEvent = Simulator::Schedule (m_nextoffer, &DhcpClient::Select, this);
    }
  else
    {
      uint32_t addr = m_myAddress.Get ();
      packet = Create<Packet> ((uint8_t*)&addr, sizeof(addr));
      header.ResetOpt ();
      m_tran = (uint32_t) (m_ran->GetValue ());
      header.SetTran (m_tran);
      header.SetTime ();
      header.SetType (DhcpHeader::DHCPREQ);
      header.SetReq (m_myAddress);
      m_offeredAddress = m_myAddress;
      header.SetChaddr48 (Mac48Address::ConvertFrom (GetNode ()->GetDevice (device)->GetAddress ()));
      packet->AddHeader (header);
      if ((m_socket->SendTo (packet, 0, InetSocketAddress (m_remoteAddress, DHCP_PEER_PORT))) >= 0)
        {
          NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Trace TX: DHCP REQUEST");
        }
      else
        {
          NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Error while sending DHCP REQ to " << m_remoteAddress);
        }
      m_state = WAIT_ACK;
    }
}

void DhcpClient::Boot (void)
{
  DhcpHeader header;
  Ptr<Packet> packet;
  packet = Create<Packet> ();
  header.ResetOpt ();
  m_tran = (uint32_t) (m_ran->GetValue ());
  header.SetTran (m_tran);
  header.SetType (DhcpHeader::DHCPDISCOVER);
  header.SetTime ();
  header.SetChaddr48 (Mac48Address::ConvertFrom (GetNode ()->GetDevice (device)->GetAddress ()));
  packet->AddHeader (header);

  if ((m_socket->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), DHCP_PEER_PORT))) >= 0)
    {
      NS_LOG_INFO ("[node " << GetNode ()->GetId () << "] Trace TX: DHCP DISCOVER" );
    }
  else
    {
      NS_LOG_INFO ("[node " << GetNode ()->GetId () << "]  " << "Error while sending DHCP DISCOVER to " << m_remoteAddress);
    }
  m_state = WAIT_OFFER;
  m_offered = false;
  m_discoverEvent = Simulator::Schedule (m_rtrs, &DhcpClient::Boot, this);
}

} // Namespace ns3
