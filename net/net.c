#include <net.h>
#include <lib.h>
#include <icmp.h>
#include <udp.h>
#include <bufpool.h>
#include <arp.h>
#include <process.h>
#include <ip.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * net_init  -  Initialize network data structures and processes
 *------------------------------------------------------------------------
 */

struct network NetData;
ibid32 netbufpool;
uint32 netportseed;

/*------------------------------------------------------------------------
 * net_init  -  Initialize network data structures and processes
 *------------------------------------------------------------------------
 */

static process netin(void);
process ipout(void);

void net_init(void)
{
    int32 nbufs; /* Total no of buffers	*/

    memset(&NetData, NULLCH, sizeof(struct network));

    /* Obtain the Ethernet MAC address */

    control(ETHER0, ETH_CTRL_GET_MAC, (int32)NetData.ethucast, 0);
    memset((char *)NetData.ethbcast, 0xFF, ETH_ADDR_LEN);

    /* Initialize the random port seed */

    netportseed = getticks();

    /* Create the network buffer pool */

    nbufs = UDP_SLOTS * UDP_QSIZ + ICMP_SLOTS * ICMP_QSIZ + 1; // +1 slot for ARP resolve

    netbufpool = mkbufpool(PACKLEN, nbufs);

    /* Initialize the ARP cache */

    arp_init();

    /* Initialize UDP TODO */

    /* Initialize ICMP */

    icmp_init();

    /* Initialize the IP output queue */

    ipoqueue.iqhead = 0;
    ipoqueue.iqtail = 0;
    ipoqueue.iqsem = semcreate(0);
    if ((int32)ipoqueue.iqsem == SYSERR){
        panic("Cannot create ip output queue semaphore");
		return;
    }

    /* Create the IP output process */

    resume(create(ipout, NETSTK, NETPRIO, "ipout", 0, NULL));

    /* Create a network input process */

    resume(create(netin, NETSTK, NETPRIO, "netin", 0, NULL));
}

process netin(void)
{
    struct netpacket * pkt; /* Ptr to current packet */
    int32 retval;

    /* Do forever: read a packet from the network and process */

    while(1){
        /* Allocate a buffer */

        pkt = (struct netpacket *)getbuf(netbufpool);

        /* Obtain next packet that arrives */

        retval = read(ETHER0, (char *)pkt, PACKLEN);

        if (retval == SYSERR){
            panic("Cannot read from Ethernet\n");
        }

        /* Convert Ethernet Type to host order */

        eth_ntoh(pkt);

        /* Demultiplex on Ethernet type */

        switch (pkt->net_ethtype)
        {
        case ETH_ARP: /* Handle ARP	*/
            arp_in((struct arppacket *)pkt);
            continue;;
        case ETH_IP: /* Handle IP */
            ip_in(pkt);
            continue;
        case ETH_IPv6:
            freebuf((char *)pkt); /* Handle IPv6	*/
            continue;
        default:
            freebuf((char *)pkt); /* Ignore all other incoming packets (including those tagged with VLAN)*/
            continue;
        }
    }
    /* never reached */
    return 0;
}

/*------------------------------------------------------------------------
 * eth_ntoh  -  Convert Ethernet type field to host byte order
 *------------------------------------------------------------------------
 */
void eth_ntoh(struct netpacket * pktptr)
{
    pktptr->net_ethtype = ntohs(pktptr->net_ethtype);
}

/*------------------------------------------------------------------------
 * eth_hton  -  Convert Ethernet type field to network byte order
 *------------------------------------------------------------------------
 */
void eth_hton(struct netpacket * pktptr)
{
    pktptr->net_ethtype = htons(pktptr->net_ethtype);
}

/*------------------------------------------------------------------------
 * getport  -  Retrieve a random port number 
 *------------------------------------------------------------------------
 */
uint16 getport()
{
    netportseed = 1103515245 * netportseed + 12345;

    /* The Internet Assigned Numbers Authority (IANA) and RFC 6335 suggests the range 49152–65535 */
    return 50000 + ((uint16)((netportseed >> 16) % 15535));
}