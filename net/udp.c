#include <udp.h>

/*------------------------------------------------------------------------
 * udp_in  -  Handle an incoming UDP packet
 *------------------------------------------------------------------------
 */

void udp_in(struct netpacket * pkptr)
{

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