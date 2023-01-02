#include <queue.h>

struct qentry queuetab[NQENT];	/* Table of process queues	*/

/*------------------------------------------------------------------------
 *  enqueue  -  Insert a process at the tail of a queue
 *------------------------------------------------------------------------
 */

pid32 enqueue(pid32 pid, qid16 q)
{
    qid16 tail, prev;

    if (isbadqid(q) || isbadpid(pid)){
        return SYSERR;
    }

    tail = queuetail(q);
    prev = queuetab[tail].qprev;

    /* a process is always in a specific list, not more than one! */
    queuetab[pid].qnext = tail;
    queuetab[pid].qprev = prev;
    queuetab[prev].qnext = pid;
    queuetab[tail].qprev = pid;
    return pid;
}

/*------------------------------------------------------------------------
 *  dequeue  -  Remove and return the first process on a list
 *------------------------------------------------------------------------
 */

extern pid32 getfirst(qid16 q);

pid32 dequeue(qid16 q)
{
    pid32 pid;

    if (isbadqid(q)){
        return SYSERR;
    }
    else if (isempty(q)){
        return EMPTY;
    }

    pid = getfirst(q);
    queuetab[pid].qprev = EMPTY;
    queuetab[pid].qnext = EMPTY;
    return pid;
}