#pragma once
#include <kernel.h>

/* Constants and variables related to deferred rescheduling */

#define	DEFER_START	1	/* Start deferred rescehduling		*/
#define	DEFER_STOP	2	/* Stop  deferred rescehduling		*/

struct defer {
	int32 ndefers;	/* Number of outstanding defers 	*/
	bool8 attempt;	/* Was resched called during the	*/
				/*   deferral period? */
};

extern struct defer Defer;

void resched(void);
status resched_cntl(int32 defer);