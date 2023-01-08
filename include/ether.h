#pragma once

#include <conf.h>

#define	ETH_ADDR_LEN	6	/* Length of Ethernet (MAC) address	*/
typedef	unsigned char Eaddr[ETH_ADDR_LEN];/* a physical Ethernet address*/

#define	ETH_BUF_SIZE		1518	/* 1500 MTU + 14 ETH Header + 4 bytes optional VLAN Tagging (CRC not passed by default) */

/* State of the Ethernet interface */

#define	ETH_STATE_FREE		0	/* control block is unused 	*/
#define	ETH_STATE_DOWN		1	/* interface is currently inactive 		*/
#define	ETH_STATE_UP		2	/* interface is currently active */


/* Ethernet multicast */

#define ETH_NUM_MCAST		32     /* Max number of multicast addresses*/

/* Ehternet NIC type */

#define ETH_TYPE_3C905C 	1
#define ETH_TYPE_E1000E 	2

struct ethcblk{
    byte state; /* ETH_STATE_... as defined above 	*/
    struct dentry * phy; /* physical eth device for Tx DMA */
    byte type; /* NIC type as defined above 	*/
    /* Pointers to associated structures */
    struct dentry * dev;
    void * csr;
    uint32 pcidev;
    uint32 iobase;
    uint32 flashbase;
    uint32 membase;

    void * rxRing; /* ptr to array of recv ring descriptors */
    void * rxBufs; /* ptr to Rx packet buffers in memory	*/
    uint32 rxHead; /* Index of current head of Rx ring	*/
    uint32 rxTail; /* Index of current tail of Rx ring	*/
    uint32 rxRingSize; /* size of Rx ring descriptor array	*/
    uint32 rxIrq; /* Count of Rx interrupt requests */

    void * txRing; 	/* ptr to array of xmit ring descriptors*/
	void * txBufs; 	/* ptr to Tx packet buffers in memory	*/
	uint32 txHead;		/* Index of current head of Tx ring	*/
	uint32 txTail;		/* Index of current tail of Tx ring	*/
	uint32 txRingSize;	/* size of Tx ring descriptor array	*/
	uint32 txIrq;		/* Count of Tx interrupt requests       */

    byte devAddress[ETH_ADDR_LEN]; /* MAC address */

    uint8 addrLen; /* Hardware address length */
    uint32 mtu; /* Maximum transmission unit (payload)  */

    uint32 errors; /* Number of Ethernet errors */
    sid32 isem;		/* Semaphore for Ethernet input		*/
	sid32 osem; 		/* Semaphore for Ethernet output	*/
    uint16 istart;		/* Index of next packet in the ring     */
    int16 inPool;		/* Buffer pool ID for input buffers 	*/
    int16 outPool;	/* Buffer pool ID for output buffers	*/

    int16 proms; 		/* nonzero => promiscuous mode 		*/

    int16 ed_mcset;     /* nonzero => multicast reception set   */
    int16 ed_mcc;	 	/* count of multicast addresses		*/

    Eaddr ed_mca[ETH_NUM_MCAST]; /* array of multicast addrs 	*/
};

struct etherPkt{
    byte dst[ETH_ADDR_LEN]; /* Destination Mac address	*/
    byte src[ETH_ADDR_LEN]; /* Source Mac address		*/
    uint32 type; /* Ether type field		*/
    byte data[1]; /* Packet payload		*/
};

extern struct ethcblk ethertab[];

devcall ethread(struct dentry * devptr, char * buff, uint32 count);
devcall ethwrite(struct dentry * devptr, char * buff, uint32 count);
devcall ethinit(struct dentry * devptr);
devcall ethcontrol(struct dentry * devptr, int32 func, int32 arg1, int32 arg2);
void    ethhandler(uint32);


/* Ethernet device control functions */

#define	ETH_CTRL_GET_MAC     	1 	/* Get the MAC for this device	*/

#define ETH_PHY_CTLREG	0
#define ETH_PHY_STATREG	1
#define ETH_PHY_CTLREG_RESET	0x8000
#define ETH_PHY_STATREG_LINK	0x0004

#define ETH_PHY_10M		    0x0000
#define ETH_PHY_100M		0x2000
#define ETH_PHY_1000M		0x0040
#define ETH_PHY_CTLREG_SM	0x2040	/* Speed Mask	*/
#define ETH_PHY_CTLREG_FD	0x0100 /* Full duplex */
