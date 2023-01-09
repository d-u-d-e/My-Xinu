#pragma once
#include <kernel.h>

struct netpacket;

#define	IP_BCAST	0xFFFFFFFF	/* IP local broadcast address */
#define	IP_ALLZEROS	0x00000000	/* The all-zeros IP address     */

#define	IP_ASIZE	4		/* Bytes in an IP address	*/

#define	IP_OQSIZ	8		/* Size of IP output queue	*/


/* Queue of outgoing IP packets waiting for ipout process */

struct iqentry{
	int32	iqhead;			/* Index of next packet to send	*/
	int32	iqtail;			/* Index of next free slot	*/
	sid32	iqsem;			/* Semaphore that counts pkts */
	struct	netpacket * iqbuf[IP_OQSIZ];/* Circular packet queue */
};

void ip_in(struct netpacket *);

extern struct iqentry ipoqueue;	/* Network output queue	*/