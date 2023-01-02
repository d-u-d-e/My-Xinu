#include <kernel.h>
#include <queue.h>
#include <process.h>

/*------------------------------------------------------------------------
 *  insertd  -  Insert a process in delta list using delay as the key
 *------------------------------------------------------------------------
 */

status insertd(pid32 pid, qid16 q, int32 key)
{
    int32 next, prev;

    if (isbadqid(q) || isbadpid(pid)){
        return SYSERR;
    }

    prev = queuehead(q);
    next = queuetab[queuehead(q)].qnext;
    while ((next != queuetail(q)) && (queuetab[next].qkey <= key)){
        key -= queuetab[next].qkey;
        prev = next;
        next = queuetab[next].qnext;
    }

    /* Insert new node between prev and next nodes */

    queuetab[pid].qprev = prev;
    queuetab[pid].qnext = next;
    queuetab[prev].qnext = pid;
    queuetab[next].qprev = pid;
    if (next != queuetail(q)) {
        queuetab[next].qkey -= key;
    }

    return OK;
}