#include <lfs.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * lflgetc  -  Read the next byte from an open local file
 *------------------------------------------------------------------------
 */

devcall lflgetc(struct dentry * devptr)
{
    /* Obtain exclusive use of the file */

    struct lflcblk * lfptr = &lfltab[devptr->dvminor];
    wait(lfptr->lfmutex);

    /* If file is not open, return an error */

    if (lfptr->lfstate != LF_USED){
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    /* Return EOF for any attempt to read beyond the end-of-file */
 
    struct ldentry * ldptr = lfptr->lfdirptr; /* Ptr to file's entry in the in-memory directory */
    if (lfptr->lfpos >= ldptr->ld_size){
        signal(lfptr->lfmutex);
        return EOF;
    }

    /* If byte pointer is beyond the current data block, set up	*/
	/*	a new data block */

    if (lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ]){
        lfsetup(lfptr);
    }

    /* Extract the next byte from block, update file position, and	*/
	/* return the byte to the caller */

    int32 onebyte = 0xFF & *lfptr->lfbyte++; //avoid sign extension
    lfptr->lfpos++;
    signal(lfptr->lfmutex);
    return onebyte;
}