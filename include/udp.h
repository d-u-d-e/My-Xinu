#pragma once

#include <kernel.h>
#include <net.h>

#define	UDP_SLOTS	6 		/* Number of open UDP endpoints */
#define	UDP_QSIZ	8		/* Packets enqueued per endpoint*/

/* Constants for the state of an entry */

#define	UDP_FREE	0 		/* Entry is unused		*/
#define	UDP_USED	1		/* Entry is being used		*/
#define	UDP_RECV	2		/* Entry has a process waiting	*/

#define	UDP_DHCP_CPORT	68		/* Port number for DHCP client	*/
#define	UDP_DHCP_SPORT	67		/* Port number for DHCP server	*/

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
status udp_sendto(uid32 slot, uint32 remip, uint16 remport, char * buf, int32 len);
status udp_send(uid32 slot, char * buf, int32 len);
int32 udp_recvaddr(uid32 slot, uint32 * remip, uint16 * remport, char * buf, int32 len, uint32 timeout);
int32 udp_recv(uid32 slot, char * buf, int32 len, uint32 timeout);
uid32 udp_register(uint32 remip, uint16 remport, uint16 locport);
status udp_release(uid32 slot);