#include <lfs.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * lfdballoc  -  Allocate a new data block from free list on disk
 *			(assumes directory mutex held)
 *------------------------------------------------------------------------
 */

#define  DFILL  '+'		/* character used to fill a disk block	*/

dbid32 lfdballoc(struct lfdbfree * dbuff)
{
    dbid32 dnum = Lf_data.lf_dir.lfd_dfree;
    if (dnum == LF_DNULL){ /* Ran out of free data blocks */
        panic("out of data blocks");
    }
    int32 retval = read(Lf_data.lf_dskdev, (char *)dbuff, dnum);
    if (retval == SYSERR){
        panic("lfdballoc cannot read disk block\n");
    }

    /* Unlink d-block from in-memory directory */

    Lf_data.lf_dir.lfd_dfree = dbuff->lf_nextdb;
    write(Lf_data.lf_dskdev, (char *)&Lf_data.lf_dir, LF_AREA_DIR);
    Lf_data.lf_dirdirty = FALSE;

    /* Fill data block to erase old data */

    memset((char *)dbuff, DFILL, LF_BLKSIZ);
    return dnum;
}