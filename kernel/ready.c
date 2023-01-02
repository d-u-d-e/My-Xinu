#include <kernel.h>
#include <process.h>
#include <resched.h>

qid16 readylist;	/* Index of ready list */

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */

extern status insert(pid32 pid, qid16 q, int32 key);

status ready(pid32 pid)
{
    register struct procent * prptr;
    if (isbadpid(pid)){
        return SYSERR;
    }

    prptr = &proctab[pid];
    prptr->prstate = PR_READY;
    insert(pid, readylist, prptr->prprio);
    resched();
    return OK;
}