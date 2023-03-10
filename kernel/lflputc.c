#include <lfs.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * lflputc  -  Write a single byte to an open local file
 *------------------------------------------------------------------------
 */

devcall lflputc(struct dentry * devptr, char ch)
{
    /* Obtain exclusive use of the file */

    struct lflcblk * lfptr = &lfltab[devptr->dvminor];
    wait(lfptr->lfmutex);

    /* If file is not open, return an error */

    if (lfptr->lfstate != LF_USED){
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    /* Return SYSERR for an attempt to skip bytes beyond the byte	*/
	/* that is currently the end of the file		 	*/

    struct ldentry * ldptr = lfptr->lfdirptr;

    if (lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ]){
        /* Set up block for current file position */
        lfsetup(lfptr);
    }

    /* If appending a byte to the file, increment the file size. */
	/* Note: comparison might be equal, but should not be greater.*/

    if (lfptr->lfpos >= ldptr->ld_size){
        ldptr->ld_size++;
        Lf_data.lf_dirdirty = TRUE;
    }

    /* Place byte in buffer and mark buffer "dirty" */

    *lfptr->lfbyte++ = ch;
    lfptr->lfpos++;
    lfptr->lfdbdirty = TRUE;

    signal(lfptr->lfmutex);
    return OK;
}