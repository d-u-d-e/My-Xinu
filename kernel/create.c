#include <kernel.h>
#include <process.h>
#include <conf.h>

#define	roundew(x)	( (x+3) & ~0x3)

extern char * getstk(uint32 nbytes);
extern void userret(void);

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */

static pid32 newpid(void)
{
    uint32	i;	/* iterate through all processes */
    static pid32 nextpid = 1; /* position in table to try or */
					/*  one beyond end of table	*/
    
    for (i = 0; i < NPROC; i++){
        nextpid %= NPROC;
        if (proctab[nextpid].prstate == PR_FREE){
            return nextpid++;
        }
        else{
            nextpid++;
        }
    }
    return (pid32)SYSERR;
}

pid32 create(
    void *  procaddr, /* procedure address */
    uint32  ssize, /* stack size in bytes */
    pri16   priority, /* process priority > 0 */
    char *  name, /* name (for debugging) */
    uint32  nargs, /* number of args that follow */
    ...
    )
{
    intmask mask;
    pid32 pid;
    struct procent * prptr;
    int32 i;
    uint32 *a;
    uint32 *saddr;

    mask = disable();
    if (ssize < MINSTK)
        ssize = MINSTK;

    ssize = (uint32)roundew(ssize);
    if (((saddr = (uint32 *)getstk(ssize)) == NULL) || 
        (pid = newpid()) == SYSERR || priority < 1){
        restore(mask);
        return SYSERR;
    }

    prcount++;
    prptr = &proctab[pid];

    /* initialize process table entry for new process */

    prptr->prstate = PR_SUSP;
    prptr->prprio = priority;
    prptr->prstkbase = (char *)saddr;
    prptr->prstklen = ssize;
    prptr->prname[PNMLEN - 1] = NULLCH;
    for (i = 0; i < PNMLEN - 1 && (prptr->prname[i] = name[i]) != NULLCH; i++);
    prptr->prsem = -1;
    prptr->prparent = (pid32)getpid();
    prptr->prhasmsg = FALSE;
    
    /* set up initial device descriptors for the shell		*/
	prptr->prdesc[0] = CONSOLE;	/* stdin  is CONSOLE device	*/
	prptr->prdesc[1] = CONSOLE;	/* stdout is CONSOLE device	*/
	prptr->prdesc[2] = CONSOLE;	/* stderr is CONSOLE device	*/

    /* Initialize stack as if the process was called		*/
    *saddr = STACKMAGIC;
    /* push arguments */

    a = (uint32 *)(&nargs + 1); /* start of args */
    a += nargs - 1; /* last argument */

    /* ARM calling convention: r0-r3 are used for parameters;
     * someone needs to set r0-r3 in order to pass the first four parameters
     * to the procedure; ctxsw does the job, if called by another process!
     */

    for (; nargs > 4; nargs--)
        *--saddr = *a--;

    /* this will cause ctxsw to mov procaddr into pc; the final stack associated to the 
     * procedure will contain args from the fifth onwards! 
     */
    *--saddr = (uint32)procaddr; /* return address */

    /* ctxsw pushes r0-r11 (32-bit registers), lr, lr, cpsr */

    for (i = 11; i >= 4; i--)
        *--saddr = 0; /* r11 down to r4 are set to 0 */
    /* next, if a register is used to pass parameters, save them on stack;
     * they will be restored by ctxsw as said above!
    */
    for (i = 4; i > 0; i--){ /* next r3, r2, r1, r0 */
		if(i <= nargs)
			*--saddr = *a--;
		else
			*--saddr = 0;
    }

    *--saddr = (uint32)INITRET;	/* push on return address to return after procedure (this will set lr!)*/
    *--saddr = (uint32)0x00000053;	/* CPSR F bit set, SVC mode */
    prptr->prstkptr = (char *)saddr;
    restore(mask);
    return pid;
}