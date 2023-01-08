#include <ether.h>
#include <am335x_eth.h>
#include <semaphore.h>
#include <net.h>
#include <lib.h>

devcall ethwrite(struct dentry * devptr, char * buff, uint32 count)
{
    struct ethcblk * ethptr = &ethertab[devptr->dvminor];

    /* Get the pointer to Ethernet CSR */

    struct eth_a_csreg * csrptr = (struct eth_a_csreg *)ethptr->csr;
    
    /* Wait for empty slot in the queue */

    wait(ethptr->osem); /* signalled by handler */

    /* Get pointer to the descriptor */

    struct eth_a_tx_desc * tdescptr = (struct eth_a_tx_desc *)ethptr->txRing + ethptr->txTail;

    /* Adjust count if greater than max. possible packet size */
	if(count > PACKLEN) {
		count = PACKLEN;
	}

    /* Initialize the descriptor */

    tdescptr->next = NULL;
    tdescptr->buflen = count;
    tdescptr->bufoff = 0;
    tdescptr->packlen = count;
    tdescptr->stat = (ETH_AM335X_TDS_SOP | 
                ETH_AM335X_TDS_EOP |
                ETH_AM335X_TDS_OWN | 
                ETH_AM335X_TDS_DIR | 
                ETH_AM335X_TDS_P1);

    /* Copy the packet into the Tx buffer */

    memcpy((char *)tdescptr->buffer, buff, count);

    // packet must be at least 64 bytes including CRC
    // since CRC is inserted by EMAC by default, we pad the packet to be at least 60 bytes

    if (count < 60){
        memset((char *)tdescptr->buffer + count, 0, 60 - count);
        tdescptr->buflen = 60;
        tdescptr->packlen = 60;
    }

    /* Insert the descriptor into Tx queue */

    if (csrptr->stateram->tx_hdp[0] == 0){ // queue is empty
        /* Start DMA */
        csrptr->stateram->tx_hdp[0] = (uint32)tdescptr;
    }
    else{
        /* Tx queue not empty, insert at end */
        /* Mind that the next instruction might execute just after the hardware */
        /* has set the EOQ bit, so we need to take this into account in the handler */

        struct eth_a_tx_desc * start = (struct eth_a_tx_desc *)csrptr->stateram->tx_hdp[0];

        while (start->next != NULL)
        {
            start = start->next; /* find end of queue */
        }
        start->next = tdescptr; /* add the packet */
    }

    /* Increment the tail index of the Tx ring */

    ethptr->txTail++;
    if(ethptr->txTail >= ethptr->txRingSize) {
		ethptr->txTail = 0;
	}
    return count;
}