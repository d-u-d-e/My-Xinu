#include <kernel.h>
#include <process.h>
#include <clock.h>
#include <resched.h>

/*------------------------------------------------------------------------
 *  recvtime  -  Wait specified time to receive a message and return
 *------------------------------------------------------------------------
 */
umsg32 recvtime(int32 maxwait)
{
    intmask mask;

    if (maxwait < 0){
        return SYSERR;
    }

    mask = disable();

    /* Schedule wakeup and place process in timed-receive state */

    struct procent * prptr = &proctab[currpid];
    if (prptr->prhasmsg == FALSE){ /* Delay if no message waiting */
        if (insertd(currpid, sleepq, maxwait) == SYSERR){
            restore(mask);
            return SYSERR;
        }
        prptr->prstate = PR_RECTIM;
        resched();
    }

    /* Either message arrived or timer expired */

    umsg32 msg;
    if (prptr->prhasmsg){
        msg = prptr->prmsg;
        prptr->prhasmsg = FALSE;
    }
    else{
        msg = TIMEOUT;
    }
    restore(mask);
    return msg;
}