#include <name.h>

/*------------------------------------------------------------------------
 *  mount  -  Add a prefix mapping to the name space
 *------------------------------------------------------------------------
 */

syscall mount(char * prefix, char * replace, did32 device){

    int mask = disable();

    int32 psiz = namlen(prefix, NM_PRELEN);
    int32 rsiz = namlen(replace, NM_REPLLEN);

    /* Check for table overflow */

    if (nnames >= NNAMES){
        kprintf("namespace: overflow\n");
        restore(mask);
        return SYSERR;
    }

    /* If arguments are invalid or table is full, return error */

    if (psiz == SYSERR || rsiz == SYSERR){
        restore(mask);
        return SYSERR;
    }

    /* Allocate a slot in the table */

    struct nmentry * namptr = &nametab[nnames]; /* Next unused entry in table */

    /* copy prefix and replacement strings and record device ID */

    for (int32 i = 0; i < psiz; i++){
        namptr->nprefix[i] = *prefix++;
    }

    for (int32 i = 0; i < rsiz; i++){
        namptr->nreplace[i] = *replace++;
    }

    namptr->ndevice = device;
    nnames++;

    restore(mask);
    return OK;
}


/*------------------------------------------------------------------------
 *  namlen  -  Compute the length of a string stopping at maxlen
 *------------------------------------------------------------------------
 */

int32 namlen(char * name, int32 maxlen)
{
    int32 i;

    for (i = 0; i < maxlen; i++){
        if (*name++ == NULLCH){
            return i + 1; /* Include NULLCH in length */
        }
    }
    return SYSERR;
}