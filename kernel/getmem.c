#include <kernel.h>
#include <memory.h>

/*------------------------------------------------------------------------
 *  getmem  -  Allocate heap storage, returning lowest word address
 *------------------------------------------------------------------------
 */

char * getmem(uint32 nbytes)
{
    intmask mask;

    mask = disable();
    if (nbytes == 0){
        restore(mask);
        return (char *)SYSERR;
    }

    // get a multiple of 8 bytes
    nbytes = (uint32) roundmb(nbytes);

    struct memblk * prev, * curr, * leftover;

    prev = &memlist;
    curr = memlist.mnext;
    while (curr != NULL){
        if (curr->mlength == nbytes){ /* Block is exact match */
            prev->mnext = curr->mnext;
            memlist.mlength -= nbytes;
            restore(mask);
            return (char *)(curr);
        }
        else if (curr->mlength > nbytes){ /* Split big block	*/
            leftover = (struct memblk *)((uint32)curr + nbytes);
            prev->mnext = leftover;
            leftover->mnext = curr->mnext;
            leftover->mlength = curr->mlength - nbytes;
            memlist.mlength -= nbytes;
            restore(mask);
            return (char *)(curr);
        }
        else{ /* Move to next block	*/
            prev = curr;
            curr = curr->mnext;
        }
    }
    restore(mask);
    return (char *)SYSERR;
}