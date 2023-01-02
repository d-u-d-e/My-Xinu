#include <kernel.h>

/*------------------------------------------------------------------------
 *  yield  -  Voluntarily relinquish the CPU (end a timeslice)
 *------------------------------------------------------------------------
 */

extern void resched(void);

syscall yield(void)
{
    intmask mask;
    mask = disable();
    resched();
    restore(mask);
    return OK;
}