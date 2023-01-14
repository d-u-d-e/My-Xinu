#pragma once
#include <kernel.h>
#include <ether.h>

/* Format of an Ethernet packet carrying IPv4 and UDP */

#define NETSTK		8192 		/* Stack size for network setup */
#define NETPRIO		500    		/* Network startup priority 	*/


/* NETWORK BYTE ORDER CONVERSION NOT NEEDED ON A BIG-ENDIAN COMPUTER */

#define	htons(x)   ( (0xFF & ((x) >> 8)) | ((0xFF & (x)) << 8) )
#define	htonl(x)   ( (((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | \
		     (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) )

#define	ntohs(x)   ( (0xFF & ((x) >> 8) ) | ( (0xFF & (x)) << 8) )
#define	ntohl(x)   (  (((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | \
		      (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) )


#define	ETH_ARP     0x0806		/* Ethernet type for ARP	*/
#define	ETH_IP      0x0800		/* Ethernet type for IP		*/
#define	ETH_IPv6    0x86DD		/* Ethernet type for IPv6	*/

#define NETBOOTFILE	128		/* Size of the netboot filename	*/

#pragma pack(2)
struct netpacket{
	byte	net_ethdst[ETH_ADDR_LEN];/* Ethernet dest. MAC address	*/
	byte	net_ethsrc[ETH_ADDR_LEN];/* Ethernet source MAC address	*/
	uint16	net_ethtype;		/* Ethernet type field		*/
	byte	net_ipvh;		/* IP version and hdr length	*/
	byte	net_iptos;		/* IP type of service		*/
	uint16	net_iplen;		/* IP total packet length	*/
	uint16	net_ipid;		/* IP datagram ID		*/
	uint16	net_ipfrag;		/* IP flags & fragment offset	*/
	byte	net_ipttl;		/* IP time-to-live		*/
	byte	net_ipproto;	/* IP protocol (actually type)	*/
	uint16	net_ipcksum;	/* IP checksum			*/
	uint32	net_ipsrc;		/* IP source address		*/
	uint32	net_ipdst;		/* IP destination address	*/
	union {
	 struct {
	  uint16 	net_udpsport;	/* UDP source protocol port	*/
	  uint16	net_udpdport;	/* UDP destination protocol port*/
	  uint16	net_udplen;	/* UDP total length		*/
	  uint16	net_udpcksum;	/* UDP checksum			*/
	  byte		net_udpdata[1500-28];/* UDP payload (1500-above)*/
	 };
	 struct {
	  byte		net_ictype;	/* ICMP message type		*/
	  byte		net_iccode;	/* ICMP code field (0 for ping)	*/
	  uint16	net_iccksum;	/* ICMP message checksum	*/
	  uint16	net_icident; 	/* ICMP identifier		*/
	  uint16	net_icseq;	/* ICMP sequence number		*/
	  byte		net_icdata[1500-28];/* ICMP payload (1500-above)*/
	 };
	};
};
#pragma pack()

#define	PACKLEN	sizeof(struct netpacket)

struct network { /* Network information	*/
    uint32  ipucast; /* Computer's IP unicast address */
    uint32  ipbcast; /* IP broadcast address	*/
    uint32  ipmask;  /* IP address mask */
    uint32  ipprefix; /* IP (network) prefix */
	uint32	bootserver;	/* Boot server address		*/
    uint32  iprouter; /* Default router address */
    uint32  dnsserver;	/* DNS server address */
    uint32  ntpserver; /* NTP (time) server address */
    bool8   ipvalid; /* nonzero => above are valid	*/
    byte    ethucast[ETH_ADDR_LEN]; /* Ethernet unicast address */
    byte    ethbcast[ETH_ADDR_LEN]; /* Ethernet broadcast address */
	char 	bootfile[NETBOOTFILE]; /* Name of boot file	*/
};

extern struct network NetData;

void eth_ntoh(struct netpacket * pktptr);
void eth_hton(struct netpacket * pktptr);

extern ibid32 netbufpool;
