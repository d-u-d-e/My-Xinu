#include <kernel.h>
#include <lib.h>
#include <process.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_ps - shell command to print the process table
 *------------------------------------------------------------------------
 */

shellcmd xsh_ps(int nargs, char * args[])
{

    char *pstate[]	= {		/* names for process states	*/
    "free ", "curr ", "ready", "recv ", "sleep", "susp ",
    "wait ", "rtime"};

    if (nargs > 1 && strncmp(args[1], "--help", 7) == 0){
        printf("Usage: %s\n\n", args[0]);
        printf("Description:\n");
        printf("\tdisplay information about running processes\n");
        printf("\tthe directory specified by dir\n");
        printf("Options:\n");
        printf("\t--help\t display this help and exit\n");
        return 0;
    }

    /* Check for valid number of arguments */

    if (nargs > 1){
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n", args[0]);
        return 1;
    }

    /* Print header for items from the process table */

    printf("%3s %-16s %5s %4s %4s %10s %-10s %10s\n",
		   "Pid", "Name", "State", "Prio", "Ppid", "Stack Base",
		   "Stack Ptr", "Stack Size");

	printf("%3s %-16s %5s %4s %4s %10s %-10s %10s\n",
		   "---", "----------------", "-----", "----", "----",
		   "----------", "----------", "----------");
    
    /* Output information for each process */

    struct procent * prptr;
    for (int i = 0; i < NPROC; i++){
        prptr = &proctab[i];
        if (prptr->prstate == PR_FREE){
            continue;
        }
        printf("%3d %-16s %s %4d %4d 0x%08X 0x%08X %8d\n",
			i, prptr->prname, pstate[(int)prptr->prstate],
			prptr->prprio, prptr->prparent, prptr->prstkbase,
			prptr->prstkptr, prptr->prstklen);
    }
    return 0;
}