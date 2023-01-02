#include <kernel.h>
#include <process.h>
#include <queue.h>

/*------------------------------------------------------------------------
 *  insert  -  Insert a process into a queue in descending key order
 *------------------------------------------------------------------------
 */

status insert(pid32 pid, qid16 q, int32 key)
{
    if (isbadpid(pid) || isbadqid(q)){
        return SYSERR;
    }

    qid16 curr, prev;
    curr = firstid(q); 
    /* if curr is the tail (i.e. empty queue), next while loop will exit immediately
     * because the "key" of the tail is always smaller than the minimum key value
     */
    while (queuetab[curr].qkey >= key){
        curr = queuetab[curr].qnext;
    }

    /* Insert process between curr node and previous node */

    prev = queuetab[curr].qprev; /* Get index of previous node	*/
    queuetab[pid].qnext = curr;
    queuetab[pid].qprev = prev;
    queuetab[pid].qkey = key;
    queuetab[prev].qnext = pid;
    queuetab[curr].qprev = pid;
    return OK;
}