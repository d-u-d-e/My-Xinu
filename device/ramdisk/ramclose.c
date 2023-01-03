#include <kernel.h>

/*------------------------------------------------------------------------
 * Ramclose  -  Close a ram disk
 *------------------------------------------------------------------------
 */

devcall ramclose(struct dentry * devptr)
{
    return OK;
}