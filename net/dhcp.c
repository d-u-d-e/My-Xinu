#include <dhcp.h>
#include <udp.h>
#include <lib.h>
#include <ip.h>

static void dhcp_bld_bootp_msg(struct dhcpmsg * dmsg);
static int32 dhcp_bld_disc(struct dhcpmsg * dmsg);
static char * dhcp_get_opt_val(const struct dhcpmsg * dmsg, uint32 dmsg_size, uint8 option_key);
static int32 dhcp_bld_req(struct dhcpmsg * dmsg, const struct dhcpmsg * dmsg_offer, uint32 dsmg_offer_size);

/*------------------------------------------------------------------------
 * getlocalip - use DHCP to obtain an IP address
 *------------------------------------------------------------------------
 */

uint32 getlocalip(void)
{
    int32 slot = udp_register(0, UDP_DHCP_SPORT, UDP_DHCP_CPORT);
    if (slot == SYSERR){
        kprintf("getlocalip: cannot register with UDP\n");
        return SYSERR;
    }

    struct dhcpmsg dmsg_snd, dmsg_rcv;
    int32 len = dhcp_bld_disc(&dmsg_snd);
    int32 inlen;
    char * eop, * optptr;
    int32 msgtype;
    uint32 addrmask, routeraddr, dnsaddr, ntpaddr;
    uint32 tmp;
    uint32 * tmp_server_ip;

    for (int i = 0; i < DHCP_RETRY; i++){
        udp_sendto(slot, IP_BCAST, UDP_DHCP_SPORT, (char *)&dmsg_snd, len);
        /* Read 3 incoming DHCP messages and check for an offer	*/
	    /* or wait for three timeout periods if no message */
	    /* arrives.	*/

        for (int j = 0; i < 3; j++){
            inlen = udp_recv(slot, (char *)&dmsg_rcv, sizeof(struct dhcpmsg), 2000);
            if (inlen == TIMEOUT){
                continue;
            }
            else if(inlen == SYSERR){
                return SYSERR;
            }
            /* Check that incoming message is a valid	*/
		    /* response (ID matches our request) */

            if (dmsg_rcv.dc_xid != dmsg_snd.dc_xid){
                continue;
            }

            eop = (char *)&dmsg_rcv + inlen - 1;
            optptr = (char *)&dmsg_rcv.dc_opt;
            msgtype = addrmask = routeraddr = dnsaddr = ntpaddr = 0;

            /* parse options */
            while (optptr < eop){
                switch(*optptr){
                    case DHCP_MESSAGE_TYPE:
                        msgtype = 0xFF & *(optptr + 2);
                        break;
                    case DHCP_SUBNET_MASK:
                        memcpy((void *)&tmp, optptr + 2, 4);
                        addrmask = ntohl(tmp);
                        break;
                    case DHCP_ROUTER:
                        memcpy((void *)&tmp, optptr + 2, 4);
                        routeraddr = ntohl(tmp);
                        break;
                    case DHCP_DNS_SERVER:
                        memcpy((void *)&tmp, optptr + 2, 4);
                        dnsaddr = ntohl(tmp);
                        break;
                    case DHCP_NTP_SERVER:
                        memcpy((void *)&tmp, optptr+2, 4);
				        ntpaddr = ntohl(tmp);
				        break;
                }
                optptr++; /* Move to length octet */
                optptr += (0xFF & *optptr) + 1;
            }

            if (msgtype == 0x02){ /* DHCPOFFER*/
                len = dhcp_bld_req(&dmsg_snd, &dmsg_rcv, inlen);
                if (len == SYSERR){
                    kprintf("getlocalip: %s\n", "unable to build DHCP request");
                    return SYSERR;
                }
                udp_sendto(slot, IP_BCAST, UDP_DHCP_SPORT, (char *)&dmsg_snd, len);
                continue;
            }
            else if (dmsg_rcv.dc_opt[2] != 0x05){
                /* If not an ack skip it */
			    continue;
            }

            /* ACK arrived */

            if (addrmask != 0){
                NetData.ipmask = addrmask;
            }
            if (routeraddr != 0){
                NetData.iprouter = routeraddr;
            }
            if (dnsaddr != 0){
                NetData.dnsserver = dnsaddr;
            }
            if (ntpaddr != 0){
                NetData.ntpserver = ntpaddr;
            }

            NetData.ipucast = ntohl(dmsg_rcv.dc_yip);
            NetData.ipprefix = NetData.ipucast & NetData.ipmask;
            NetData.ipbcast = NetData.ipprefix | ~NetData.ipmask;
            NetData.ipvalid = TRUE;
            udp_release(slot);

            /* Retrieve the boot server IP */

            if (dot2ip((char *)dmsg_rcv.sname, &NetData.bootserver) != OK){
                /* could not retrieve boot server from BOOTP fields */
                /* use DHCP server address */

                tmp_server_ip = (uint32 *)dhcp_get_opt_val(&dmsg_rcv, inlen, DHCP_SERVER_ID);
                memcpy((char *)&tmp, tmp_server_ip, 4);
                NetData.bootserver = ntohl(tmp);
            }
            memcpy(NetData.bootfile, dmsg_rcv.bootfile, sizeof(dmsg_rcv.bootfile));
            return NetData.ipucast;
        }
    }

    kprintf("DHCP failed to get a response\n");
    udp_release(slot);
    return (uint32)SYSERR;
}

/*------------------------------------------------------------------------
 * dhcp_bld_req - handcraft a DHCP request message in dmsg
 *------------------------------------------------------------------------
 */
