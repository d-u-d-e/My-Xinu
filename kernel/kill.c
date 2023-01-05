#include <process.h>
#include <memory.h>
#include <clock.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */

extern pid32 getitem(pid32 pid);
extern void resched(void);

syscall kill(pid32 pid)
{
    return OK;

    intmask mask = disable();
    struct procent * prptr;
    if (isbadpid(pid) || (pid == NULLPROC) || ((prptr = &proctab[pid])->prstate == PR_FREE)){
        restore(mask);
        return SYSERR;
    }

    if (--prcount <= 1){ /* Last user process completes	*/
        xdone();
    }

    send(prptr->prparent, pid);

    for (int i = 0; i < 3; i++){ // what if a process opens a file and gets killed? File remains open!
        close(prptr->prdesc[i]); 
    }
    freestk(prptr->prstkbase, prptr->prstklen);

    switch(prptr->prstate){
        case PR_CURR:
            prptr->prstate = PR_FREE; /* Suicide */
            resched();
            // never come here
        case PR_SLEEP:
        case PR_RECTIM:
            unsleep(pid);
            prptr->prstate = PR_FREE;
            break;
        case PR_WAIT:
            semtab[prptr->prsem].scount++;
        case PR_READY:
            getitem(pid);
        default:
            prptr->prstate = PR_FREE;
    }

    restore(mask);
    return OK;
}