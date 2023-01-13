#include <icmp.h>
#include <bufpool.h>
#include <lib.h>
#include <ip.h>
#include <process.h>
#include <resched.h>

struct icmpentry icmptab[ICMP_SLOTS];	/* Table of processes using ping*/


static struct netpacket * icmp_mkpkt(
    uint32 remip, 
    uint16 type, 
    uint16 ident, 
    uint16 seq, 
    char * buf, 
    uint32 len);


/*------------------------------------------------------------------------
 * icmp_init  -  Initialize icmp table
 *------------------------------------------------------------------------
 */
void icmp_init(void)
{
    int32 i;
    for (i = 0; i < ICMP_SLOTS; i++){
        icmptab[i].icstate = ICMP_FREE;
    }
    return;
}

void icmp_in(struct netpacket * pkt)
{
    intmask mask = disable();

    struct netpacket * replypkt;

    /* Discard all ICMP messages except ping */

    if (pkt->net_ictype != ICMP_ECHOREPLY && pkt->net_ictype != ICMP_ECHOREQST){
        freebuf((char *)pkt);
        restore(mask);
        return;
    }

    /* Handle Echo Request message */

    if (pkt->net_ictype == ICMP_ECHOREPLY){
        /* Send echo reply message */
        replypkt = icmp_mkpkt(pkt->net_ipsrc, ICMP_ECHOREPLY, 
            pkt->net_icident, pkt->net_icseq, (char *) &pkt->net_icdata,
            pkt->net_iplen - IP_HDR_LEN - ICMP_HDR_LEN);
        if ((int32)replypkt != SYSERR){
            ip_enqueue(replypkt);
        }
        freebuf((char *)pkt);
        restore(mask);
        return;
    }

    /* Handle Echo Reply message: verify that ID is valid */

    int32 slot = pkt->net_icident;
    if (slot < 0 || slot >= ICMP_SLOTS){
        freebuf((char *)pkt);
        restore(mask);
        return;
    }   

    /* Verify that slot in table is in use and IP address	*/
	/* in incomming packet matches IP address in table	*/

    struct icmpentry * icmptr = &icmptab[slot];
    if (icmptr->icstate == ICMP_FREE){
        freebuf((char *)pkt);
        restore(mask);
        return;
    }

    /* Check address in incoming packet */
    /* If source is not equal to remote IP, and source is not host's ip and source is not loopback */

    if ((pkt->net_ipsrc != icmptr->icremip) &&
        (pkt->net_ipsrc != NetData.ipucast) &&
        (pkt->net_ipsrc & 0x7F000000) != 0x7F000000){
        
        freebuf((char *)pkt);
        restore(mask);
        return;
    }

    /* Add packet to queue */

    icmptr->iccount++;
    icmptr->icqueue[icmptr->ictail++] = pkt;
    if (icmptr->ictail >= ICMP_QSIZ){
        icmptr->ictail = 0;
    }
    if (icmptr->icstate == ICMP_RECV){
        icmptr->icstate = ICMP_USED;
        send(icmptr->icpid, OK);
    }
    restore(mask);
    return;
}

/*------------------------------------------------------------------------
 * icmp_cksum  -  Compute a checksum for a specified set of data bytes
 *------------------------------------------------------------------------
 */
uint16 icmp_cksum(char * buf, int32 buflen)
{
    int32 scount = buflen >> 1; /* Divide by 2 and round down */
    uint16 * sptr = (uint16 *)buf;
    uint32 cksum = 0;
    uint16 word;

    for(; scount > 0; scount--){
        word = (uint32) *sptr++;
        cksum += ntohs(word);
    }

    /* If buffer lenght is odd, add last byte */

    if ((buflen & 0x01) != 0){
        cksum += 0xFFFF & ((uint32)(*(byte *)sptr) << 8); /* plus 0 pad */
    }
    cksum += (cksum >> 16); /* end-around carry */
    cksum = 0XFFFF & ~cksum;
    return (uint16) cksum;
}

/*------------------------------------------------------------------------
 * icmp_hton  -  Convert ICMP ping fields to network byte order
 *------------------------------------------------------------------------
 */
void icmp_hton(struct netpacket * pkptr)
{
    pkptr->net_iccksum = htons(pkptr->net_iccksum);
    pkptr->net_icident = htons(pkptr->net_icident);
    pkptr->net_icseq = htons(pkptr->net_icseq);
}

/*------------------------------------------------------------------------
 * icmp_ntoh  -  Convert ICMP ping fields to host byte order
 *------------------------------------------------------------------------
 */
void icmp_ntoh(struct netpacket * pkptr)
{
    pkptr->net_iccksum = ntohs(pkptr->net_iccksum);
    pkptr->net_icident = ntohs(pkptr->net_icident);
    pkptr->net_icseq = ntohs(pkptr->net_icseq);
}


/*------------------------------------------------------------------------
 * icmp_mkpkt  -  Make an icmp packet by filling in fields
 *------------------------------------------------------------------------
 */
