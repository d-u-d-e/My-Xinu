#include <udp.h>
#include <process.h>
#include <bufpool.h>
#include <resched.h>
#include <lib.h>
#include <ip.h>

struct udpentry udptab[UDP_SLOTS];	/* Table of UDP endpoints */


/*------------------------------------------------------------------------
 * udp_init  -  Initialize all entries in the UDP endpoint table
 *------------------------------------------------------------------------
 */
void udp_init(void)
{
	for(int i = 0; i < UDP_SLOTS; i++) {
		udptab[i].udstate = UDP_FREE;
	}
}


/*------------------------------------------------------------------------
 * udp_in  -  Handle an incoming UDP packet
 *------------------------------------------------------------------------
 */

void udp_in(struct netpacket * pkptr)
{
    /* Ensure only one process can access the UDP table at a time	*/

    intmask mask = disable();
    struct udpentry * udptr;

    for (int i = 0; i < UDP_SLOTS; i++){
        udptr = &udptab[i];
        if (udptr->udstate == UDP_FREE){
            continue;
        }

        if ((pkptr->net_udpdport == udptr->udlocport) && 
            (udptr->udremport == 0 || pkptr->net_udpsport == udptr->udremport) &&
            (udptr->udremip == 0 || pkptr->net_ipsrc == udptr->udremip)){
            /* Entry matches incoming packet */

            if (udptr->udcount < UDP_QSIZ){
                udptr->udcount++;
                udptr->udqueue[udptr->udtail++] = pkptr;
                if (udptr->udtail >= UDP_QSIZ){
                    udptr->udtail = 0;
                }
                if (udptr->udstate == UDP_RECV){
                    udptr->udstate = UDP_USED;
                    send(udptr->udpid, OK);
                }
                restore(mask);
                return;
            }
        }
    }

    /* No match - simply discard packet */

    freebuf((char *)pkptr);
    restore(mask);
    return;
}

/*------------------------------------------------------------------------
 * udp_ntoh  -  Convert UDP header fields from net to host byte order
 *------------------------------------------------------------------------
 */

void udp_ntoh(struct netpacket * pktptr)
{
    pktptr->net_udpsport = ntohs(pktptr->net_udpsport);
    pktptr->net_udpdport = ntohs(pktptr->net_udpdport);
    pktptr->net_udplen = ntohs(pktptr->net_udplen);
}

/*------------------------------------------------------------------------
 * udp_hton  -  Convert packet header fields from host to net byte order
 *------------------------------------------------------------------------
 */
void udp_hton(struct netpacket * pktptr)
{
    pktptr->net_udpsport = htons(pktptr->net_udpsport);
    pktptr->net_udpdport = htons(pktptr->net_udpdport);
    pktptr->net_udplen = htons(pktptr->net_udplen);
}


/*------------------------------------------------------------------------
 * udp_sendto  -  Send a UDP packet to a specified destination
 *------------------------------------------------------------------------
 */
status udp_sendto(uid32 slot, uint32 remip, uint16 remport, char * buf, int32 len)
{
    intmask	mask;
	struct netpacket * pkt;
	int32 pktlen;
	static uint16 ident = 1;
	char * udataptr;

    /* Ensure only one process can access the UDP table at a time	*/
    mask = disable();

    /* Verify that the slot is valid */
	if (slot < 0 || slot >= UDP_SLOTS){
		restore(mask);
		return SYSERR;
	}

    struct udpentry * udptr = &udptab[slot];

    /* Verify that the slot has been registered and is valid */
	if (udptr->udstate == UDP_FREE){
		restore(mask);
		return SYSERR;
	}

    /* Allocate a network buffer to hold the packet */

    pkt = (struct netpacket *)getbuf(netbufpool);

    if ((int32)pkt == SYSERR){
		restore(mask);
		return SYSERR;
	}

    /* Create a UDP packet in pkt */

    pktlen = ((char *)&pkt->net_udpdata - (char *)pkt) + len;

    memcpy((char *)&pkt->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);
    pkt->net_ethtype = ETH_IP;
    pkt->net_ipvh = 0x45;
    pkt->net_iptos = 0x00;
    pkt->net_iplen = pktlen - ETH_HDR_LEN;
    pkt->net_ipid = ident++;
    pkt->net_ipfrag = 0x0000;
    pkt->net_ipttl = 0xFF;
    pkt->net_ipproto = IP_UDP;
    pkt->net_ipcksum = 0x0000;
    pkt->net_ipsrc = NetData.ipucast;
    pkt->net_ipdst = remip;

    pkt->net_udpsport = udptr->udlocport;
    pkt->net_udpdport = remport;
    pkt->net_udplen = (uint16)(UDP_HDR_LEN + len);
    pkt->net_udpcksum = 0x0000; /* ignore */
    udataptr = (char *)pkt->net_udpdata;
    memcpy(udataptr, buf, len);

    /* Call ipsend to send the datagram */

    ip_send(pkt);
    restore(mask);
    return OK;
}

