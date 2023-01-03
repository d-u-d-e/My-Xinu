#include <lfs.h>
#include <semaphore.h>
#include <lib.h>

/*------------------------------------------------------------------------
 * lfsinit  -  Initialize the local file system master device
 *------------------------------------------------------------------------
 */

struct lfdata Lf_data;

devcall lfsinit(struct dentry * devptr){

    /* Indicate that no disk device has been selected */

    Lf_data.lf_dskdev = -1;
    
    /* Create a mutual exclusion semaphore */

    Lf_data.lf_mutex = semcreate(1);

    /* Zero directory area (for debugging) */

    memset((char *)&Lf_data.lf_dir, NULLCH, sizeof(struct lfdir));
    
    Lf_data.lf_dirpresent = Lf_data.lf_dirdirty = FALSE;
    return OK;
}