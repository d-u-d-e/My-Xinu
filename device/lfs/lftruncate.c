#include <lfs.h>

/*------------------------------------------------------------------------
 * lftruncate  -  Truncate a file by freeing its index and data blocks
 *			(assumes directory mutex held)
 *------------------------------------------------------------------------
 */

status lftruncate(struct lflcblk * lfptr)
{
    struct ldentry * ldptr; /* Pointer to file's dir. entry	*/

    ldptr = lfptr->lfdirptr;
    if (ldptr->ld_size == 0){ /* File is already empty */
        return OK;
    }

    /* Clean up the open local file first */

    if ((lfptr->lfibdirty) || (lfptr->lfdbdirty)){
        lfflush(lfptr);
    }

    lfptr->lfpos = 0;
    lfptr->lfinum = LF_INULL;
    lfptr->lfdnum = LF_DNULL;
    lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];

    ibid32 ifree = Lf_data.lf_dir.lfd_ifree;

    /* Record file's first i-block and clear directory entry */

    ibid32 firstib = ldptr->ld_ilist;
    ldptr->ld_ilist = LF_INULL;
    ldptr->ld_size = 0;
    Lf_data.lf_dirdirty = TRUE;

    /* Walk along index block list, disposing of each data block	*/
	/* and clearing the corresponding pointer.  A note on loop	*/
	/* termination: last pointer is set to ifree below.		*/

    ibid32 nextib;
    dbid32 nextdb;
    struct lfiblk iblock;
    int32 i;
    for (nextib = firstib; nextib != ifree; nextib = iblock.ib_next){
        /* Obtain a copy of current index block from disk */

        lfibget(Lf_data.lf_dskdev, nextib, &iblock);

        /* Free each data block in the index block */

        for (i = 0; i < LF_IBLEN; i++){ /* For each d-block	*/
            nextdb = iblock.ib_dba[i];
            if (nextdb != LF_DNULL){ /* Free the data block */
                lfdbfree(Lf_data.lf_dskdev, nextdb);
                /* Clear entry in i-block for this d-block	*/
                iblock.ib_dba[i] = LF_DNULL;
            }
        }

        iblock.ib_offset = 0;

        /* For the last index block on the list, make it point	*/
		/* to the current free list */

        if (iblock.ib_next == LF_INULL){
            iblock.ib_next = ifree;
        }

        /* Write cleared i-block back to disk */

        lfibput(Lf_data.lf_dskdev, nextib, &iblock);
    }

    /* Last index block on the file list now points to first node	*/
	/* on the current free list. Once we make the free list	*/
	/* point to the first index block on the file list, the	*/
	/* entire set of index blocks will be on the free list	*/

    Lf_data.lf_dir.lfd_ifree = firstib;
    Lf_data.lf_dirdirty = TRUE;
    return OK;
}