/*------------------------------------------------------------------------
 * udp_send  -  Send a UDP packet using info in a UDP table entry
 *------------------------------------------------------------------------
 */
status udp_send(uid32 slot, char * buf, int32 len)
{
	intmask	mask;
	struct netpacket * pkt;
	int32 pktlen;
	static uint16 ident = 1;
	char * udataptr;
	uint32 remip;
	uint16 remport;
	uint16 locport;
	uint32 locip;

    /* Ensure only one process can access the UDP table at a time	*/
    mask = disable();

    /* Verify that the slot is valid */
	if (slot < 0 || slot >= UDP_SLOTS){
		restore(mask);
		return SYSERR;
	}

    struct udpentry * udptr = &udptab[slot];

    /* Verify that the slot has been registered and is valid */
	if (udptr->udstate == UDP_FREE){
		restore(mask);
		return SYSERR;
	}

    /* Verify that the slot has a specified remote address */

    remip = udptr->udremip;
    if (remip == 0){
        restore(mask);
        return SYSERR;
    }

    locip = NetData.ipucast;
    remport = udptr->udremport;
    locport = udptr->udlocport;

    /* Allocate a network buffer to hold the packet */

    pkt = (struct netpacket *)getbuf(netbufpool);

    if ((int32)pkt == SYSERR){
		restore(mask);
		return SYSERR;
	}

    /* Create a UDP packet in pkt */

    pktlen = ((char *)&pkt->net_udpdata - (char *)pkt) + len;

    memcpy((char *)&pkt->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);
    pkt->net_ethtype = ETH_IP;
    pkt->net_ipvh = 0x45;
    pkt->net_iptos = 0x00;
    pkt->net_iplen = pktlen - ETH_HDR_LEN;
    pkt->net_ipid = ident++;
    pkt->net_ipfrag = 0x0000;
    pkt->net_ipttl = 0xFF;
    pkt->net_ipproto = IP_UDP;
    pkt->net_ipcksum = 0x0000;
    pkt->net_ipsrc = locip;
    pkt->net_ipdst = remip;

    pkt->net_udpsport = locport;
    pkt->net_udpdport = remport;
    pkt->net_udplen = (uint16)(UDP_HDR_LEN + len);
    pkt->net_udpcksum = 0x0000; /* ignore */
    udataptr = (char *)pkt->net_udpdata;
    memcpy(udataptr, buf, len);

    /* Call ipsend to send the datagram */

    ip_send(pkt);
    restore(mask);
    return OK;
}


/*------------------------------------------------------------------------
 * udp_recvaddr  -  Receive a UDP packet and record the sender's address
 *------------------------------------------------------------------------
 */
