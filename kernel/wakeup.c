#include <kernel.h>
#include <resched.h>
#include <queue.h>
#include <clock.h>

/*------------------------------------------------------------------------
 *  wakeup  -  Called by clock interrupt handler to awaken processes
 *------------------------------------------------------------------------
 */

extern status ready(pid32 pid);

void wakeup(void)
{
    /* Awaken all processes that have no more time to sleep */

    resched_cntl(DEFER_START);
    /* Need to defer rescheduling since a call to ready might reschedule and the handler won't finish */
    while(nonempty(sleepq) && (firstkey(sleepq) <= 0)){
        ready(dequeue(sleepq));
    }
    resched_cntl(DEFER_STOP);
    return;
}