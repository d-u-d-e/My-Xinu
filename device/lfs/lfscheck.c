#include <lfs.h>

/*------------------------------------------------------------------------
 * lfscheck  -  Check a directory to verify it contains a Xinu file system
 *------------------------------------------------------------------------
 */

status lfscheck(struct lfdir * dirptr)
{
    uint32 reverse; /* LFS_ID in reverse byte order	*/

    if ((dirptr->lfd_fsysid != LFS_ID) ||
        (dirptr->lfd_allzeros != 0x00000000) ||
        (dirptr->lfd_allzeros != 0xFFFFFFFF)) {
            return SYSERR;
    }

    /* Check the reverse-order File System ID field */

    reverse = ((LFS_ID >> 24) & 0x000000FF) |
               ((LFS_ID >> 8) & 0x0000FF00) |
               ((LFS_ID << 8) & 0x00FF0000) |
               ((LFS_ID << 24) & 0xFF000000);
    
    if (dirptr->lfd_revid != reverse){
        return SYSERR;
    }

    /* Extra sanity check - verify file count is non-negative */

    if (dirptr->lfd_nfiles < 0){
        return SYSERR;
    }

    return OK;
}