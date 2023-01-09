#include <arp.h>
#include <net.h>
#include <bufpool.h>
#include <lib.h>
#include <process.h>
#include <ether.h>
#include <ip.h>

struct arpentry arpcache[ARP_SIZ];

static int32 arp_alloc();

/*------------------------------------------------------------------------
 * arp_init  -  Initialize ARP cache for an Ethernet interface
 *------------------------------------------------------------------------
 */

void arp_init(void){
    int32 i; /* ARP cache index	*/

    for (i = 0; i < ARP_SIZ; i++){
        arpcache[i].arstate = AR_FREE;
    }
}

/*------------------------------------------------------------------------
 * arp_in  -  Handle an incoming ARP packet
 *------------------------------------------------------------------------
 */

void arp_in(struct arppacket * pktptr)
{
    /* convert fro network order to host order */

    arp_ntoh(pktptr);

    /* Verify ARP is for IPv4 and Ethernet */

    if ((pktptr->arp_htype != ARP_HTYPE) || 
        (pktptr->arp_ptype != ARP_PTYPE)){
        freebuf((char *)pktptr);
        return;
    }

    /* Ensure only one process uses ARP at a time */
    intmask mask = disable();

    /* Search cache for sender's IP address */
    bool8 found = FALSE;
    struct arpentry * arptr;
    int32 slot;

    for (slot = 0; slot < ARP_SIZ; slot++){
        arptr = &arpcache[slot];

        /* Skip table entries that are unused */
        if (arptr->arstate == AR_FREE){
            continue;
        }

        /* If sender's address matches, we've found it */
        if (arptr->arpaddr == pktptr->arp_sndpa){
            found = TRUE;
            break;
        }
    }

    if (found){
        /* Update sender's hardware address */
        memcpy(arptr->arhaddr, pktptr->arp_sndha, ARP_HALEN); /* this is stored in network byte order */

        /* If a process was waiting, inform the process */

        if (arptr->arstate == AR_PENDING){
            arptr->arstate = AR_RESOLVED;
            send(arptr->arpid, OK);
        }
    }

    /* For an ARP reply, processing is complete */

    if (pktptr->arp_op == ARP_OP_RPLY){
        freebuf((char *)pktptr);
        restore(mask);
        return;
    }

    /* The following is for an ARP request packet: if the local	*/
	/* machine is not the target or the local IP address is not	*/
	/* yet known, ignore the request (i.e., processing is complete)*/

    if ((!NetData.ipvalid) || pktptr->arp_tarpa != NetData.ipucast){
        freebuf((char *)pktptr);
        restore(mask);
        return;
    }

    /* Request has been sent to the local machine's address.  So,	*/
	/* add sender's info to cache, if not already present */

    if (!found){
        slot = arp_alloc();
        if (slot == SYSERR){ /* Cache is full */
            kprintf("ARP cache overflow on interface\n");
            freebuf((char *)pktptr);
            restore(mask);
            return;
        }
        arptr = &arpcache[slot];
        arptr->arpaddr = pktptr->arp_sndpa;
        memcpy(arptr->arhaddr, pktptr->arp_sndha, ARP_HALEN);
        arptr->arstate = AR_RESOLVED;
    }

    /* Hand-craft an ARP reply packet and send back to requester */

    struct arppacket apkt;		/* Local packet buffer	*/

    memcpy(apkt.arp_ethdst, pktptr->arp_sndha, ARP_HALEN);
    memcpy(apkt.arp_ethsrc, NetData.ethucast, ARP_HALEN);
    apkt.arp_ethtype = ETH_ARP;
    apkt.arp_htype = ARP_HTYPE; /* Hardware is Ethernet	*/
    apkt.arp_ptype = ARP_PTYPE; /* Protocol is IP */
    apkt.arp_hlen = ARP_HALEN;
    apkt.arp_plen = ARP_PALEN;
    apkt.arp_op = ARP_OP_RPLY;

    /* Insert local Ethernet and IP address in sender fields */

    memcpy(apkt.arp_sndha, NetData.ethucast, ARP_HALEN);
    apkt.arp_sndpa = NetData.ipucast;

    /* Copy target Ethernet and IP addresses from request packet */

    memcpy(apkt.arp_tarha, pktptr->arp_sndha, ARP_HALEN);
    apkt.arp_tarpa = pktptr->arp_sndpa;

    /* Convert ARP packet from host to network byte order */

    arp_hton(&apkt);

    /* Convert the Ethernet header to network byte order */

    eth_hton((struct netpacket *)&apkt);

    /* Send the reply */

    write(ETHER0, (char *)&apkt, sizeof(struct arppacket));
    freebuf((char *)pktptr);
    restore(mask);
    return;
}

/*------------------------------------------------------------------------
 * arp_hton  -  Convert ARP packet fields from host to net byte order
 *------------------------------------------------------------------------
 */
void arp_hton(struct arppacket * pktptr)
{
    pktptr->arp_htype = htons(pktptr->arp_htype);
    pktptr->arp_ptype = htons(pktptr->arp_ptype);
    pktptr->arp_op    = htons(pktptr->arp_op);
    pktptr->arp_sndpa = htonl(pktptr->arp_sndpa);
    pktptr->arp_tarpa = htonl(pktptr->arp_tarpa);
}

