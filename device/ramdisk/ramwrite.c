#include <ramdisk.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * ramwrite  -  Write a block to a ram disk
 *------------------------------------------------------------------------
 */

devcall ramwrite(struct dentry * devptr, char * buff, int32 blk)
{
    /* blk: block number of block to write */

    int32 bpos; /* Byte position of blk	*/

    bpos = RM_BLKSIZ * blk;
    memcpy(&Ram.disk[bpos], buff, RM_BLKSIZ);
    return OK;
}