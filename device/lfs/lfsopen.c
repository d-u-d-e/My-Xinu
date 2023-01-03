#include <kernel.h>

/*------------------------------------------------------------------------
 * lfsopen - Open a file and allocate a local file pseudo-device
 *------------------------------------------------------------------------
 */

devcall lfsopen(struct dentry * devptr, char * name, char * mode){
    return OK;
}