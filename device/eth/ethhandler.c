#include <ether.h>
#include <am335x_eth.h>
#include <resched.h>
#include <semaphore.h>

void ethhandler(uint32 xnum)
{
    struct eth_a_rx_desc * rdescptr;
    struct eth_a_tx_desc * tdescptr;

    struct ethcblk * ethptr = &ethertab[0];

    struct eth_a_csreg * csrptr = (struct eth_a_csreg *)ethptr->csr;

    if (xnum == ETH_AM335X_RXINT){ /* Receive interrupt */
        /* Get the pointer to last desc in the queue	*/

        rdescptr = (struct eth_a_rx_desc *)ethptr->rxRing + ethptr->rxTail;

        /* Defer scheduling until all descriptors are processed	*/

        resched_cntl(DEFER_START);

        /* The HDP register must never be written to a second time while a previous list is active. To add additional
        descriptors to a descriptor list already owned by the EMAC, the NULL next pointer of the last descriptor of
        the previous list is patched with a pointer to the first descriptor in the new list. The list of new descriptors
        to be appended to the existing list must itself be NULL terminated before the pointer patch is performed.
        If the EMAC reads the next pointer of a descriptor as NULL in the instant before an application appends
        additional descriptors to the list by patching the pointer, this may result in a race condition. Thus, the
        software application must always examine the Flags field of all EOP packets, looking for a special flag
        called end-of-queue (EOQ). The EOQ flag is set by the EMAC on the last descriptor of a packet when the
        descriptor's next pointer is NULL, allowing the EMAC to indicate to the software application that it has
        reached the end of the list. When the software application sees the EOQ flag set, and there are more
        descriptors to process, the application may then submit the new list or missed list portion by writing the
        new list pointer to the same HDP register that started the process */


        /* The data written by the host (buffer descriptor
        address of the last processed buffer) is compared to the data in the register written by the subsystem
        (address of last buffer descriptor used by the subsystem). If the two values are not equal (which means
        that the 3PSW has received more packets than the CPU has processed), the receive packet
        completion interrupt signal remains asserted. If the two values are equal (which means that the host
        has processed all packets that the system has received), the pending interrupt is de-asserted */

        while (semcount(ethptr->isem) < ethptr->rxRingSize){ /* if there is space */

            if (rdescptr->stat & ETH_AM335X_RDS_OWN){  /* is the descriptor owned by EMAC? */
                break;
            }

            if (rdescptr->stat & ETH_AM335X_RDS_EOQ){
                if (rdescptr->next != NULL){ /* ethread added descriptors after the hardware detected a NULL next descriptor */
                    /* restart DMA */
                    csrptr->stateram->rx_hdp[0] = (uint32)rdescptr->next;
                }
                else{
                    /* application has not added free buffers */
                    kprintf("eth: reached end of queue!\n");
                }
            }

             /* Acknowledge the interrupt by writing the completion pointer */
            csrptr->stateram->rx_cp[0] = (uint32)rdescptr;

            ethptr->rxTail++;
            rdescptr++;
            if (ethptr->rxTail >= ethptr->rxRingSize){
                ethptr->rxTail = 0;
                rdescptr = (struct eth_a_rx_desc *)ethptr->rxRing;
            }
            /* Signal the input semaphore	*/
            signal(ethptr->isem);
        }

        /* Acknowledge the receive interrupt */
		csrptr->cpdma->eoi_vector = 0x1;

        /* Resume rescheduling	*/
		resched_cntl(DEFER_STOP);
    }
    else if (xnum == ETH_AM335X_TXINT){

         /* Get the pointer to first desc in the queue	*/
        tdescptr = (struct eth_a_tx_desc *)ethptr->txRing + ethptr->txHead;

        /* Defer scheduling until all descriptors are processed	*/

        resched_cntl(DEFER_START);

        while(semcount(ethptr->osem) < (int32)ethptr->txRingSize) {
            
            /* Do we need to restart DMA? */

            if (tdescptr->stat & ETH_AM335X_TDS_OWN){  /* is the descriptor owned by EMAC? */
                break;
            }

            if (tdescptr->next != NULL && (tdescptr->stat & ETH_AM335X_TDS_EOQ)){
                /* restart DMA */
                csrptr->stateram->tx_hdp[0] = (uint32)tdescptr->next;
            }

             /* Acknowledge the interrupt by writing the completion pointer */
            csrptr->stateram->tx_cp[0] = (uint32)tdescptr;

            ethptr->txHead++;
            tdescptr++;
            
            if (ethptr->txHead >= ethptr->txRingSize){
                ethptr->txHead = 0;
                tdescptr = (struct eth_a_tx_desc *)ethptr->txRing;
            }
            
            /* Signal the output semaphore */
            signal(ethptr->osem);
        }

        /* Acknowledge the transmit interrupt */
		csrptr->cpdma->eoi_vector = 0x2;

        /* Resume rescheduling	*/
		resched_cntl(DEFER_STOP);
    }
}