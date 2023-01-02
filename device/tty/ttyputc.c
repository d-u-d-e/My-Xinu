#include <tty.h>
#include <semaphore.h>

devcall	ttyputc(struct dentry * devptr, char ch)
{
    struct ttycblk * typtr = &ttytab[devptr->dvminor];

    /* Handle output CRLF by sending CR first */

    if (ch == TY_NEWLINE && typtr->tyocrlf) {
        ttyputc(devptr, TY_RETURN);
	}

    wait(typtr->tyosem); /* Wait for space in queue */
    *typtr->tyotail++ = ch;

    /* Wrap around to beginning of buffer, if needed */

	if (typtr->tyotail >= &typtr->tyobuff[TY_OBUFLEN]) {
		typtr->tyotail = typtr->tyobuff;
	}

    /* Start output in case device is idle */

	ttykickout((struct uart_csreg *)devptr->dvcsr);

	return OK;
}