#pragma once

#include <kernel.h>
#include <process.h>
#include <semaphore.h>

/* Default # of queue entries: 1 per process plus 2 for ready list plus	*/
/*			2 for sleep list plus 2 per semaphore		*/

#ifndef NQENT
#define NQENT	(NPROC + 4 + 2 * NSEM)
#endif

#define	EMPTY	(-1)		/* Null value for qnext or qprev index */
#define	MAXKEY	0x8FFFFFFF	/* Max key that can be stored in queue */
#define	MINKEY	0x80000000	/* Min key that can be stored in queue */

struct qentry{		/* One per process plus two per list	*/
	int32	qkey;		/* Key on which the queue is ordered	*/
	qid16	qnext;		/* Index of next process or tail	*/
	qid16	qprev;		/* Index of previous process or head	*/
};

extern struct qentry queuetab[];

/* Inline to check queue id assumes interrupts are disabled */

#define	isbadqid(x)	(((int32)(x) < NPROC) || ((int32)(x) >= NQENT-1))

/* Inline queue manipulation functions */

#define	queuehead(q)	(q)
#define	queuetail(q)	((q) + 1)
#define	firstid(q)	(queuetab[queuehead(q)].qnext)
#define	lastid(q)	(queuetab[queuetail(q)].qprev)
#define	isempty(q)	(firstid(q) >= NPROC)
#define	nonempty(q)	(firstid(q) <  NPROC)
#define	firstkey(q)	(queuetab[firstid(q)].qkey)
#define	lastkey(q)	(queuetab[lastid(q)].qkey)

pid32 dequeue(qid16 q);
pid32 enqueue(pid32 pid, qid16 q);
qid16 newqueue(void);