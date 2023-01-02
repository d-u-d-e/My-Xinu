#include <uart.h>

/*------------------------------------------------------------------------
 *  ttykickout  -  "Kick" the hardware for a tty device, causing it to
 *		     generate an output interrupt (interrupts disabled)
 *------------------------------------------------------------------------
 */

void ttykickout(struct uart_csreg * csrptr)
{
    /* Force the UART hardware to generate an output interrupt */

	csrptr->ier = UART_IER_ERBFI | UART_IER_ETBEI;

	return;
}