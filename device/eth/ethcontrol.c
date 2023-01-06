#include <ether.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * ethcontrol - implement control function for the ethernet device
 *------------------------------------------------------------------------
 */

devcall ethcontrol(struct dentry * devptr, int32 func, int32 arg1, int32 arg2)
{
    struct ethcblk * ethptr = &ethertab[devptr->dvminor];
    int32 retval = OK;

    switch (func)
    {
    case ETH_CTRL_GET_MAC:
        memcpy((byte *)arg1, ethptr->devAddress, ETH_ADDR_LEN);
        break;
    
    default:
        return SYSERR;
        break;
    }
    return retval;
}