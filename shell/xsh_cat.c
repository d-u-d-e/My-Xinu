#include <kernel.h>
#include <lib.h>
#include <process.h>
#include <stdio.h>
#include <conf.h>

/*------------------------------------------------------------------------
 * xsh_cat - shell command to cat one or more files
 *------------------------------------------------------------------------
 */

shellcmd xsh_cat(int nargs, char * args[])
{
	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s [file...]\n\n", args[0]);
		printf("Description:\n");
		printf("\twrites contents of files or stdin to stdout\n");
		printf("Options:\n");
		printf("\tfile...\tzero or more file names\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

    int32 nextch;
    if (nargs == 1) {
        nextch = getc(stdin);
        while (nextch != EOF) {
            putc(stdout, nextch);
            nextch = getc(stdin);
        }
        return 0;
	}

    char * argptr;
    did32 descr;
    for (int i = 1; i < nargs; i++){
        argptr = args[i];
        if ((argptr[0] == '-') && (argptr[1] == NULLCH))
            descr = stdin;
        else{
            descr = open(NAMESPACE, argptr, "ro");
            if (descr == (did32)SYSERR){
				fprintf(stderr, "%s: cannot open file %s\n",
					args[0], argptr);
				return 1;   
            }
        }
        nextch = getc(descr);
        while (nextch != EOF){
            putc(stdout, nextch);
            nextch = getc(descr);
        }
        close(descr);
    }
    return 0;
}