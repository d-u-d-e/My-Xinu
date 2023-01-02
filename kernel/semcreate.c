#include <kernel.h>
#include <semaphore.h>

static sid32 newsem(void);

/*------------------------------------------------------------------------
 *  semcreate  -  Create a new semaphore and return the ID to the caller
 *------------------------------------------------------------------------
 */

sid32 semcreate(int32 count)
{
    intmask mask;
    sid32 sem;
    mask = disable();
    if (count < 0 || ((sem = newsem()) == SYSERR)){
        restore(mask);
        return SYSERR;
    }
    semtab[sem].scount = count;
    restore(mask);
    return sem;
}   


/*------------------------------------------------------------------------
 *  newsem  -  Allocate an unused semaphore and return its index
 *------------------------------------------------------------------------
 */

static sid32 newsem(void)
{
    static sid32 nextsem = 0;
    sid32 sem;
    int32 i;

    for (i = 0; i < NSEM; i++){
        sem = nextsem++;
        if (nextsem >= NSEM){
            nextsem = 0;
        }
        if (semtab[sem].sstate == S_FREE){
            semtab[sem].sstate = S_USED;
            return sem;
        }
    }
    return SYSERR;
}