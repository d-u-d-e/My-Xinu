#include <semaphore.h>
#include <process.h>
#include <queue.h>
#include <resched.h>

/*------------------------------------------------------------------------
 *  wait  -  Cause current process to wait on a semaphore
 *------------------------------------------------------------------------
 */

syscall wait(sid32 sem)
{
    intmask mask;
    struct procent * prptr;		/* Ptr to process's table entry	*/
	struct sentry * semptr;		/* Ptr to sempahore table entry	*/

    if (isbadsem(sem)){
        return SYSERR;
    }

    mask = disable();
    semptr = &semtab[sem];
    if (semptr->sstate == S_FREE){
        restore(mask);
        return SYSERR;
    }

    if (--(semptr->scount) < 0) {
        prptr = &proctab[currpid];
        prptr->prstate = PR_WAIT;
        prptr->prsem = sem; /* Record semaphore ID */
        enqueue(currpid, semptr->squeue);
        resched();
    }

    restore(mask);
    return OK;
}

