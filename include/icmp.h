#pragma once

#include <net.h>

#define	ICMP_SLOTS	10		/* num. of open ICMP endpoints	*/
#define	ICMP_QSIZ	8		/* incoming packets per slot	*/

void icmp_in(struct netpacket * pkptr);
uint16 icmp_cksum(char * buf, int32 buflen);
void icmp_hton(struct netpacket * pkptr);
void icmp_ntoh(struct netpacket * pkptr);
void icmp_init(void);

/* table of processes that are waiting for ping replies */

struct	icmpentry {			/* entry in the ICMP table	*/
	int32	icstate;		/* state of entry: free/used	*/
	uint32	icremip;		/* remote IP address		*/
	int32	ichead;			/* index of next packet to read	*/
	int32	ictail;			/* index of next slot to insert	*/
	int32	iccount;		/* count of packets enqueued	*/
	pid32	icpid;			/* ID of waiting process	*/
	struct	netpacket * icqueue[ICMP_QSIZ];/* circular packet queue	*/
};

#define ICMP_HDR_LEN	8		/* bytes in an ICMP header	*/


/* Constants for the state of an entry */

#define	ICMP_FREE	0		/* entry is unused		*/
#define	ICMP_USED	1		/* entry is being used		*/
#define	ICMP_RECV	2		/* entry has a process waiting	*/

/* ICMP message types for ping */

#define	ICMP_ECHOREPLY	0		/* ICMP Echo Reply message	*/
#define ICMP_ECHOREQST	8		/* ICMP Echo Request message	*/

extern struct icmpentry icmptab[ICMP_SLOTS]; /* Table of processes using ping*/
