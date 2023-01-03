#include <lfs.h>

/*------------------------------------------------------------------------
 *  lfibclear  --  Clear an in-core copy of an index block
 *------------------------------------------------------------------------
 */

void lfibclear(struct lfiblk * ibptr, int32 offset)
{
    int32 i;

    ibptr->ib_offset = offset; /* Assign specified file offset */
    for (i = 0; i < LF_IBLEN; i++){ /* Clear each data block pointer */
        ibptr->ib_dba[i] = LF_DNULL;
    }
    ibptr->ib_next = LF_INULL; /* Set next ptr to null */
    return;
}