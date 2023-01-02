#include <kernel.h>
#include <process.h>

/*------------------------------------------------------------------------
 *  resume  -  Unsuspend a process, making it ready
 *------------------------------------------------------------------------
 */

extern status ready(pid32 pid);

pri16 resume(pid32 pid)
{
    intmask mask;
    struct procent * prptr;
    pri16 prio;

    mask = disable();
    if (isbadpid(pid)){
        restore(mask);
        return (pri16)SYSERR;
    }
    prptr = &proctab[pid];
    if (prptr->prstate != PR_SUSP){
        restore(mask);
        return (pri16)SYSERR;
    }
    prio = prptr->prprio; /* Record priority to return */
    ready(pid);
    restore(mask);
    return prio;
}