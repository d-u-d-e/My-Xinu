#include <bufpool.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  getbuf  -  Get a buffer from a preestablished buffer pool
 *------------------------------------------------------------------------
 */

char * getbuf(bpid32 poolid)
{
    intmask mask = disable();

    /* Check arguments */

    if (poolid < 0 || poolid >= nbpools)
    {
        restore(mask);
        return (char *)SYSERR;
    }

    struct bpentry * bpptr = &buftab[poolid];

    /* Wait for pool to have > 0 buffers and allocate a buffer */

    wait(bpptr->bpsem);
    struct bpentry * bufptr = bpptr->bpnext;

    /* Unlink buffer from pool */

    bpptr->bpnext = bufptr->bpnext;

    /* Record pool ID in first four bytes of buffer	and skip */

    *(bpid32 *)bufptr = poolid; //this hidden info is used to release the buffer to the pool
    bufptr = (struct bpentry *)(sizeof(bpid32) + (char *)bufptr); //advance
    restore(mask);
    return (char *)bufptr;
}