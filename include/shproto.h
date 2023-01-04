#pragma once

#include <kernel.h>

/* in file xsh_ls.c */
extern shellcmd xsh_ls (int32, char *[]);
status addargs(pid32 pid, int32 ntok, int32 tok[], 
        int32 tlen, char * tokbuf, void * dummy);