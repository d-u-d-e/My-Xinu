#include <process.h>
#include <clock.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */

extern status ready(pid32 pid);

syscall send(pid32 pid, umsg32 msg)
{
    intmask mask = disable();
    struct procent * prptr;

    if (isbadpid(pid)){
        restore(mask);
        return SYSERR;
    }

    prptr = &proctab[pid];
    if (prptr->prhasmsg){
        restore(mask);
        return SYSERR;
    }

    prptr->prmsg = msg;
    prptr->prhasmsg = TRUE;

    /* If recipient waiting or in timed-wait make it ready */

    if (prptr->prstate == PR_RECV){
        ready(pid);
    }
    else if (prptr->prstate == PR_RECTIM){
        unsleep(pid);
        ready(pid);
    }
    restore(mask);
    return OK;
}