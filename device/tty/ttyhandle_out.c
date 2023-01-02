#include <kernel.h>
#include <tty.h>
#include <uart.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  ttyhandle_out  -  Handle an output on a tty device by sending more
 *		     characters to the device FIFO (interrupts disabled)
 *------------------------------------------------------------------------
 */

void ttyhandle_out(struct ttycblk * typtr, struct uart_csreg * csrptr)
{
    int32 ochars; /* Number of output chars sent to UART */

    uint32 ier = 0;
    int32 uspace;			/* Space left in onboard UART	*/

    /* If output is currently held, simply ignore the call */

    if (typtr->tyoheld){
        return;
    }

    /* If echo and output queues empty, turn off interrupts */

    if ((typtr->tyehead == typtr->tyetail) && 
        (semcount(typtr->tyosem) >= TY_OBUFLEN)){
        ier = csrptr->ier;
        csrptr->ier = ier & ~UART_IER_ETBEI;
        return;
    }

    /* Initialize uspace to the available space in the Tx FIFO */
    uspace = UART_FIFO_SIZE - csrptr->txfifo_lvl;

    /* While onboard FIFO is not full and the echo queue is	*/
	/* nonempty, xmit chars from the echo queue */

    while ((uspace > 0) && typtr->tyehead != typtr->tyetail) {
        csrptr->buffer = *typtr->tyehead++;
        if (typtr->tyehead >= &typtr->tyebuff[TY_EBUFLEN])
            typtr->tyehead = typtr->tyebuff; // wrap around
        uspace--;
    }

    /* While onboard FIFO is not full and the output queue is	*/
	/* nonempty, transmit chars from the output queue */

    ochars = 0;
    int32 avail = TY_OBUFLEN - semcount(typtr->tyosem); // tyosem holds number of free spaces
    while ((uspace > 0) && (avail > 0)) {
        csrptr->buffer = *typtr->tyohead++;
        if (typtr->tyohead >= &typtr->tyobuff[TY_OBUFLEN]) {
            typtr->tyohead = typtr->tyobuff;
        }
        avail--;
        uspace--;
        ochars++;
    }

    if (ochars > 0){
        signaln(typtr->tyosem, ochars); //at most ochars processes are waiting to send data
    }

    if ((typtr->tyehead == typtr->tyetail) &&
        (semcount(typtr->tyosem) >= TY_OBUFLEN)){
            ier = csrptr->ier;
            csrptr->ier = ier & ~UART_IER_ETBEI;
    }
    return;
}