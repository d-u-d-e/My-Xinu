#include <kernel.h>
#include <conf.h>
#include <device.h>

/*------------------------------------------------------------------------
 *  close  -  Close a device
 *------------------------------------------------------------------------
 */

syscall close(did32 descrp) /* Descriptor for device */
{
    intmask mask;
    struct dentry * devptr; /* Entry in device switch table	*/
    int32 retval;

    mask = disable();
    if (isbaddev(descrp)){
        restore(mask);
        return SYSERR;
    }
    devptr = (struct dentry *) &devtab[descrp];
    retval = (*devptr->dvclose)(devptr);
    restore(mask);
    return retval;
}
