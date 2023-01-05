#include <kernel.h>
#include <lib.h>
#include <process.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_kill - shell command to kill a process
 *------------------------------------------------------------------------
 */

shellcmd xsh_kill(int nargs, char * args[])
{
    if (nargs > 1 && strncmp(args[1], "--help", 7) == 0){
        printf("Usage: %s PID\n\n", args[0]);
        printf("Description:\n");
        printf("\tterminate a process\n");
        printf("\tthe directory specified by dir\n");
        printf("Options:\n");
        printf("\tPID \tthe ID of a process to terminate\n");
        printf("\t--help\t display this help and exit\n");
        return 0;
    }

    /* Check argument count */

    if (nargs != 2){
        fprintf(stderr, "%s: incorrect argument\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n", args[0]);
        return 1;
    }

    /* compute process ID from argument string */

    char * chptr = args[1];
    char ch = *chptr++;
    pid32 pid = 0;
    while(ch != NULLCH){
        if ((ch < '0') || (ch > '9')){
            fprintf(stderr, "%s: non-digit in process ID\n", args[0]);
            return 1;
        }
        pid = 10 * pid + (ch - '0');
        ch = *chptr++;
    }
    if (pid == 0){
        fprintf(stderr, "%s: cannot kill the null process\n", args[0]);
        return 1;   
    }

    int32 retval = kill(pid);
    if (retval == SYSERR){
        fprintf(stderr, "%s: cannot kill process %d\n", args[0], pid);
        return 1;           
    }
    return 0;
}