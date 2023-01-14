#include <kernel.h>
#include <resched.h>
#include <process.h>
#include <semaphore.h>
#include <conf.h>
#include <memory.h>
#include <queue.h>
#include <bufpool.h>
#include <clock.h>
#include <lib.h>
#include <net.h>
#include <stdio.h>

struct memblk memlist;	/* List of free memory blocks */

/* Active system status */

int	prcount; /* Total number of live processes	*/
pid32 currpid; /* ID of currently executing process	*/


/* Declarations of major kernel variables */
struct procent proctab[NPROC];	/* Process table */
struct sentry semtab[NSEM];	/* Semaphore table			*/

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

/* Control sequence to reset the console colors and cursor position.
 * These are understood by terminals and terminals emulators.
 * '\033[' is the control sequence; '0m' resets attributes; '2J' clears entire screen;
 * ';' defaults to '1;1' and sets cursor at row 1, column 1
 * see https://en.wikipedia.org/wiki/ANSI_escape_code for details
 */

#define	CONSOLE_RESET	" \033[0m\033[2J\033[;H"

extern syscall kprintf(char *fmt, ...);
extern syscall init(did32 descrp);
extern pri16 resume(pid32 pid);
extern pid32 create(
    void *  procaddr,
    uint32  ssize, 
    pri16   priority, 
    char *  name, 
    uint32  nargs, ...);


extern void platinit(void);

/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */

static void sysinit()
{
	int32 i;
	struct procent * prptr;	/* Ptr to process table entry	*/
	struct sentry * semptr;	/* Ptr to semaphore table entry	*/

	/* Platform Specific Initialization */
	platinit();

	/* Reset the console */
	kprintf(CONSOLE_RESET);
	kprintf("\n%s\n\n", VERSION);

	/* Initialize the interrupt vectors */
	initevec();

	/* Initialize free memory list */
	meminit();

	/* Count the Null process as the first process in the system */
	prcount = 1;

	/* Scheduling is not currently blocked */
	Defer.ndefers = 0;

	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
	}

	/* Initialize the Null process entry */	

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->prprio = 0;
	strncpy(prptr->prname, "prnull", 7);
	/* This will return a stack located where the current stack resides!
	 * But it's okay since the current stack is no longer needed later!
	*/
	prptr->prstkbase = getstk(NULLSTK); 
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	currpid = NULLPROC;

	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readylist = newqueue();

	/* Initialize the real time clock */

	clkinit();

	for (i = 0; i < NDEVS; i++) {
		init(i);
	}
	return;
}

/*------------------------------------------------------------------------
 *
 * startup  -  Finish startup takss that cannot be run from the Null
 *		  process and then create and resume the main process
 *
 *------------------------------------------------------------------------
 */

extern process main(void);
extern uint32 getlocalip(void);

static process startup(void)
{
	/* DO STUFF THAT CAN'T BE DONE FROM NULL PROCESS */
	/* remember that the null process cannot cause itself to be */
	/* dequed from the ready queue */

	/* Use DHCP to obtain an IP address and format it */

	uint32 ipaddr;
	char str[128];

	ipaddr = getlocalip();
	if ((int32)ipaddr == SYSERR){
		kprintf("Cannot obtain an IP address\n");
	}
	else{
		/* Print the IP in dotted decimal and hex */
		ipaddr = NetData.ipucast;
		sprintf(str, "%d.%d.%d.%d", (ipaddr >> 24) & 0xFF, (ipaddr >> 16) & 0xFF, 
			(ipaddr >> 8) & 0xFF, ipaddr & 0xFF);
		kprintf("Obtained IP address %s  (0x%08x)\n", str, ipaddr);
	}


	/* Create a process to execute function main() */

	resume(create((void *)main, INITSTK, INITPRIO,
					"Main process", 0, NULL));

	/* Startup process exits at this point */

	return OK;
}

extern void net_init(void);

void nulluser()
{	
	struct memblk * memptr;	/* Ptr to memory block		*/
	uint32 free_mem;		/* Total amount of free memory	*/
	
	/* Initialize the system */

	sysinit();

	/* Output Xinu memory layout */
	free_mem = 0;

	for (memptr = memlist.mnext; memptr != NULL; memptr = memptr->mnext){
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory. Free list:\n", free_mem);
	for (memptr = memlist.mnext; memptr != NULL; memptr = memptr->mnext){
		kprintf("			[0x%08X to 0x%08X]\n", 
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
			(uint32)&etext - (uint32)&text);
	kprintf("			[0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Enable interrupts */
	enable();

	/* Initialize the network stack and start processes */
	net_init();

	/* Create a process to finish startup and start main */

	resume(create((void *)startup, INITSTK, INITPRIO, 
		"Startup process", 0, NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/* something to run when no other process is ready to execute)	*/

	while (TRUE) {
		/* Loop until there is an external interrupt */
		;
	}
}