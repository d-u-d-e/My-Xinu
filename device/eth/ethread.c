#include <ether.h>
#include <am335x_eth.h>
#include <semaphore.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * ethread - read an incoming packet on TI AM335X Ethernet (interrupts disabled)
 *------------------------------------------------------------------------
 */

devcall ethread(struct dentry * devptr, char * buff, uint32 count)
{
    struct ethcblk * ethptr = &ethertab[devptr->dvminor];

    /* Get the pointer to Ethernet CSR */

    struct eth_a_csreg * csrptr = (struct eth_a_csreg *)ethptr->csr;
    
    /* Wait for a packet */
    wait(ethptr->isem); /* signalled by handler */

    /* Get pointer to the descriptor */

    struct eth_a_rx_desc * rdescptr = (struct eth_a_rx_desc *)ethptr->rxRing + ethptr->rxHead;

    /* Increment the head index of rx ring */

    ethptr->rxHead++;
    if(ethptr->rxHead >= ethptr->rxRingSize){
        ethptr->rxHead = 0;
    }

    /* Read the packet length */

    uint32 retval = rdescptr->packlen;
    if (retval > count){
        retval = count;
    }

    /* Copy the packet into user provided buffer */

    memcpy((char *)buff, (char *)rdescptr->buffer, retval); /* one buffer holds 1 packet */

    /* Initialize the descriptor for next packet */

    rdescptr->bufoff = 0; /* SOP packets get buffoff set to the DMA RX state */
    rdescptr->buflen = ETH_BUF_SIZE; /* prev. set to actual number of written bytes in the buffer */
    rdescptr->packlen = 0; /* in SOP packets prev. set to packet len */

    if (rdescptr->next == NULL && (rdescptr->stat & ETH_AM335X_RDS_EOQ)){
        /* we are freeing a slot after the end of queue has been detected; here DMA has been halted */
        /* restart DMA */
        rdescptr->stat = ETH_AM335X_RDS_OWN;
        csrptr->stateram->rx_hdp[0] = (uint32)rdescptr;
    }
    else{
        /* Just insert the descriptor into Rx queue */
        rdescptr->stat = ETH_AM335X_RDS_OWN;
        rdescptr->next = NULL;
        struct eth_a_rx_desc * start = (struct eth_a_rx_desc *)csrptr->stateram->rx_hdp[0];
        while (start->next != NULL){ /* find the end of queue */
            start = start->next;
        }
        start->next = rdescptr;
    }

    return retval;
}