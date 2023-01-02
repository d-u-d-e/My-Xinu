#include <kernel.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  semcount  -  Return the count of a semaphore (because any integer is
 *		   possible, return of SYSERR may be ambiguous)
 *------------------------------------------------------------------------
 */

syscall semcount(sid32 semid)
{
    intmask mask;
    int32 count;

    mask = disable();
    if (isbadsem(semid) || semtab[semid].sstate == S_FREE){
        restore(mask);
        return SYSERR;
    }
    count = semtab[semid].scount;
    restore(mask);
    return count;
}