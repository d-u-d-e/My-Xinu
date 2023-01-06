#include <bufpool.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  freebuf  -  Free a buffer that was allocated from a pool by getbuf
 *------------------------------------------------------------------------
 */

syscall freebuf(char * bufaddr)
{
    intmask mask = disable();

    /* Extract pool ID from integer prior to buffer address */

    bufaddr -= sizeof(bpid32);
    bpid32 poolid = *(bpid32 *)bufaddr;
    if (poolid < 0 || poolid >= nbpools){
        restore(mask);
        return SYSERR;
    }

    /* Get address of correct pool entry in table */

    struct bpentry * bpptr = &buftab[poolid];

    /* Insert buffer into list and signal semaphore */

    ((struct bpentry *)bufaddr)->bpnext = bpptr->bpnext;

    bpptr->bpnext = (struct bpentry *)bufaddr;
    signal(bpptr->bpsem);
    restore(mask);
    return OK;
}