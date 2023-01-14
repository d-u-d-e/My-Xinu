#include <ip.h>
#include <net.h>
#include <bufpool.h>
#include <semaphore.h>
#include <lib.h>
#include <arp.h>
#include <udp.h>
#include <icmp.h>

struct iqentry ipoqueue;

static uint16 ipcksum(struct netpacket * pkt);
static void ip_ntoh(struct netpacket * pktptr);
static void ip_hton(struct netpacket * pktptr);
static void ip_local(struct netpacket * pktptr);
static status ip_out(struct netpacket * pkptr);


void ip_in(struct netpacket * pktptr)
{
    /* Verify checksum */

    if (ipcksum(pktptr) != 0){ /* silently discard */
        freebuf((char *)pktptr);
        return;
    }

    /* Convert IP header fields to host order */

    ip_ntoh(pktptr);

    /* Ensure version and length are valid */

    if (pktptr->net_ipvh != 0x45) /* 4 is ipv4, 5 is header length in words of 32 bits (20 bytes) */
    {
        kprintf("IP version failed\n");
		freebuf((char *)pktptr);
		return;
    }

    int32 icmplen;

    /* Verify encapsulated prototcol checksums and then convert	*/
	/* the encapsulated headers to host byte order	*/

    switch (pktptr->net_ipproto)
    {
        case IP_UDP:
            /* Skipping UDP checksum for now */
            udp_ntoh(pktptr);
            break;
        case IP_ICMP:
            icmplen = pktptr->net_iplen - IP_HDR_LEN;
            if (icmp_cksum((char *)&pktptr->net_ictype, icmplen) != 0){
                freebuf((char *)pktptr);
                return;
            }
            icmp_ntoh(pktptr);
            break;
        default:
            break;
    }

    /* If we do not yet have a valid address, accept UDP packets	*/
	/*	(to get DHCP replies) and drop others */

    if (!NetData.ipvalid){
        if (pktptr->net_ipproto == IP_UDP){
            ip_local(pktptr);
            return;
        }
        else{
            freebuf((char *)pktptr);
            return;
        }
    }

    /* If packet is destined for us, accept it; otherwise, drop it	*/

    if ((pktptr->net_ipdst == NetData.ipucast) ||
        (pktptr->net_ipdst == NetData.ipbcast) ||
        (pktptr->net_ipdst == IP_BCAST)){
        ip_local(pktptr);
        return;
    }
    else{
        /* Drop the packet */
		freebuf((char *)pktptr);
		return;  
    }
}

/*------------------------------------------------------------------------
 * ip_local  -  Deliver an IP datagram to the local stack
 *------------------------------------------------------------------------
 */

void ip_local(struct netpacket * pktptr)
{
    /* Use datagram contents to determine how to process */

    switch (pktptr->net_ipproto)
    {
    case IP_UDP:
        udp_in(pktptr);
        return;
    case IP_ICMP:
        icmp_in(pktptr);
        return;
    default:
        freebuf((char *)pktptr);
        return;
    }
}


/*------------------------------------------------------------------------
 * ipcksum  -  Compute the IP header checksum for a datagram
 *------------------------------------------------------------------------
 */

uint16 ipcksum(struct netpacket * pkt)
{
    uint32 cksum = 0;
    uint16 * hptr = (uint16 *)&pkt->net_ipvh;
    uint16 word;

    for (int i = 0; i < 10; i++){ /* 10 since IPv4 header is 20 bytes without options */
        word = *hptr++;
        cksum += (uint32) htons(word); 
    }

    /* Add in carry, and take the ones-complement */

    cksum += (cksum >> 16);
    cksum = 0xFFFF & ~cksum;
    return (uint16) cksum;
}

/*------------------------------------------------------------------------
 * ip_ntoh  -  Convert IP header fields to host byte order
 *------------------------------------------------------------------------
 */

void ip_ntoh(struct netpacket * pktptr)
{
    pktptr->net_iplen = ntohs(pktptr->net_iplen);
    pktptr->net_ipid = ntohs(pktptr->net_ipid);
    pktptr->net_ipfrag = ntohs(pktptr->net_ipfrag);
    pktptr->net_ipsrc = ntohl(pktptr->net_ipsrc);
    pktptr->net_ipdst = ntohl(pktptr->net_ipdst);
}

/*------------------------------------------------------------------------
 * ip_hton  -  Convert IP header fields to network byte order
 *------------------------------------------------------------------------
 */

void ip_hton(struct netpacket * pktptr)
{
    pktptr->net_iplen = htons(pktptr->net_iplen);
    pktptr->net_ipid = htons(pktptr->net_ipid);
    pktptr->net_ipfrag = htons(pktptr->net_ipfrag);
    pktptr->net_ipsrc = htonl(pktptr->net_ipsrc);
    pktptr->net_ipdst = htonl(pktptr->net_ipdst);
}

/*------------------------------------------------------------------------
 *  ip_enqueue  -  Deposit an outgoing IP datagram on the IP output queue
 *------------------------------------------------------------------------
 */

status ip_enqueue(struct netpacket * pktptr)
{
    intmask mask;
    struct iqentry * iptr;

    /* Ensure only one process accesses output queue at a time */

    mask = disable();

    /* Enqueue packet on network output queue */

    iptr = &ipoqueue;
    if (semcount(iptr->iqsem) >= IP_OQSIZ){
        kprintf("ipout: output queue overflow\n");
		freebuf((char *)pktptr);
		restore(mask);
		return SYSERR;
    }
    iptr->iqbuf[iptr->iqtail++] = pktptr;
    if (iptr->iqtail >= IP_OQSIZ){
        iptr->iqtail = 0;
    }
    signal(iptr->iqsem);
    restore(mask);
    return OK;
}

