#include <tty.h>
#include <semaphore.h>

devcall ttygetc(struct dentry * devptr)
{
    char ch;
    struct ttycblk * typtr = &ttytab[devptr->dvminor];

    /* Wait for a character in the buffer and extract one character	*/

    wait(typtr->tyisem);

    ch = *typtr->tyihead++;
    
    /* Wrap around to beginning of buffer, if needed */

	if (typtr->tyihead >= &typtr->tyibuff[TY_IBUFLEN]) {
		typtr->tyihead = typtr->tyibuff;
	}

    /* In cooked mode, check for the EOF character */

    if ((typtr->tyimode == TY_IMCOOKED) && (typtr->tyeof) &&
        (ch == typtr->tyeofch)){
            return (devcall)EOF;
    }

    return (devcall)ch;
} 