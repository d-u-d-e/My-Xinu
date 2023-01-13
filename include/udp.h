#pragma once

#include <kernel.h>
#include <net.h>

#define	UDP_SLOTS	6 		/* Number of open UDP endpoints */
#define	UDP_QSIZ	8		/* Packets enqueued per endpoint*/

void udp_in(struct netpacket * pkptr);
void udp_hton(struct netpacket * pktptr);
void udp_ntoh(struct netpacket * pktptr);