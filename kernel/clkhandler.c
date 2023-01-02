#include <kernel.h>
#include <clock.h>
#include <queue.h>
#include <resched.h>

/*-----------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *-----------------------------------------------------------------------
 */

extern void wakeup(void);

void clkhandler(void)
{
	volatile struct am335x_timer1ms * csrptr =
			(struct am335x_timer1ms *)0x44E31000;

    /* If there is no interrupt, return */

	if((csrptr->tisr & AM335X_TIMER1MS_TISR_OVF_IT_FLAG) == 0) {
		return;
	}

    /* Acknowledge the interrupt */

	csrptr->tisr = AM335X_TIMER1MS_TISR_OVF_IT_FLAG;

    /* Increment 1000ms counter */
	count1000++;

    /* After 1 sec, increment clktime */

	if(count1000 >= 1000) {
		clktime++;
		count1000 = 0;
	}

    /* check if sleep queue is empty */

    if (!isempty(sleepq)) {
		/* sleepq nonempty, decrement the key of */
		/* topmost process on sleepq */

		if ((--queuetab[firstid(sleepq)].qkey) == 0){
			wakeup(); /* wake up all processes with key == 0 */
		}
	}

    /* Decrement the preemption counter */
	/* Reschedule if necessary */

	if((--preempt) == 0) {
		preempt = QUANTUM;
		resched();
	}
}