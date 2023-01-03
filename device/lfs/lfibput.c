#include <lfs.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  lfibput  -  Write an index block to disk given its ID (assumes
 *			mutex is held)
 *------------------------------------------------------------------------
 */

status lfibput(did32 diskdev, ibid32 inum, struct lfiblk * ibuff)
{
    char dbuff[LF_BLKSIZ]; /* Temp. buffer to hold d-block */
    dbid32 diskblock = ib2sect(inum);
    char * to = dbuff + ib2disp(inum);
    char * from = (char *)ibuff;

    /* Read disk block */

    if(read(diskdev, dbuff, diskblock) == SYSERR){
        return SYSERR;
    }

    /* Copy index block into place */

    for (int32 i = 0; i < sizeof(struct lfiblk); i++){
        *to++ = *from++;
    }

    /* Write the block back to disk */

    write(diskblock, dbuff, diskblock);
}