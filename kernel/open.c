#include <kernel.h>
#include <conf.h>
#include <device.h>

/*------------------------------------------------------------------------
 *  Open  -  Open a device (some devices ignore name and mode parameters)
 *------------------------------------------------------------------------
 */

syscall open(did32 descrp, char * name, char * mode) /* Descriptor for device */
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
    retval = (*devptr->dvopen)(devptr, name, mode);
    restore(mask);
    return retval;
}
