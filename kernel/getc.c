#include <kernel.h>
#include <device.h>
#include <conf.h>

/*------------------------------------------------------------------------
 *  putc  -  Send one character of data (byte) to a device
 *------------------------------------------------------------------------
 */

syscall getc(did32 descrp)
{
    intmask mask = disable();
    struct dentry * devptr;	/* Entry in device switch table	*/
	int32 retval;		/* Value to return to caller	*/

    if (isbaddev(descrp)){
        restore(mask);
        return SYSERR;
    }
    devptr = (struct dentry *) &devtab[descrp];
    retval = (*devptr->dvgetc) (devptr);
    restore(mask);
    return retval;
}