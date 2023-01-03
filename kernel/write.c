#include <kernel.h>
#include <conf.h>
#include <device.h>

/*------------------------------------------------------------------------
 *  write  -  Write one or more bytes to a device
 *------------------------------------------------------------------------
 */

syscall write(did32 descrp, char * buff, uint32 count)
{
    intmask mask;
    struct dentry * devptr;
    int32 retval;

    mask = disable();
    if (isbaddev(descrp)){
        restore(mask);
        return SYSERR;
    }
    devptr = (struct dentry *) &devtab[descrp];
    retval = (*devptr->dvwrite)(devptr, buff, count);
    restore(mask);
    return retval;
}