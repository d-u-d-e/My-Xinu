#include <semaphore.h>
#include <queue.h>

/*------------------------------------------------------------------------
 *  signal  -  Signal a semaphore, releasing a process if one is waiting
 *------------------------------------------------------------------------
 */

extern status ready(pid32 pid);

syscall signal(sid32 sem)
{
    intmask mask;
    struct sentry * semptr;

    if (isbadsem(sem)){
        return SYSERR;
    }

    mask = disable();
    semptr = &semtab[sem];
    if (semptr->sstate == S_FREE){
        restore(mask);
        return SYSERR;
    }
    if ((semptr->scount++) < 0){ /* Release a waiting process */
        ready(dequeue(semptr->squeue));
    }
    restore(mask);
    return OK;
}