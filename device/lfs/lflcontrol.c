#include <lfs.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * lflcontrol  -  Provide control functions for a local file pseudo-device
 *------------------------------------------------------------------------
 */

devcall lflcontrol(struct dentry * devptr, int32 func, int32 arg1, int32 arg2)
{
    struct lflcblk * lfptr; /* Ptr to open file table entry	*/

    int32 retval;

    /* Obtain exclusive use of the file */

    lfptr = &lfltab[devptr->dvminor];
    wait(lfptr->lfmutex);

    /* If file is not open, return an error */

    if (lfptr->lfstate != LF_USED){
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    switch (func)
    {
    case LF_CTL_TRUNC:
        /* Truncate a file */
        wait(Lf_data.lf_mutex);
        retval = lftruncate(lfptr);
        signal(Lf_data.lf_mutex);
        signal(lfptr->lfmutex);
        return retval;
    default:
        kprintf("lfcontrol: function %d not valid\n", func);
        signal(lfptr->lfmutex);
        return SYSERR;
    }
}