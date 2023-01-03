#include <shell.h>
#include <lib.h>
#include <stdio.h>

/************************************************************************/
/* Table of Xinu shell commands and the function associated with each	*/
/************************************************************************/


/************************************************************************/
/* shell  -  Provide an interactive user interface that executes	*/
/*	     commands.  Each command begins with a command name, has	*/
/*	     a set of optional arguments, has optional input or		*/
/*	     output redirection, and an optional specification for	*/
/*	     background execution (ampersand).  The syntax is:		*/
/*									*/
/*		   command_name [args*] [redirection] [&]		*/
/*									*/
/*	     Redirection is either or both of:				*/
/*									*/
/*				< input_file				*/
/*			or						*/
/*				> output_file				*/
/*									*/
/************************************************************************/

process shell(did32 dev) /* ID of tty device from which	to accept commands */
{
    /* Print shell banner and startup message */

    int32 lexan(char * line, int32 len, char * tokbuf, int32 * tlen, int32 tok[], int32 toktyp[]);

    fprintf(dev, "\n\n%s%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
    SHELL_BAN0,SHELL_BAN1,SHELL_BAN2,SHELL_BAN3,SHELL_BAN4,
    SHELL_BAN5,SHELL_BAN6,SHELL_BAN7,SHELL_BAN8,SHELL_BAN9);

    fprintf(dev, "%s\n\n", SHELL_STRTMSG);

    /* Continually prompt the user, read input, and execute command	*/

    int32 tlen, len;

    char buf[SHELL_BUFLEN];

    while (TRUE){
        fprintf(dev, SHELL_PROMPT);
        len = read(dev, buf, sizeof(buf));

        /* Exit gracefully on end-of-file */

        if (len == EOF){
            break;
        }

        /* If line contains only NEWLINE, go to next line */

        if (len <= 1){
            continue;
        }

        buf[len] = SH_NEWLINE; /* terminate line */
        
        char tokbuf[SHELL_BUFLEN + SHELL_MAXTOK]; // + null for each token
        int32 tok[SHELL_MAXTOK];
        int32 toktyp[SHELL_MAXTOK];
        int32 ntok = lexan(buf, len, tokbuf, &tlen, tok, toktyp);

        /* Handle parsing error */

        if (ntok == SYSERR){
            fprintf(dev, "%s\n", SHELL_SYNERRMSG);
            continue;
        }

        /* If line is empty, go to next input line */

        if (ntok == 0){
            fprintf(dev, "\n");
            continue;
        }

        /* If last token is '&', set background */

        bool8 backgnd = FALSE;

        if (toktyp[ntok - 1] == SH_TOK_AMPER){
            ntok--;
            tlen -= 2;
            backgnd = TRUE;
        }

        /* Check for input/output redirection (default is none) */

        /* Verify remaining tokens are type "other" */

        /* Lookup first token in the command table */

        /* Handle command not found */

        /* Handle built-in command */

        /* Open files and redirect I/O if specified */

        /* Spawn child thread for non-built-in commands */

        /* If creation or argument copy fails, report error */

        /* Set stdinput and stdoutput in child to redirect I/O */

    }

    /* Terminate the shell process by returning from the top level */

    fprintf(dev, SHELL_EXITMSG);
    return OK;
}

