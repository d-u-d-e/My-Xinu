#include <kernel.h>

/*------------------------------------------------------------------------
 * panic  -  Display a message and stop all processing
 *------------------------------------------------------------------------
 */
void panic (char * msg) /* Message to display */
{
	disable();	/* Disable interrupts */
	kprintf("\n\n\rpanic: %s\n\n", msg);
	while(1) {;} /* Busy loop forever */
}