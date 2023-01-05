#pragma once

#include <kernel.h>

/* in file xsh_ls.c */
shellcmd xsh_ls (int, char *[]);
shellcmd xsh_exit (int, char *[]);
shellcmd xsh_kill(int nargs, char * args[]);
shellcmd xsh_ps(int nargs, char * args[]);
shellcmd xsh_cat(int nargs, char * args[]);

status addargs(pid32 pid, int32 ntok, int32 tok[], 
        int32 tlen, char * tokbuf, void * dummy);