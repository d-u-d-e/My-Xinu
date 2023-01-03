#include <lfs.h>
#include <device.h>
#include <lib.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lfscreate  -  Create an initially-empty file system on a disk
 *------------------------------------------------------------------------
 */

status lfscreate(did32 disk, ibid32 lfiblks, uint32 dsiz)
{
    /* lfiblks is num. of index blocks on disk */
    uint32  sectors; /* Number of sectors to use */
    uint32  ibsectors; /* Number of sectors of i-blocks */
    uint32	ibpersector; /* Number of i-blocks per sector*/

    /* See if the disk device is valid or if a disk has already	*/
	/*	been formatted */

    if (isbaddev(disk) || Lf_data.lf_dskdev >= 0){
        return SYSERR;
    }

    Lf_data.lf_dskdev = disk;

    /* Compute total sectors on disk */

    sectors = dsiz / LF_BLKSIZ; /* Truncate to full sector */

    /* Compute number of sectors comprising i-blocks */

    ibpersector = LF_BLKSIZ / sizeof(struct lfiblk); // this is just 7
    ibsectors = (lfiblks + (ibpersector - 1)) / ibpersector; /* Round up */
    lfiblks = ibsectors * ibpersector;
    if (ibsectors > sectors / 2){ /* Invalid arguments */
        return SYSERR;
    }

    /* Create an initial directory */

    struct lfdir dir; /* Buffer to hold the directory */

    memset((char *)&dir, NULLCH, sizeof(struct lfdir));
    dir.lfd_fsysid = LFS_ID;
    dir.lfd_nfiles = 0;
    dir.lfd_allzeros = 0;
    dir.lfd_allones = 0xFFFFFFFF;
    dir.lfd_revid = (LFS_ID >> 24 & 0x000000FF) | (LFS_ID >> 8 & 0x0000FF00) |
                    (LFS_ID << 8 & 0x00FF0000) | (LFS_ID << 24 & 0xFF000000);
    dbid32 dbindex = (dbid32)(ibsectors + 1); //data blocks follow index blocks
    dir.lfd_dfree = dbindex;
    uint32 dblks = sectors - ibsectors - 1; /* Total free data blocks */
    int32 retval = write(disk, (char *)&dir, LF_AREA_DIR);
    if (retval == SYSERR){
        return SYSERR;
    }

    /* Create list of free i-blocks on disk */
    struct lfiblk iblock;	/* Space for one i-block */
    lfibclear(&iblock, 0);
    int32 i;
    for (i = 0; i < lfiblks - 1; i++){
        iblock.ib_next = (ibid32)(i + 1);
        lfibput(disk, i, &iblock);
    }
    iblock.ib_next = LF_INULL;
    lfibput(disk, i, &iblock);

    /* Create list of free data blocks on disk */

    struct lfdbfree dblock;	/* Data block on the free list	*/
    memset((char *)&dblock, NULLCH, LF_BLKSIZ);
    for (i = 0; i < dblks - 1; i++){
        dblock.lf_nextdb = dbindex + 1;
        write(disk, (char *)&dblock, dbindex);
        dbindex++;
    } 

    dblock.lf_nextdb = LF_DNULL;
    write(disk, (char *)&dblock, dbindex);
    close(disk);
    return OK;
}