int32 dhcp_bld_req(struct dhcpmsg * dmsg, const struct dhcpmsg * dmsg_offer, uint32 dsmg_offer_size)
{
    uint32 j = 0;
    dhcp_bld_bootp_msg(dmsg);
    dmsg->dc_sip = dmsg_offer->dc_sip;

    dmsg->dc_opt[j++] = 0xFF & DHCP_MESSAGE_TYPE;
    dmsg->dc_opt[j++] = 0xFF & 1; /* Option length */
    dmsg->dc_opt[j++] = 0xFF & 3; /* DHCP Request message */
    dmsg->dc_opt[j++] = 0xFF & 0; /* Options padding */

    dmsg->dc_opt[j++] = 0xFF & DHCP_REQUESTED_IP;
    dmsg->dc_opt[j++] = 0xFF & 4;  /* Option length */
    memcpy(&dmsg->dc_opt[j], &dmsg_offer->dc_yip, 4);
    j += 4;

    /* Retrieve the DHCP server IP from the DHCP options */

    uint32 * server_ip = (uint32 *)dhcp_get_opt_val(dmsg_offer, dsmg_offer_size, DHCP_SERVER_ID);

    if (server_ip == 0){
        kprintf("Unable to get server IP add. from DHCP Offer\n");
		return SYSERR;
    }

    dmsg->dc_opt[j++] = 0xFF & DHCP_SERVER_ID; /* Set server identifier */
    dmsg->dc_opt[j++] = 0xFF & 4;  /* Option length */
    memcpy(&dmsg->dc_opt[j], server_ip, 4);
    j += 4;
    dmsg->dc_opt[j++] = DHCP_MESSAGE_END; /* End of options */

    return (uint32)((char *)&dmsg->dc_opt[j] - (char *)dmsg); 
}

/*------------------------------------------------------------------------
 * dhcp_bld_disc  -  handcraft a DHCP Discover message in dmsg
 *------------------------------------------------------------------------
 */
int32 dhcp_bld_disc(struct dhcpmsg * dmsg)
{
    uint32 j = 0;
    dhcp_bld_bootp_msg(dmsg);

    dmsg->dc_opt[j++] = 0xFF & DHCP_MESSAGE_TYPE;
    dmsg->dc_opt[j++] = 0xFF & 1; /* Option length */
    dmsg->dc_opt[j++] = 0xFF & 1; /* DHCP Dicover message */
    dmsg->dc_opt[j++] = 0xFF & 0; /* Options padding */

    dmsg->dc_opt[j++] = 0xFF & DHCP_PARAMETER_REQUEST_LIST;
    dmsg->dc_opt[j++] = 0xFF & 2;  /* Option length */
    dmsg->dc_opt[j++] = 0xFF & DHCP_SUBNET_MASK; /* Request subnet mask */
    dmsg->dc_opt[j++] = 0xFF & DHCP_ROUTER; /* Request default router addr */
    dmsg->dc_opt[j++] = DHCP_MESSAGE_END; /* End of options */

    return (uint32)((char *)&dmsg->dc_opt[j] - (char *)dmsg);
}


/*------------------------------------------------------------------------
 * dhcp_bld_bootp_msg  -  Set the common fields for all DHCP messages
 *------------------------------------------------------------------------
 */
void dhcp_bld_bootp_msg(struct dhcpmsg * dmsg)
{
    uint32 xid; /* Xid used for the exchange */

    memcpy(&xid, NetData.ethucast, 4); /* Use 4 bytes from MAC as unique XID */
    memset(dmsg, 0x00, sizeof(struct dhcpmsg));

    dmsg->dc_bop = 0x01; /* Outgoing request */
    dmsg->dc_htype = 0x01; /* Hardware type is Ethernet	*/
    dmsg->dc_hlen = 0x06; /* Hardware address length */
    dmsg->dc_hops = 0x00; /* Hop count */
    dmsg->dc_xid = htonl(xid);
    dmsg->dc_secs = 0x0000;
    dmsg->dc_flags = 0x0000;
    dmsg->dc_cip = 0x00000000; /* Client IP address	*/
    dmsg->dc_sip = 0x00000000; /* Server IP address	*/
    dmsg->dc_gip = 0x00000000; /* Gateway IP address */
    dmsg->dc_yip = 0x00000000; /* Your IP address */
    memset(&dmsg->dc_chaddr, '\0', 16);
    memcpy(&dmsg->dc_chaddr, NetData.ethucast, ETH_ADDR_LEN);
    memset(&dmsg->dc_bootp, '\0', 192); /* Zero the bootp area */
    dmsg->dc_cookie = htonl(0x63825363U);
}

/*------------------------------------------------------------------------
 * dhcp_get_opt_val  -	Retrieve a pointer to the value for a specified
 *			  DHCP options key
 *------------------------------------------------------------------------
 */
char * dhcp_get_opt_val(const struct dhcpmsg * dmsg, uint32 dmsg_size, uint8 option_key)
{
    unsigned char * eom, * opt_tmp;

    eom = (unsigned char *)dmsg + dmsg_size - 1;
    opt_tmp = (unsigned char *)dmsg->dc_opt;

    while(opt_tmp < eom){
        /* If the option value matches return the value */

        if (*opt_tmp == option_key){
            return (char *)(opt_tmp + 2); /* past option value and size */
        }

        opt_tmp++; // move to len octet
        opt_tmp += *(uint8 *)opt_tmp + 1;
    }

    /* Option value not found */
    return NULL;
}
