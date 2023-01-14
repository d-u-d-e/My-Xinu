#pragma once
#include <kernel.h>

struct netpacket;

#define	IP_BCAST	0xFFFFFFFF	/* IP local broadcast address */
#define	IP_ALLZEROS	0x00000000	/* The all-zeros IP address     */

#define	IP_ASIZE	4		/* Bytes in an IP address	*/

#define	IP_OQSIZ	8		/* Size of IP output queue	*/

#define	IP_UDP		17		/* UDP protocol type for IP 	*/
#define	IP_ICMP		1		/* ICMP protocol type for IP 	*/

#define	IP_HDR_LEN	20		/* Bytes in an IP header without options */

/* Queue of outgoing IP packets waiting for ipout process */

struct iqentry{
	int32	iqhead;			/* Index of next packet to send	*/
	int32	iqtail;			/* Index of next free slot	*/
	sid32	iqsem;			/* Semaphore that counts pkts */
	struct	netpacket * iqbuf[IP_OQSIZ];/* Circular packet queue */
};

extern struct iqentry ipoqueue;	/* Network output queue	*/

void ip_in(struct netpacket *);
status ip_enqueue(struct netpacket * pktptr);
status ip_send(struct netpacket * pktptr);
status dot2ip(char * dotted, uint32 * result);