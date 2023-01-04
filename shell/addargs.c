#include "shproto.h"
#include <process.h>
#include <lib.h>

/*------------------------------------------------------------------------
 *  addargs  -  Add local copy of argv-style arguments to the stack of
 *		  a command process that has been created by the shell
 *------------------------------------------------------------------------
 */

status addargs(pid32 pid, int32 ntok, int32 tok[], 
        int32 tlen, char * tokbuf, void * dummy)
{
    intmask mask = disable();

    /* Check argument count and data length */

    if (ntok <= 0 || tlen < 0){
        restore(mask);
        return SYSERR;
    }

    struct procent * prptr = &proctab[pid];

    /* Compute lowest location in the process stack where the	*/
	/*	args array will be stored followed by the argument	*/
	/*	strings */

    uint32 aloc = (uint32) (prptr->prstkbase - prptr->prstklen 
    + sizeof(uint32)); /* Argument location in process stack as an int */

    /* Location in process's stack to place args vector */
    uint32 * argloc = (uint32 *)((aloc + 3) & ~0x3); /* round multiple of 4 */

    /* Compute the first location beyond args array for the strings	*/

    char * argstr = (char *)(argloc + (ntok + 1)); /* +1 for a null ptr	*/

    /* Set each location in the args vector to be the address of	*/
	/* string area plus the offset of this argument		*/

    uint32 * aptr; int32 i;
    for (aptr = argloc, i = 0; i < ntok; i++){
        *aptr++ = (uint32)(argstr + tok[i]);
    }
    
    /* Add a null pointer to the args array */
    
    *aptr++ = (uint32)NULL;

    /* Copy the argument strings from tokbuf into process's	stack	*/
	/* just beyond the args vector	*/

    memcpy(aptr, tokbuf, tlen);

    /* Find the second argument in process's stack */

    uint32 * search; /* pointer that searches for dummy argument on stack */

    for (search = (uint32 *)prptr->prstkptr; search < (uint32 *)prptr->prstkbase; search++){
        /* If found, replace with the address of the args vector */

        if (*search == (uint32)dummy){
            *search = (uint32)argloc;
            restore(mask);
            return OK;
        }
    }

    /*--------------------*/ //highest base
    /*--dummy <- argloc---*/// this is basically char * argv[], a pointer to an array of pointers, null terminated
    /*--------....--------*/
    /*--------NULL--------*/
    /*--------TOKN-char---*/
    /*--------....--------*/
    /*--------TOKN-char---*/// <------ 
    /*--------....--------*///       |
    /*--------NULL--------*///       |
    /*--------TOK1-char---*///       |
    /*--------....--------*///       |
    /*--------TOK1-char---*/// <--|  |
    /*--------NULL--------*///    |  |
    /*------TOKN-PTR------*/// ---|--/
    /*--------....--------*///    |
    /*------TOK1-PTR------*/// ---/ end lowest multiple of 4 == argloc

    /* Argument value not found on the stack - report an error */

    restore(mask);
    return SYSERR;
}