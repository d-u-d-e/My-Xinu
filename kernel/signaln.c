#include <semaphore.h>
#include <resched.h>
#include <queue.h>

/*------------------------------------------------------------------------
 *  signaln  -  Signal a semaphore n times, releasing n waiting processes
 *------------------------------------------------------------------------
 */

extern status ready(pid32 pid);

syscall signaln(sid32 sem, int32 count)
{
    intmask mask = disable();

    if (isbadsem(sem) || (count < 0)){
        restore(mask);
        return SYSERR;
    }

    struct sentry * semptr = &semtab[sem];
    if (semptr->sstate == S_FREE){
        restore(mask);
        return SYSERR;
    }

    resched_cntl(DEFER_START);
    for(; count > 0; count--){
        if ((semptr->scount++) < 0){
            ready(dequeue(semptr->squeue));
        }
    }
    resched_cntl(DEFER_STOP);
    restore(mask);
    return OK;
}