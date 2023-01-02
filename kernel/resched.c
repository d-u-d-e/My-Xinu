#include <resched.h>
#include <process.h>
#include <queue.h>
#include <clock.h>

struct defer Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 * Assumes interrupts are disabled
 */

extern status insert(pid32 pid, qid16 q, int32 key);
extern void ctxsw(char **, char **);

void resched(void)
{
    struct procent * ptold; /* ptr to table entry for old process */
    struct procent * ptnew; /* ptr to table entry for new process */

    /* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

    ptold = &proctab[currpid];

    if (ptold->prstate == PR_CURR){
        if (ptold->prprio > firstkey(readylist)){
            return;
        }

        /* Old process will no longer remain current */

        ptold->prstate = PR_READY;
        insert(currpid, readylist, ptold->prprio);
    }

    /* Force context switch to highest priority ready process */

    currpid = dequeue(readylist);
    ptnew = &proctab[currpid];
    ptnew->prstate = PR_CURR;
    preempt = QUANTUM; /* Reset time slice for process	*/

    ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

    /* Old process returns here when resumed */
    return; 
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */

status resched_cntl(int32 defer)
{
    switch (defer)
    {
    case DEFER_START:
        if (Defer.ndefers++ == 0){
            Defer.attempt = FALSE;
        }
        return OK;
    case DEFER_STOP:
        if (Defer.ndefers <= 0){
            return SYSERR;
        }
        if ((--Defer.ndefers == 0) && Defer.attempt){
            resched();
        }
        return OK;
    default:
        return SYSERR;
    }
}

