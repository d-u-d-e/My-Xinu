#include <process.h>
#include <resched.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */

umsg32 receive(void)
{
    intmask mask = disable();

    struct procent * prptr = &proctab[currpid];
    if (prptr->prhasmsg == FALSE){
        prptr->prstate = PR_RECV;
        resched(); /* Block until message arrives */
    }
    umsg32 msg = prptr->prmsg; /* Retrieve message */
    prptr->prhasmsg = FALSE; /* Reset message flag */
    restore(mask);
    return msg;
}