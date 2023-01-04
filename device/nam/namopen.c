#include <name.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  namopen  -  Open a file or device based on the name
 *------------------------------------------------------------------------
 */

devcall namopen(struct dentry * devptr, char * name, char * mode)
{

    char newname[NM_MAXLEN]; /* Name with prefix replaced */
    did32 newdev; /* Device ID after mapping */

    newdev = nammap(name, newname, devptr->dvnum);

    if (newdev == SYSERR){
        return SYSERR;
    }

    /* Open underlying device and return status */
    return open(newdev, name, mode);
}