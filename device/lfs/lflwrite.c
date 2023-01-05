#include <lfs.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * lflwrite  --  Write data to a previously opened local disk file
 *------------------------------------------------------------------------
 */

devcall lflwrite(struct dentry * devptr, char * buff, uint32 count)
{
    if (count < 0){
        return SYSERR;
    }

    /* Iteratate and write one byte at a time */
    int32 i;
    for (i = 0; i < count; i++){
        if (lflputc(devptr, *buff++) == SYSERR){
            return SYSERR;
        }
    }
    return count;
}