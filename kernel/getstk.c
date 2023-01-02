#include <kernel.h>
#include <memory.h>

/*------------------------------------------------------------------------
 *  getstk  -  Allocate stack memory, returning highest word address
 *------------------------------------------------------------------------
 */

char * getstk(uint32 nbytes)
{
    intmask mask;
    struct memblk * prev, * curr;	/* Walk through memory list	*/
	struct memblk * fits, * fitsprev; /* Record block that fits	*/

    mask = disable();
    if (nbytes == 0){
        restore(mask);
        return NULL;
    }

    nbytes = (uint32) roundmb(nbytes);

    prev = &memlist;
    curr = memlist.mnext;
    fits = NULL;
    fitsprev = NULL;

    while (curr != NULL){ /* Scan entire list */
        if (curr->mlength >= nbytes){ /* Record block address */
            fits = curr; /* when request fits */
            fitsprev = prev;
        }
        prev = curr;
        curr = curr->mnext;
    }

    if (fits == NULL){ /* No block was found */
        restore(mask);
        return NULL;
    }
    else if (nbytes == fits->mlength){ /* Block is exact match	*/
        fitsprev->mnext = fits->mnext;
    }
    else{ /* Remove top section	*/
        fits->mlength -= nbytes;
        fits = (struct memblk *)((uint32)fits + fits->mlength);
    }
    memlist.mlength -= nbytes;
    restore(mask);
    return (char *)((uint32)fits + nbytes - sizeof(uint32)); /* stack begins at highest address */
}