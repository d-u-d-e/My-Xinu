#include <conf.h>
#include <device.h>

/*------------------------------------------------------------------------
 *  control  -  Control a device or a driver (e.g., set the driver mode)
 *------------------------------------------------------------------------
 */

syscall control(did32 descrp, int32 func, int32 arg1, int32 arg2)
{
    intmask mask = disable();

    if (isbaddev(descrp)){
        restore(mask);
        return SYSERR;
    }

    struct dentry * devptr = (struct dentry *) &devtab[descrp];
    int32 retval = (*devptr->dvcntl)(devptr, func, arg1, arg2);
    restore(mask);
    return retval;
}