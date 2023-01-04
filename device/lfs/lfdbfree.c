#include <lfs.h>

/*------------------------------------------------------------------------
 *  lfdbfree  -  Free a data block given its block number (assumes
 *			directory mutex is held)
 *------------------------------------------------------------------------
 */

status lfdbfree(did32 diskdev, dbid32 dnum)
{
    struct lfdir * dirptr;
    struct lfdbfree buf;

    dirptr = &Lf_data.lf_dir;
    buf.lf_nextdb = dirptr->lfd_dfree;
    dirptr->lfd_dfree = dnum;
    write(diskdev, (char *)&buf, dnum);
    write(diskdev, (char *)dirptr, LF_AREA_DIR);
    return OK;
}