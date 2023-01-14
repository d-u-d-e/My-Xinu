#include <shell.h>
#include <lib.h>
#include <stdio.h>
#include "shproto.h"
#include <name.h>
#include <file.h>

/************************************************************************/
/* Table of Xinu shell commands and the function associated with each	*/
/************************************************************************/
const struct cmdent	cmdtab[] = {
    {"ls", FALSE, xsh_ls},
    {"exit", TRUE, xsh_exit},
    {"kill", TRUE, xsh_kill},
    {"ps", FALSE, xsh_ps},
    {"cat", FALSE, xsh_cat},
    {"echo", FALSE, xsh_echo},
    {"ping", FALSE, xsh_ping},
    {"clear", TRUE, xsh_clear}
};

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

uint32 ncmd = sizeof(cmdtab) / sizeof(struct cmdent);

extern umsg32 recvclr(void);

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

        char * outname = NULL;
        char * inname = NULL;

        if ((ntok >= 3) && ((toktyp[ntok - 2] == SH_TOK_LESS) || 
            (toktyp[ntok - 2] == SH_TOK_GREATER))){

            if (toktyp[ntok - 1] != SH_TOK_OTHER){
                fprintf(dev, "%s\n", SHELL_SYNERRMSG);
                continue;
            }
            if (toktyp[ntok - 2] == SH_TOK_LESS){
                inname = &tokbuf[tok[ntok - 1]];
            }
            else{
                outname = &tokbuf[tok[ntok - 1]];
            }
            ntok -= 2;
            tlen = tok[ntok];
        }

        if ((ntok >= 3) && ((toktyp[ntok - 2] == SH_TOK_LESS) || 
            (toktyp[ntok - 2] == SH_TOK_GREATER))){

            if (toktyp[ntok - 1] != SH_TOK_OTHER){
                fprintf(dev, "%s\n", SHELL_SYNERRMSG);
                continue;
            }
            if (toktyp[ntok - 2] == SH_TOK_LESS){
                if (inname != NULL){ // can't have < a < b
                    fprintf(dev, "%s\n", SHELL_SYNERRMSG);
                    continue;    
                }
                inname = &tokbuf[tok[ntok - 1]]; // here < a > b
            }
            else{
                if (outname != NULL){ // can't have > a > b
                    fprintf(dev, "%s\n", SHELL_SYNERRMSG);
                    continue;    
                }
                outname = &tokbuf[tok[ntok - 1]]; // here > a < b
            }
            ntok -= 2;
            tlen = tok[ntok];
        }

        /* Verify remaining tokens are type "other" */
        
        int32 i;
        for (i = 0; i < ntok; i++){
            if (toktyp[i] != SH_TOK_OTHER){
                break;
            }
        }
        if ((ntok == 0) || (i < ntok)) {
            fprintf(dev, SHELL_SYNERRMSG);
            continue;
        }

        /* Lookup first token in the command table */

        int32 j;
        char * src, * cmp;
        bool8 diff;
        for (j = 0; j < ncmd; j++){
            src = cmdtab[j].cname;
            cmp = tokbuf;
            diff = FALSE;
            while (*src != NULLCH){
                if (*cmp != *src){
                    diff = TRUE;
                    break;
                }
                src++; cmp++;
            }
            if (diff || (*cmp != NULLCH)){
                continue;
            }
            else{
                break;
            }
        }

        /* Handle command not found */

        if (j >= ncmd){
            fprintf(dev, "command %s not found\n", tokbuf);
            continue;
        }

        /* Handle built-in command */

        char * args[SHELL_MAXTOK];

        if (cmdtab[j].cbuiltin){ /* No background or redirect. */
            if(inname != NULL || outname != NULL || backgnd){
                fprintf(dev, SHELL_BGERRMSG);
                continue;                
            }
            else{
                /* Set up arg vector for call */

                for (i = 0; i < ntok; i++){
                    args[i] = &tokbuf[tok[i]];
                }

                /* Call builtin shell function */

                if ((*cmdtab[j].cfunc)(ntok, args) == SHELL_EXIT){
                    break;
                }
            }
            continue;
        }

        did32 stdinput, stdoutput; /* Descriptors for redirected input / output */
        stdinput = stdoutput = dev;

        /* Open files and redirect I/O if specified */

        if (inname != NULL){
            stdinput = open(NAMESPACE, inname, "ro");
            if (stdinput == SYSERR){
                fprintf(dev, SHELL_INERRMSG, inname);
                continue;
            }
        }
        if (outname != NULL){
            stdoutput = open(NAMESPACE, outname, "w");
            if (stdoutput == SYSERR){
                fprintf(dev, SHELL_OUTERRMSG, outname);
                continue;
            }
            else{
                control(stdoutput, F_CTL_TRUNC, 0, 0);
            }
        }

        /* Spawn child thread for non-built-in commands */

        int32 tmparg;
        pid32 child = create(cmdtab[j].cfunc, SHELL_CMDSTK, SHELL_CMDPRIO, 
            cmdtab[j].cname, 2, ntok, &tmparg);

        /* If creation or argument copy fails, report error */

        if ((child == SYSERR) || 
            (addargs(child, ntok, tok, tlen, tokbuf, &tmparg) == SYSERR)){
                fprintf(dev, SHELL_CREATMSG);
                continue;
        }

        /* Set stdinput and stdoutput in child to redirect I/O */

        proctab[child].prdesc[0] = stdinput;
        proctab[child].prdesc[1] = stdoutput;

        int32 msg = recvclr(); /* Message from receive() for child termination */
        resume(child);
        if (!backgnd){
            msg = receive();
            while (msg != child){
                msg = receive();
            }
        }
    }

    /* Terminate the shell process by returning from the top level */

    fprintf(dev, SHELL_EXITMSG);
    return OK;
}

