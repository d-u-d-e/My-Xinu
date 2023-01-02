#include <kernel.h>
#include <process.h>
#include <clock.h>

#define	MAXSECONDS	2147483		/* Max seconds per 32-bit msec	*/

/*------------------------------------------------------------------------
 *  sleepms  -  Delay the calling process n milliseconds
 *------------------------------------------------------------------------
 */

extern syscall yield(void);
extern status insertd(pid32 pid, qid16 q, int32 key);
extern void resched(void);

syscall sleepms(int32 delay) /* Time to delay in ms */
{
    intmask mask;

    if (delay < 0){
        return SYSERR;
    }

    if (delay == 0){
        yield();
        return OK;
    }

    /* Delay calling process */

    mask = disable();
    if (insertd(currpid, sleepq, delay) == SYSERR){
        restore(mask);
        return SYSERR;
    }

    proctab[currpid].prstate = PR_SLEEP;
    resched();
    restore(mask);
    return OK;
}

/*------------------------------------------------------------------------
 *  sleep  -  Delay the calling process n seconds
 *------------------------------------------------------------------------
 */

syscall sleep(int32 delay) /* Time to delay in seconds	*/
{ 
    if (delay < 0 || delay > MAXSECONDS){
        return SYSERR;
    }
    
    sleepms(1000 * delay);
    return OK;
}