int32 udp_recvaddr(uid32 slot, uint32 * remip, uint16 * remport, char * buf, int32 len, uint32 timeout)
{
   intmask	mask;
	struct udpentry * udptr;
	umsg32 msg;
	struct netpacket * pkt;
	int32 i;
	int32 msglen;
	char * udataptr;

    /* Ensure only one process can access the UDP table at a time	*/
    mask = disable();

    /* Verify that the slot is valid */
    if (slot < 0 || slot >= UDP_SLOTS){
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

	/* Verify that the slot has been registered and is valid */

	if (udptr->udstate != UDP_USED) {
		restore(mask);
		return SYSERR;
	}

    /* Wait for a packet to arrive */

    if (udptr->udcount == 0){
        udptr->udstate = UDP_RECV;
        udptr->udpid = currpid;
        msg = recvclr();
        msg = recvtime(timeout); /* Wait for a packet	*/
        udptr->udstate = UDP_USED;
        if (msg == TIMEOUT){
            restore(mask);
            return TIMEOUT;
        }
        else if (msg != OK){
            restore(mask);
            return SYSERR;
        }
    }

    /* Packet has arrived -- dequeue it */

    pkt = udptr->udqueue[udptr->udhead++];
    if (udptr->udhead >= UDP_QSIZ) {
		udptr->udhead = 0;
	}
	udptr->udcount--;

    /* Record sender's IP address and UDP port number */

    *remip = pkt->net_ipsrc;
    *remport = pkt->net_udpsport;

    /* Copy UDP data from packet into caller's buffer */

    msglen = pkt->net_udplen - UDP_HDR_LEN;
    udataptr = (char *)pkt->net_udpdata;
    if (len < msglen){
        msglen = len;
    }
    for (i = 0; i < msglen; i++){
        *buf++ = *udataptr++;
    }
    freebuf((char *)pkt);
    restore(mask);
    return msglen;    
}

/*------------------------------------------------------------------------
 * udp_recv  -  Receive a UDP packet
 *------------------------------------------------------------------------
 */
int32 udp_recv(uid32 slot, char * buf, int32 len, uint32 timeout)
{
    intmask	mask;
	struct udpentry * udptr;
	umsg32 msg;
	struct netpacket * pkt;
	int32 i;
	int32 msglen;
	char * udataptr;

    /* Ensure only one process can access the UDP table at a time	*/
    mask = disable();

    /* Verify that the slot is valid */
    if (slot < 0 || slot >= UDP_SLOTS){
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

	/* Verify that the slot has been registered and is valid */

	if (udptr->udstate != UDP_USED) {
		restore(mask);
		return SYSERR;
	}

    /* Wait for a packet to arrive */

    if (udptr->udcount == 0){
        udptr->udstate = UDP_RECV;
        udptr->udpid = currpid;
        msg = recvclr();
        msg = recvtime(timeout); /* Wait for a packet	*/
        udptr->udstate = UDP_USED;
        if (msg == TIMEOUT){
            restore(mask);
            return TIMEOUT;
        }
        else if (msg != OK){
            restore(mask);
            return SYSERR;
        }
    }

    /* Packet has arrived -- dequeue it */

    pkt = udptr->udqueue[udptr->udhead++];
    if (udptr->udhead >= UDP_QSIZ) {
		udptr->udhead = 0;
	}
	udptr->udcount--;

    /* Copy UDP data from packet into caller's buffer */

    msglen = pkt->net_udplen - UDP_HDR_LEN;
    udataptr = (char *)pkt->net_udpdata;
    if (len < msglen){
        msglen = len;
    }
    for (i = 0; i < msglen; i++){
        *buf++ = *udataptr++;
    }
    freebuf((char *)pkt);
    restore(mask);
    return msglen;
}


/*------------------------------------------------------------------------
 * udp_register  -  Register a remote IP, remote port & local port to
 *		      receive incoming UDP messages from the specified
 *		      remote site sent to the specified local port
 *------------------------------------------------------------------------
 */

uid32 udp_register(uint32 remip, uint16 remport, uint16 locport)
{
    intmask mask;
    int32 slot;
    struct udpentry * udptr;

    /* Ensure only one process can access the UDP table at a time	*/

    mask = disable();

    /* See if request already registered */

    for (slot = 0; slot < UDP_SLOTS; slot++){
        udptr = &udptab[slot];
        if (udptr->udstate == UDP_FREE){
            continue;
        }

        if ((remport == udptr->udremport) &&
            (locport == udptr->udlocport) &&
            (remip == udptr->udremip)){
            
            /* Request is already in the table */

            restore(mask);
            return SYSERR;
        }
    }

    /* Find a free slot and allocate it */

    for (slot = 0; slot < UDP_SLOTS; slot++){
        udptr = &udptab[slot];
        if (udptr->udstate != UDP_FREE){
            continue;
        }
        udptr->udlocport = locport;
        udptr->udremport = remport;
        udptr->udremip = remip;
        udptr->udpid = -1;
        udptr->udcount = 0;
        udptr->udhead = udptr->udtail = 0;
        udptr->udstate = UDP_USED;
        restore(mask);
        return slot;
    }

    restore(mask);
    return SYSERR;
}


/*------------------------------------------------------------------------
 * udp_release  -  Release a previously-registered UDP slot
 *------------------------------------------------------------------------
 */

status udp_release(uid32 slot)
{
    intmask mask;
    struct udpentry * udptr;
    struct netpacket * pkt;

    /* Ensure only one process can access the UDP table at a time	*/
    mask = disable();

    /* Verify that the slot is valid */
    if (slot < 0 || slot >= UDP_SLOTS){
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

    /* Verify that the slot has been registered and is valid */
    if (udptr->udstate == UDP_FREE){
        restore(mask);
        return SYSERR;
    }

    /* Defer rescheduling to prevent freebuf from switching context	*/

    resched_cntl(DEFER_START);
    while (udptr->udcount > 0){
		pkt = udptr->udqueue[udptr->udhead++];
		if (udptr->udhead >= UDP_QSIZ) {
			udptr->udhead = 0;
		}
		freebuf((char *)pkt);
		udptr->udcount--;  
    }
    udptr->udstate = UDP_FREE;
    resched_cntl(DEFER_STOP);
    restore(mask);
    return OK;
}