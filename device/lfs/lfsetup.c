#include <lfs.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * lfsetup  -  Set a file's index block and data block for the current
 *		 file position (assumes file mutex held)
 *------------------------------------------------------------------------
 */

status lfsetup(struct lflcblk * lfptr)
{
    /* Obtain exclusive access to the directory */
    wait(Lf_data.lf_mutex);

    /* Get pointers to in-memory directory, file's entry in the	*/
	/* directory, and the in-memory index block	*/

    struct ldentry * ldptr = lfptr->lfdirptr;
    struct lfiblk * ibptr = &lfptr->lfiblock;

    /* If existing index block or data block changed, write to disk	*/

    if (lfptr->lfibdirty || lfptr->lfdbdirty){
        lfflush(lfptr);
    }

    ibid32 ibnum = lfptr->lfinum; /* Get ID of curr. index block */

    /* If there is no index block in memory (e.g., because the file	*/
	/*	was just opened), either load the first index block of	*/
	/*	the file or allocate a new first index block		*/

    if (ibnum == LF_INULL){
        /* Check directory entry to see if index block exists */
        ibnum = ldptr->ld_ilist;
        if (ibnum == LF_INULL){ /* Empty file - get new i-block */
            ibnum = lfiballoc();
            lfibclear(ibptr, 0);
            ldptr->ld_ilist = ibnum;
            lfptr->lfibdirty = TRUE;
        }
        else{ /* Nonempty - read first i-block */
            lfibget(Lf_data.lf_dskdev, ibnum, ibptr);
        }
        lfptr->lfinum = ibnum;

    /* Otherwise, if current file position has been moved to an	*/
	/* offset before the current index block, start at the	*/
	/* beginning of the index list for the file			*/

    }
    else if (lfptr->lfpos < ibptr->ib_offset){
        /* Load initial index block for the file (we know that	*/
		/* at least one index block exists)		*/

        ibnum = ldptr->ld_ilist;
        lfibget(Lf_data.lf_dskdev, ibnum, ibptr);
        lfptr->lfinum = ibnum;
    }

    /* At this point, an index block is in memory, but may cover	*/
	/* an offset less than the current file position.  Loop until	*/
	/* the index block covers the current file position.		*/

    uint32 newoffset;

    while ((lfptr->lfpos & ~LF_IMASK) > ibptr->ib_offset){
        ibnum = ibptr->ib_next;
        if (ibnum == LF_INULL){
            /* Allocate new index block to extend file */
            ibnum = lfiballoc();
            ibptr->ib_next = ibnum;
            lfibput(Lf_data.lf_dskdev, lfptr->lfinum, ibptr);
            lfptr->lfinum = ibnum;
            newoffset = ibptr->ib_offset + LF_IDATA;
            lfibclear(ibptr, newoffset);
            lfptr->lfibdirty = TRUE;
        }
        else{
            lfibget(Lf_data.lf_dskdev, ibnum, ibptr);
        }
        lfptr->lfdnum = LF_DNULL; /* Invalidate old data block */
    }

    /* At this point, the index block in lfiblock covers the	 */
	/* current file position (i.e., position lfptr->lfpos). The  */
	/* next step consists of loading the correct data block.	 */

    int32 dindex = (lfptr->lfpos & LF_IMASK) >> 9; // divide by 512 which is disk sector size

    /* 8192 / 512 == 16 data blocks indexed by one i-block */

    /* If data block index does not match current data block, read	*/
	/* the correct data block from disk */

    dbid32 dnum = lfptr->lfiblock.ib_dba[dindex];
    if (dnum == LF_DNULL){ /* Allocate new data block */
        dnum = lfdballoc((struct lfdbfree *)&lfptr->lfdblock);
        lfptr->lfiblock.ib_dba[dindex] = dnum;
        lfptr->lfibdirty = TRUE;
    }
    else if (dnum != lfptr->lfdnum){
        read(Lf_data.lf_dskdev, (char *)lfptr->lfdblock, dnum);
        lfptr->lfdbdirty = FALSE;
    }
    lfptr->lfdnum = dnum;

    /* Use current file offset to set the pointer to the next byte	*/
	/* within the data block */

    lfptr->lfbyte = &lfptr->lfdblock[lfptr->lfpos & LF_DMASK];
    signal(Lf_data.lf_mutex);
    return OK;
}