#include <kernel.h>

struct dentry;

/*------------------------------------------------------------------------
 * Ramclose  -  Close a ram disk
 *------------------------------------------------------------------------
 */

devcall ramclose(struct dentry * devptr)
{
    return OK;
}