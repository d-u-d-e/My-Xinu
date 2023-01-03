#include <ramdisk.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * ramread  -  Read a block from a ram disk
 *------------------------------------------------------------------------
 */

devcall ramread(struct dentry * devptr, char * buff, int32 blk)
{
    /* blk: block number of block to read */

    int32 bpos; /* Byte position of blk	*/

    bpos = RM_BLKSIZ * blk;
    memcpy(buff, &Ram.disk[bpos], RM_BLKSIZ);
    return OK;
}