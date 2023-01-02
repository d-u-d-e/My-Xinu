#include <kernel.h>
#include <conf.h>
#include <tty.h>
#include <uart.h>
#include <resched.h>

/*------------------------------------------------------------------------
 *  ttyhandler  -  Handle an interrupt for a tty (serial) device
 *------------------------------------------------------------------------
 */

extern void ttyhandle_in(struct ttycblk * typtr, struct uart_csreg * csrptr);
extern void ttyhandle_out(struct ttycblk * typtr, struct uart_csreg * csrptr);

void ttyhandler(uint32 xnum)
{
    struct dentry * devptr;
    struct ttycblk * typtr;
    struct uart_csreg * csrptr;
    uint32 iir = 0; /* Interrupt identification	*/
    uint32 lsr = 0; /* Line status */

    /* Get CSR address of the device (assume console for now) */

    devptr = (struct dentry *) &devtab[CONSOLE];
    csrptr = (struct uart_csreg *) devptr->dvcsr;

    typtr = &ttytab[devptr->dvminor];

    /* Decode hardware interrupt request from UART device */

    iir = csrptr->iir;

    if (iir & UART_IIR_IRQ){
        return;
    }

    /* Decode the interrupt cause based upon the value extracted	*/
	/* from the UART interrupt identification register. */

    iir &= UART_IIR_IDMASK;

    switch (iir){
        /* Receiver line status interrupt (error) */

        case UART_IIR_RLSI:
            lsr = csrptr->lsr;
            if (lsr & UART_LSR_BI){ /* Break Interrupt 
            (happens if transmitter is holding the line in the non idle state for more than one byte data) */

                /* Read the RHR register to acknowledge */
                lsr = csrptr->buffer;
            }
            return;

        case UART_IIR_RDA: /* Receiver data available or timed out */
        case UART_IIR_RTO:
            /* If the uart has not received enough characters 
            to fill the FIFO to the trigger point and no further are arriving 
            then a timeout interrupt will occur so the characters are not lost. */

            resched_cntl(DEFER_START);

            /* While chars avail. in UART buffer, call ttyhandle_in	*/

            while( (csrptr->lsr & UART_LSR_DR) != 0){
                ttyhandle_in(typtr, csrptr);
            }

            resched_cntl(DEFER_STOP);
            return;

        case UART_IIR_THRE:
            /* Transmitter output FIFO is empty (i.e., ready for more)	*/
            ttyhandle_out(typtr, csrptr);
            return;
        case UART_IIR_MSC: /* Modem status change (simply ignore) */
            return;
    }
}