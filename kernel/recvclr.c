#include <process.h>

/*------------------------------------------------------------------------
 *  recvclr  -  Clear incoming message, and return message if one waiting
 *------------------------------------------------------------------------
 */

umsg32 recvclr(void)
{
    intmask mask;
    struct procent * prptr;
    umsg32 msg;

    mask = disable();
    prptr = &proctab[currpid];
    if (prptr->prhasmsg == TRUE){
        msg = prptr->prmsg;
        prptr->prhasmsg = FALSE;
    }
    else{
        msg = OK;
    }
    restore(mask);
    return msg;
}