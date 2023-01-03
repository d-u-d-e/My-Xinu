#include <kernel.h>
#include <conf.h>

/*------------------------------------------------------------------------
 * ramopen  -  Open a ram disk
 *------------------------------------------------------------------------
 */

devcall ramopen(struct dentry * devptr, char * name, char * mode)
{
    /* No action -- just return the device descriptor */
    return devptr->dvnum;
}