struct netpacket * icmp_mkpkt(
    uint32 remip, 
    uint16 type, 
    uint16 ident, 
    uint16 seq, 
    char * buf, 
    uint32 len) /* Length of data in buffer	*/
{
    /* Allocate packet */

    static uint32 ipident = 32767;

    struct netpacket * pkt = (struct netpacket *)getbuf(netbufpool);

    if ((int32)pkt == SYSERR){
        panic("icmp_mkpkt: cannot get a network buffer\n");
    }

    /* Create icmp packet in pkt */

    memcpy(pkt->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);
    pkt->net_ethtype = ETH_IP;
    pkt->net_ipvh = 0x45;
    pkt->net_iptos = 0x00;
    pkt->net_iplen = IP_HDR_LEN + ICMP_HDR_LEN + len;
    pkt->net_ipid = ipident++;
    pkt->net_ipfrag = 0x0000;
    pkt->net_ipproto = IP_ICMP;
    pkt->net_ipcksum = 0x0000;
    pkt->net_ipsrc = NetData.ipucast;
    pkt->net_ipdst = remip;

    pkt->net_ictype = type;
    pkt->net_iccode = 0; /* Code is zero for ping	*/
    pkt->net_iccksum = 0x0000;
    pkt->net_icident = ident;
    pkt->net_icseq = seq;
    memcpy(pkt->net_icdata, buf, len);
    return pkt;
}


/*------------------------------------------------------------------------
 * icmp_send  -  Send an icmp packet
 *------------------------------------------------------------------------
 */

status icmp_send(
    uint32 remip, 
    uint16 type, 
    uint16 ident, 
    uint16 seq,
    char * buf,
    int32 len
    )
{
    struct netpacket * pkt = icmp_mkpkt(remip, type, ident, seq, buf, len);
    if ((int32)pkt == SYSERR){
        return SYSERR;
    }
    return ip_send(pkt);
}

/*------------------------------------------------------------------------
 * icmp_register  -  Register a remote IP address for ping replies
 *------------------------------------------------------------------------
 */

int32 icmp_register(uint32 remip)
{
    intmask mask = disable();

    /* Find a free slot in the table */

    int32 freeslot = -1;
    struct icmpentry * icmptr;
    for(int i = 0; i < ICMP_SLOTS; i++){
        icmptr = &icmptab[i];
        if (icmptr->icstate == ICMP_FREE){
            if(freeslot == -1){
                freeslot = i;
            }
            else if (icmptr->icremip == remip){
                restore(mask);
                return SYSERR; /* Already registered */
            }
        }
    }
    if (freeslot == -1){ /* No free entries in table */
        restore(mask);
        return SYSERR;
    }

    /* Fill in table entry */

    icmptr = &icmptab[freeslot];
    icmptr->icstate = ICMP_USED;
    icmptr->icremip = remip;
    icmptr->iccount = 0;
    icmptr->ichead = icmptr->ictail = 0;
    icmptr->icpid = -1;
    restore(mask);
    return freeslot;
}

/*------------------------------------------------------------------------
 * icmp_recv  -  Receive an icmp echo reply packet
 *------------------------------------------------------------------------
 */
int32 icmp_recv(int32 icmpid, char * buff, int32 len, uint32 timeout)
{
    if (icmpid < 0 || icmpid >= ICMP_SLOTS){
        return SYSERR;
    }

    /* Insure only one process touches the table at a time */

    intmask mask = disable();

    /* Verify that the ID has been registered and is idle */

    struct icmpentry * icmptr = &icmptab[icmpid];
    if (icmptr->icstate != ICMP_USED){
        restore(mask);
        return SYSERR;
    }

    uint32 msg;
    if (icmptr->iccount == 0){
        icmptr->icstate = ICMP_RECV;
        icmptr->icpid = currpid;
        msg = recvclr();
        msg = recvtime(timeout); /* Wait for a reply */
        icmptr->icstate = ICMP_USED;
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

    struct netpacket * pkt = icmptr->icqueue[icmptr->ichead++];
    if (icmptr->ichead >= ICMP_SLOTS){
        icmptr->ichead = 0;
    }
    icmptr->iccount--;

    /* Copy data from ICMP message into caller's buffer */

    int32 datalen = pkt->net_iplen - IP_HDR_LEN - ICMP_HDR_LEN;
    char * icdataptr = (char *) &pkt->net_icdata;
    int32 i;
    for (i = 0; i < datalen; i++){
        if (i >= len){
            break;
        }
        *buff++ = *icdataptr++;
    }
    freebuf((char *)pkt);
    restore(mask);
    return i;
}

/*------------------------------------------------------------------------
 * icmp_release  -  Release a previously-registered ICMP icmpid
 *------------------------------------------------------------------------
 */

status icmp_release(int32 icmpid) /* Slot in icmptab to release	*/
{
    intmask mask = disable();

    struct icmpentry * icmptr;
    struct netpacket * pkt;

    /* Check arg and ensure entry in table is in use */

    if (icmpid < 0 || icmpid >= ICMP_SLOTS){
        restore(mask);
        return SYSERR;
    }

    icmptr = &icmptab[icmpid];
    if (icmptr->icstate != ICMP_USED){
        restore(mask);
        return SYSERR;
    }

    /* Remove each packet from the queue and free the buffer */

    resched_cntl(DEFER_START);
    while (icmptr->iccount > 0){
        pkt = icmptr->icqueue[icmptr->ichead++];
        if (icmptr->ichead >= ICMP_SLOTS) {
			icmptr->ichead = 0;
		}
        freebuf((char *)pkt); //might reschedule when the loop isn't over without DEFER
        icmptr->iccount--;
    }

    /* Mark the entry free */
    icmptr->icstate = ICMP_FREE;
    resched_cntl(DEFER_STOP);
    restore(mask);
    return OK;
}