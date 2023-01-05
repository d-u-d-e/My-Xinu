#include <process.h>
#include <queue.h>

/*------------------------------------------------------------------------
 *  unsleep  -  Internal function to remove a process from the sleep
 *		    queue prematurely.  The caller must adjust the delay
 *		    of successive processes.
 *------------------------------------------------------------------------
 */

extern pid32 getitem(pid32 pid);

status unsleep(pid32 pid)
{
    struct procent * prptr;

    intmask mask = disable();

    if (isbadpid(pid)){
        return SYSERR;
    }

    /* Verify that candidate process is on the sleep queue */

    prptr = &proctab[pid];
    if ((prptr->prstate != PR_SLEEP) && (prptr->prstate != PR_RECTIM)){
        restore(mask);
        return SYSERR;
    }

    /* Increment delay of next process if such a process exists */

    pid32 pidnext = queuetab[pid].qnext;
    if (pidnext < NPROC){
        queuetab[pidnext].qkey += queuetab[pid].qkey;
    }

    getitem(pid); /* Unlink process from queue */
    restore(mask);
    return OK;
}