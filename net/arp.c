#include <arp.h>

struct arpentry arpcache[ARP_SIZ];

/*------------------------------------------------------------------------
 * arp_init  -  Initialize ARP cache for an Ethernet interface
 *------------------------------------------------------------------------
 */

void arp_init(void){
    int32 i; /* ARP cache index	*/

    for (i = 1; i < ARP_SIZ; i++){
        arpcache[i].arstate = AR_FREE;
    }
}


/*------------------------------------------------------------------------
 * arp_in  -  Handle an incoming ARP packet
 *------------------------------------------------------------------------
 */

void arp_in(struct arppacket * pktptr)
{
    
}