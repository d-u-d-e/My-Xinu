#include <kernel.h>
#include <stdarg.h>
#include <conf.h>
#include <uart.h>

extern intmask disable();
extern void restore(intmask);

/*------------------------------------------------------------------------
 * kputc  -  use polled I/O to write a character to the console
 *------------------------------------------------------------------------
 */
syscall kputc(byte c) /* character to write	*/
{
	struct dentry * devptr;
	volatile struct	uart_csreg * csrptr;
	intmask	mask;

	/* Disable interrupts */
	mask = disable();

	/* Get CSR address of the console */

	devptr = (struct dentry *) &devtab[CONSOLE];
	csrptr = (struct uart_csreg *)devptr->dvcsr;

	/* Fail if no console device was found */

	if (csrptr == NULL) {
		restore(mask);
		return SYSERR;
	}

	/* Repeatedly poll the device until it becomes nonbusy */

	while ( (csrptr->lsr & UART_LSR_THRE) == 0 ) {
		;
	}

	/* Write the character */

	csrptr->buffer = c;

	/* Honor CRLF - when writing NEWLINE also send CARRIAGE RETURN	*/

	if (c == '\n') {
		/* Poll until transmitter queue is empty */
		while ( (csrptr->lsr & UART_LSR_THRE) == 0 ) {
			;
		}
		csrptr->buffer = '\r';
	}

	restore(mask);
	return OK;
}



extern void _doprnt(char *, va_list, int (*)(int));

/*------------------------------------------------------------------------
 * kprintf  -  use polled I/O to print formatted output on the console
 *------------------------------------------------------------------------
 */
syscall kprintf(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_doprnt(fmt, ap, (int (*)(int))kputc);
	va_end(ap);
	return OK;
}