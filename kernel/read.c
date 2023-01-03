#include <kernel.h>
#include <conf.h>
#include <device.h>

/*------------------------------------------------------------------------
 *  read  -  Read one or more bytes from a device
 *------------------------------------------------------------------------
 */

syscall read(did32 descrp, char * buff, uint32 count)
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
    retval = (*devptr->dvread)(devptr, buff, count);
    restore(mask);
    return retval;
}