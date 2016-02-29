DHCP implementation in ns-3
---------------------------

Model Description
*****************

This section documents the implementation details of Dynamic Host Configuration Protocol (DHCP)

The source code for DHCP is located in ``src/applications/model`` and consists of the 
following 6 files:
 - dhcp-server.h,
 - dhcp-server.cc,
 - dhcp-client.h,
 - dhcp-client.cc,
 - dhcp-header.h and
 - dhcp-header.cc

Helpers
*******

The following two files have been added to ``src/applications/helper`` for DHCP: 
- dhcp-helper.h and 
- dhcp-helper.cc

Tests
*****
The tests for DHCP can be found at ``src/applications/test/dhcp-test.cc`

Examples
********
The examples for DHCP can be found at ``src/applications/examples/dhcp-example.cc`


Scope and Limitations
*********************

The current implementation of DHCP supports only one DHCP server in a network, 
which can auto configure any number of clients. The server should be provided 
with a network address and mask.

The following five basic DHCP messages are supported: 

- DHCP discover,
- DHCP offer,
- DHCP request,
- DHCP ack and
- DHCP nack

Also, the following five options of BootP are supported:
- 1 (Mask),
- 50 (Requested Address),
- 51 (Address Lease Time),
- 53 (DHCP message type),
- 54 (DHCP server) and
- 255 (end)

In the current implementation, a DHCP client can obtain IPv4 address dynamically 
from the DHCP server, which must be renewed after a lease time period.

