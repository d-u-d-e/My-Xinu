#include <lfs.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  lfibget  -  Get an index block from disk given its number (assumes
 *			mutex is held)
 *------------------------------------------------------------------------
 */

void lfibget(did32 diskdev, ibid32 inum, struct lfiblk * ibuff)
{
    char dbuff[LF_BLKSIZ]; /* Temp. buffer to hold d-block */

    /* Read disk block that contains the specified index block */

    read(diskdev, dbuff, ib2sect(inum));
    char * to = (char *)ibuff;
    char * from = dbuff + ib2disp(inum);

    /* Copy index block into place */

    for (int32 i = 0; i < sizeof(struct lfiblk); i++){
        *to++ = *from++;
    }
    return;
}