/*------------------------------------------------------------------------
 * arp_ntoh  -  Convert ARP packet fields from net to host byte order
 *------------------------------------------------------------------------
 */
void arp_ntoh(struct arppacket * pktptr)
{
    pktptr->arp_htype = ntohs(pktptr->arp_htype);
	pktptr->arp_ptype = ntohs(pktptr->arp_ptype);
	pktptr->arp_op    = ntohs(pktptr->arp_op);
	pktptr->arp_sndpa = ntohl(pktptr->arp_sndpa);
	pktptr->arp_tarpa = ntohl(pktptr->arp_tarpa);
}

/*------------------------------------------------------------------------
 * arp_alloc  -  Find a free slot or kick out an entry to create one
 *------------------------------------------------------------------------
 */
int32 arp_alloc()
{
    int32 slot;

    /* Search for a free slot */

    for (slot = 0; slot < ARP_SIZ; slot++){
        if (arpcache[slot].arstate == AR_FREE){
            memset((char *)&arpcache[slot], NULLCH, sizeof(struct arpentry));
            return slot;
        }
    }

    /* Search for a resolved entry */

    for (slot = 0; slot < ARP_SIZ; slot++){
        if (arpcache[slot].arstate == AR_RESOLVED){
            memset((char *)&arpcache[slot], NULLCH, sizeof(struct arpentry));
            return slot;
        }
    }
    
    /* At this point, all slots are pending (should not happen) */

    kprintf("ARP cache size exceeded\n");
    return SYSERR;
}

/*------------------------------------------------------------------------
 * arp_resolve  -  Use ARP to resolve an IP address to an Ethernet address
 *------------------------------------------------------------------------
 */
status arp_resolve(uint32 nxthop, byte mac[ETH_ADDR_LEN])
{
    
    /* Use MAC broadcast address for IP limited broadcast */

    if (nxthop == IP_BCAST){
        memcpy(mac, NetData.ethbcast, ETH_ADDR_LEN);
        return OK;
    }

    /* Use MAC broadcast address for IP network broadcast */
    
    if (nxthop == NetData.ipbcast){
        memcpy(mac, NetData.ethbcast, ETH_ADDR_LEN);
        return OK;
    }

    /* Ensure only one process uses ARP at a time */

    intmask mask = disable();

    /* See if next hop address is already present in ARP cache */

    int32 i;
    struct arpentry * arptr;
    for (i = 0; i < ARP_SIZ; i++){
        arptr = &arpcache[i];
        if (arptr->arstate == AR_FREE)
            continue;
        if (arptr->arpaddr == nxthop){ /* Adddress is in cache */
            break;
        }
    }

    if (i < ARP_SIZ){ /* Entry was found */

        /* If entry is resolved - handle and return */
        if (arptr->arstate == AR_RESOLVED){
            memcpy(mac, arptr->arhaddr, ARP_HALEN);
            restore(mask);
            return OK;
        }

        /* Entry is already pending - return error because	*/
        /* only one process can be waiting at a time	*/

        if (arptr->arstate == AR_PENDING){
            restore(mask);
            return SYSERR;
        }
    }

    /* IP address not in cache -  allocate a new cache entry and */
    /* send an ARP request to obtain the answer	*/

    int32 slot = arp_alloc();
    if (slot == SYSERR){
        restore(mask);
        return SYSERR;
    }

    arptr = &arpcache[slot];
    arptr->arstate = AR_PENDING;
    arptr->arpaddr = nxthop;
    arptr->arpid = currpid;

    /* Hand-craft an ARP Request packet */

    struct arppacket apkt;		/* Local packet buffer */
    memcpy(apkt.arp_ethdst, NetData.ethbcast, ETH_ADDR_LEN);
    memcpy(apkt.arp_ethsrc, NetData.ethucast, ETH_ADDR_LEN);
    apkt.arp_ethtype = ETH_ARP;
    apkt.arp_htype = ARP_HTYPE;
    apkt.arp_ptype = ARP_PTYPE;
    apkt.arp_hlen = ARP_HALEN;
    apkt.arp_plen = ARP_PALEN;
    apkt.arp_op = ARP_OP_REQ;
    memcpy(apkt.arp_sndha, NetData.ethucast, ARP_HALEN);
    apkt.arp_sndpa = NetData.ipucast;
    memset(apkt.arp_tarha, '\0', ARP_HALEN); /* Target HA is unknown */
    apkt.arp_tarpa = nxthop;   /* Target protocol address */

    arp_hton(&apkt);
    eth_hton((struct netpacket *)&apkt);

    /* Send the packet ARP_RETRY times and await response */

    umsg32 msg;
    for (i = 0; i < ARP_RETRY; i++){
        write(ETHER0, (char *)&apkt, sizeof(struct arppacket));
        msg = recvtime(ARP_TIMEOUT);
        if (msg == TIMEOUT){
            continue;
        }
        else if (msg == SYSERR){
            restore(mask);
            return SYSERR;
        }
        else {
            break; /* entry is resolved */
        }
    }

    /* If no response, return TIMEOUT */

    if (msg == TIMEOUT){
        arptr->arstate = AR_FREE; /* Invalidate cache entry */
        restore(mask);
        return TIMEOUT;
    }

    /* Return hardware address */

    memcpy(mac, arptr->arhaddr, ARP_HALEN);
    restore(mask);
    return OK;
}