#include <am335x_watchdog.h>
#include <am335x_control.h>
#include <conf.h>
#include <uart.h>

extern void initintc(void);

/*------------------------------------------------------------------------
 *  counterinit  -  Initialize the ARM performance counters
 *------------------------------------------------------------------------
 */
void counterinit()
{
	/* Perfomance counters are OPTIONAL in ARMv7, but the following are strongly recomended:
	 * - a cycle counter with the ability to count every cycle or every 64-th cycle
	 * - a number of event counters. ARMv7 provides space for up to 31 counters 
	 * (a 32-bit register holds at most 32 counters, of which 1 is the cycle counter, thus limiting the
	 * number of event counters to 31 at most)
	 * - controls for enabling, resetting counters, flagging overflows (since they are 32-bit wrapping counters)
	 * , enabling interrupts on overflow.
	*/

	/* MCR{cond} coproc, opcode1, Rt, CRn, CRm{, opcode2} */


	/* Program the performance-counter control-register:
	 * Enable all counters: (this register contains the effective number of implemented counters)
	*/
	asm volatile ("MCR p15, 0, %0, c9, c12, 0\n\t" :: "r"
							(0x00000011));

	/* Program the count enable set control-register:
	 * Enable all counters:
	 * (here we enable the two event counters and the cycle counter)
	*/
	asm volatile ("MCR p15, 0, %0, c9, c12, 1\n\t" :: "r"
							(0x80000003));

	/* Program the overflow flag status-register
	 * Clear overflows:
	 * (on reads, this register holds the state of the overflow bit for the cycle counter and each 
	 * implemented event counter, two in our case)
	*/
	asm volatile ("MCR p15, 0, %0, c9, c12, 3\n\t" :: "r"
							(0x80000003));
}


/*------------------------------------------------------------------------
 * platinit - platform specific initialization for BeagleBone Black
 *------------------------------------------------------------------------
 */
void platinit(void)
{
	struct uart_csreg * uptr;	/* Address of UART's CSRs	*/
	struct watchdog_csreg * wdtptr;	/* Watchdog registers		*/

	/* Disable the watchdog timer */

	wdtptr = (struct watchdog_csreg *)WDTADDR;
	wdtptr->wspr = 0x0000aaaa;
	while(wdtptr->wwps & 0x00000010);
	wdtptr->wspr = 0x00005555;
	while(wdtptr->wwps & 0x00000010);

	/* Initialize the Interrupt Controller */

	initintc();

	/* Initialize the Performance Counters */

	counterinit();

	/* Pad control for CONSOLE */

	am335x_padctl(UART0_PADRX_ADDR,
			AM335X_PADCTL_RXTX | UART0_PADRX_MODE);
	am335x_padctl(UART0_PADTX_ADDR,
			AM335X_PADCTL_TX | UART0_PADTX_MODE);

	/* Reset the UART device */

	uptr = (struct uart_csreg *)devtab[CONSOLE].dvcsr;
	uptr->sysc |= UART_SYSC_SOFTRESET;
	while((uptr->syss & UART_SYSS_RESETDONE) == 0);
}