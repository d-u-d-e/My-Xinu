#pragma once
#include <kernel.h>
#include <ether.h>

struct network { /* Network information	*/
    uint32  ipucast; /* Computer's IP unicast address */
    uint32  ipbcase; /* IP broadcast address	*/
    uint32  ipmask;  /* IP address mask */
    uint32  ipprefix; /* IP (network) prefix */
    uint32  iprouter; /* Default router address */
    uint32  dnsserver;	/* DNS server address */
    uint32  ntpserver; /* NTP (time) server address */
    bool8   ipvalid; /* nonzero => above are valid	*/
    byte    ethucast[ETH_ADDR_LEN]; /* Ethernet multicast address */
    byte    ethbcast[ETH_ADDR_LEN]; /* Ethernet broadcast address */
};