/*------------------------------------------------------------------------
 * ip_send  -  Send an outgoing IP datagram from the local stack
 *------------------------------------------------------------------------
 */

status ip_send(struct netpacket * pktptr)
{
    intmask mask = disable();

    /* Pick up the IP destination address from the packet */

    uint32 dest = pktptr->net_ipdst;

    /* Loop back to local stack if destination 127.0.0.0/8 */

    if ((dest & 0xFF000000) == 0x7F000000){
        ip_local(pktptr);
        restore(mask);
        return OK;
    }

    /* Loop back if the destination matches our IP unicast address	*/

    if (dest == NetData.ipucast){
        ip_local(pktptr);
        restore(mask);
        return OK;
    }

    /* Broadcast if destination is 255.255.255.255 */

    int32 retval;

    if ((dest == IP_BCAST) || 
        (dest == NetData.ipbcast)){
        memcpy(pktptr->net_ethdst, NetData.ethbcast, ETH_ADDR_LEN);
        retval = ip_out(pktptr);
        restore(mask);
        return retval;
    }

    /* If destination is on the local network, next hop is the	*/
	/* destination; otherwise, next hop is default router	*/

    uint32 nxthop;

    if ((dest & NetData.ipmask) == NetData.ipprefix){
        /* Next hop is the destination itself */
        nxthop = dest;
    }
    else{
        /* Next hop is default router on the network */
        nxthop = NetData.iprouter;
    }

    if (nxthop == 0){ /* Dest. invalid or no default route	*/
        freebuf((char *)pktptr);
		return SYSERR;
    }

    /* Resolve the next-hop address to get a MAC address */

    retval = arp_resolve(nxthop, pktptr->net_ethdst);
    if (retval != OK){
        freebuf((char *)pktptr);
        return SYSERR;
    }

    /* Send the packet */
    retval = ip_out(pktptr);
    restore(mask);
    return retval;
}

/*------------------------------------------------------------------------
 *  ip_out  -  Transmit an outgoing IP datagram
 *------------------------------------------------------------------------
 */

status ip_out(struct netpacket * pktptr) 
{
    /* Compute total packet length */

    int32 len; /* Length of ICMP message */
    int32 pktlen;	/* Length of entire packet	*/
    uint16 cksum;

    pktlen = pktptr->net_iplen + ETH_HDR_LEN;

    /* Convert encapsulated protocol to network byte order */

    switch (pktptr->net_ipproto){
        case IP_UDP:
            pktptr->net_udpcksum = 0;
            udp_hton(pktptr);
            /* ...skipping UDP checksum computation */
            break;
        case IP_ICMP:
            icmp_hton(pktptr);

            /* Compute ICMP checksum */

            pktptr->net_iccksum = 0;
            len = pktptr->net_iplen - IP_HDR_LEN;
            cksum = icmp_cksum((char *)&pktptr->net_ictype, len);
            pktptr->net_iccksum = 0xFFFF & htons(cksum);
            break;
        default:
            break;
    }

    /* Convert IP fields to network byte order */

    ip_hton(pktptr);
    /* Compute IP header checksum */

    pktptr->net_ipcksum = 0;
    cksum = ipcksum(pktptr);
    pktptr->net_ipcksum = 0xFFFF & htons(cksum);

    /* Convert Ethernet fields to network byte order */

    eth_hton(pktptr);

    /* Send packet over the Ethernet */

    int32 retval = write(ETHER0, (char *)pktptr, pktlen);
    freebuf((char *)pktptr);

    if (retval == SYSERR){
        return SYSERR;
    }
    else{
        return OK;
    }
}

/*------------------------------------------------------------------------
 *  ipout  -  Process that transmits IP packets from the IP output queue
 *------------------------------------------------------------------------
 */

process ipout(void)
{
    struct iqentry * ipqptr = &ipoqueue;
    uint32 destip;
    struct netpacket * pktptr;	/* Pointer to next the packet	*/
    uint32 nxthop;
    int32 retval;

    while(1){
        /* Obtain next packet from the IP output queue */

        wait(ipqptr->iqsem);

        pktptr = ipqptr->iqbuf[ipqptr->iqhead++];
        if (ipqptr->iqhead >= IP_OQSIZ){
            ipqptr->iqhead = 0;    
        }

        /* Fill in the MAC source address */

        memcpy(pktptr->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);

        /* Extract destination address from packet */

        destip = pktptr->net_ipdst;

        /* Sanity check: packets sent to ipout should *not*	*/
		/* contain a broadcast address. */

        if ((destip == IP_BCAST) || (destip == NetData.ipbcast)) {
			kprintf("ipout: encountered a broadcast\n");
			freebuf((char *)pktptr);
			continue;
		}

        /* Check whether destination is the local computer */

        if (destip == NetData.ipucast){
            ip_local(pktptr);
            continue;
        }

        if ((destip & NetData.ipmask) == NetData.ipprefix){
            /* Next hop is the destination itself */
            nxthop = destip;
        }
        else{
            /* Next hop is default router on the network */
            nxthop = NetData.iprouter;
        }

        if (nxthop == 0) {  /* Dest. invalid or no default route*/
			freebuf((char *)pktptr);
			continue;
		}

        retval = arp_resolve(nxthop, pktptr->net_ethdst);
        if (retval != OK){
            freebuf((char *)pktptr);
            continue;
        }

        /* Send the packet */
        ip_out(pktptr);
    }
}