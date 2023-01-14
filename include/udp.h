#pragma once

#include <kernel.h>
#include <net.h>

#define	UDP_SLOTS	6 		/* Number of open UDP endpoints */
#define	UDP_QSIZ	8		/* Packets enqueued per endpoint*/

/* Constants for the state of an entry */

#define	UDP_FREE	0 		/* Entry is unused		*/
#define	UDP_USED	1		/* Entry is being used		*/
#define	UDP_RECV	2		/* Entry has a process waiting	*/

#define UDP_HDR_LEN	8		/* Bytes in a UDP header	*/

struct	udpentry {			/* Entry in the UDP endpoint tbl*/
	int32	udstate;		/* State of entry: free/used	*/
	uint32	udremip;		/* Remote IP address (zero means don't care) */
	uint16	udremport;		/* Remote protocol port number	*/
	uint16	udlocport;		/* Local protocol port number	*/
	int32	udhead;			/* Index of next packet to read	*/
	int32	udtail;			/* Index of next slot to insert	*/
	int32	udcount;		/* Count of packets enqueued	*/
	pid32	udpid;			/* ID of waiting process	*/
	struct	netpacket * udqueue[UDP_QSIZ];/* Circular packet queue	*/
};

void udp_in(struct netpacket * pkptr);
void udp_hton(struct netpacket * pktptr);
void udp_ntoh(struct netpacket * pktptr);
void udp_init(void);