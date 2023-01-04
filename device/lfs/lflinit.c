#include <lfs.h>
#include <semaphore.h>
#include <lib.h>

struct lflcblk lfltab[Nlfl]; /* Pseudo-device control blocks */

/*------------------------------------------------------------------------
 * lflinit  -  Initialize control blocks for local file pseudo-devices
 *------------------------------------------------------------------------
 */

devcall lflinit(struct dentry * devptr)
{
    struct lflcblk * lfptr;
    int32 i;

    lfptr = &lfltab[devptr->dvminor];

    /* Initialize control block entry */

    lfptr->lfstate = LF_FREE; /* Device is currently unused	*/
    lfptr->lfdev = devptr->dvnum; /* Set device ID */
    lfptr->lfmutex = semcreate(1); /* Create the mutex semaphore */

    /* Initialize the directory and file position */

    lfptr->lfdirptr = (struct ldentry *)NULL;
    lfptr->lfpos = 0;

    for(i = 0; i < LF_NAME_LEN; i++){
        lfptr->lfname[i] = NULLCH;
    }
    
    /* Zero the in-memory index block and data block */

    lfptr->lfinum = LF_INULL;
    memset((char *) &lfptr->lfiblock, NULLCH, sizeof(struct lfiblk));
    lfptr->lfdnum = LF_DNULL;
    memset((char *) &lfptr->lfdblock, NULLCH, LF_BLKSIZ);

    /* Start with the byte beyond the current data block */

    lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
    lfptr->lfibdirty = lfptr->lfdbdirty = FALSE;
    return OK; 
}