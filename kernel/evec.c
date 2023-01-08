#include <kernel.h>
#include <interrupt.h>
#include <resched.h>

char expmsg1[] = "Unhandled exception. Link Register: 0x%x";
char expmsg2[] = "**** EXCEPTION ****";

uint32 intc_vector[128];	/* Interrupt vector	*/

/*------------------------------------------------------------------------
 * initintc - Initialize the Interrupt Controller
 *------------------------------------------------------------------------
 */
int32 initintc()
{
	struct	intc_csreg *csrptr = (struct intc_csreg *)0x48200000;

	/* Reset the interrupt controller */

	csrptr->sysconfig |= (INTC_SYSCONFIG_SOFTRESET);

	/* Wait until reset is complete */

	while((csrptr->sysstatus & INTC_SYSSTATUS_RESETDONE) == 0);

	return OK;
}

/*-------------------------------------------------------------------------
 * irq_dispatch - call the handler for specific interrupt
 *-------------------------------------------------------------------------
 */
void irq_dispatch()
{
	struct intc_csreg * csrptr = (struct intc_csreg *)0x48200000;
	uint32 xnum;		/* Interrupt number of device	*/
	interrupt (*handler)(); /* Pointer to handler function	*/

	/* Get the interrupt number from the Interrupt controller */
	/* 7 bits are used for the interrupt number; some numbers are reserved */

	xnum = csrptr->sir_irq & 0x7F;

	/* Defer scheduling until interrupt is acknowledged */

	resched_cntl(DEFER_START);

	/* If a handler is set for the interrupt, call it */

	if(intc_vector[xnum]) {
		handler = ( interrupt(*)() )intc_vector[xnum];
		handler(xnum);
	}

	/* Acknowledge the interrupt (enable other IRQs to be received, however still disabled here) */

	csrptr->control |= (INTC_CONTROL_NEWIRQAGR);

	/* Resume scheduling */

	resched_cntl(DEFER_STOP);

	/* To ensure that processes are notified promptly when an I/O operation
	completes and to maintain the scheduling invariant, an interrupt
	handler must reschedule whenever it makes a waiting process ready.
	Page 226 explains why it is safe to reschedule during an interrupt.
	It might seem that an interrupt handler should not be allowed to reschedule because switching to a 
	process that runs with interrupts enabled could start a cascade of further interrupt.

	To understand why rescheduling is safe, consider the series of events leading to a
	call of resched from an interrupt handler. Suppose a process U was running with inter-
	rupts enabled when the interrupt occurred. Interrupt dispatching uses U’s stack to save
	the state, and leaves process U running with interrupts disabled while executing the in-
	terrupt handler. Suppose the handler calls resched, which switches to another process,
	T. After T returns from the context switch, T may be running with interrupts enabled,
	and another interrupt may occur. What prevents an infinite loop where unfinished inter-
	rupts pile up until a stack overflows with interrupt function calls? Recall that each
	process has its own stack. Process U had one interrupt on its stack when it was stopped
	by the context switch. The new interrupt occurs while the processor is executing proc-
	ess T, which means the processor is using T’s stack.
	Consider process U. Before another interrupt can pile up on U’s stack, U must re-
	gain control of the processor and interrupts must be enabled. At the last step before it
	gave up control, U called the scheduler, resched, which called the context switch. AtSec. 12.13
	Rescheduling During An Interrupt
	227
	that point, U was running with interrupts disabled. Therefore, when U regains control
	(i.e., when the scheduler selects U again and performs a context switch), U will begin
	executing with interrupts disabled. That is, U will start executing in the context switch.
	The context switch will return to the scheduler, the scheduler will return to the interrupt
	handler, and the handler will return to the dispatcher.
	During the sequence of returns, U will continue to execute with interrupts disabled
	until the dispatcher returns from the interrupt (i.e., until interrupt processing completes
	and the dispatcher returns to the location at which the original interrupt occurred). So,
	no additional interrupts can occur while process U is executing interrupt code (even
	though an interrupt can occur if U switches to another process and the other process
	runs with interrupts enabled). 
	
	NOTE: The important constraint is: only one interrupt can be in
	progress for a given process at any time. Because only a finite number of processes ex-
	ist in the system at a given time and each process can have at most one outstanding in-
	terrupt, the number of outstanding interrupts is bounded.
	*/

}

/*------------------------------------------------------------------------
 * set_evec - set exception vector to point to an exception handler
 *------------------------------------------------------------------------
 */
int32 set_evec(uint32 xnum, uint32 handler)
{
	struct intc_csreg * csrptr = (struct intc_csreg *)0x48200000;

	uint32 bank;	/* bank number in int controller */
	uint32 mask;	/* used to set bits in bank	*/

	if(xnum > 127) {
		return SYSERR;
	}

	intc_vector[xnum] = handler;

	/* Get the bank number based on interrupt number */

	bank = (xnum/32); /* a bank register contains 32 bits corresponding to just 32 interrupts */

	/* Get the bit inside the bank */

	mask = (0x00000001 << (xnum % 32));

	/* Reset the bit to enable that interrupt number */

	csrptr->banks[bank].mir &= (~mask); /* default mask after reset is 0xFFFFFFFF for each bank */ 

	return OK;
}