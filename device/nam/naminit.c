#include <name.h>

#ifndef	RFILESYS
#define	RFILESYS	SYSERR
#endif

#ifndef	FILESYS
#define	FILESYS		SYSERR
#endif

#ifndef	LFILESYS
#define	LFILESYS	SYSERR
#endif

struct nmentry nametab[NNAMES];	/* Table of name mappings	*/
int32 nnames; /* Number of entries allocated	*/

/*------------------------------------------------------------------------
 *  naminit  -  Initialize the syntactic namespace
 *------------------------------------------------------------------------
 */

status naminit(void)
{
    /* Set prefix table to empty */
    nnames = 0;
    int32 i;
    char tmpstr[NM_MAXLEN];
    char * tptr; /* Pointer into tempstring	*/
    char * nptr; /* Pointer to device name	*/
    int32 len;

    char devprefix[] = "/dev/"; /* Prefix to use for devices */

    struct dentry * devptr; /* Pointer to device table entry */
    char ch;
    status retval;

    for (i = 0; i < NDEVS; i++){
        tptr = tmpstr;
        nptr = devprefix;

        /* Copy prefix into tmpstr */

        len = 0;
        while ((*tptr++ = *nptr++) != NULLCH){
            len++;
        }
        tptr--; /* Move pointer to position before NULLCH */

        devptr = &devtab[i];
        nptr = devptr->dvname;

        /* Map device name to lower case and append */

        while (++len < NM_MAXLEN){
            ch = *nptr++;
            if ((ch >= 'A') && (ch <= 'Z')){
                ch += 'a' - 'A';
            }
            if ((*tptr++ = ch) == NULLCH){
                break;
            }
        }

        if (len > NM_MAXLEN){
            kprintf("namespace: device name %s too long\n", devptr->dvname);
            continue;
        }

        retval = mount(tmpstr, NULLSTR, devptr->dvnum);
        if (retval == SYSERR){
            kprintf("namespace: cannot mount device %s\n", devptr->dvname);
            continue;
        }
    }

    /* Add other prefixes (longest prefix first) */

    mount("/dev/null", "", NULLDEV);
    mount("/remote/", NULLSTR, RFILESYS);
    mount("/local/", NULLSTR, LFILESYS);
    mount("/tmp/", "tmp-", LFILESYS);
    mount("/dev/", NULLSTR, SYSERR);
    mount("~/", NULLSTR, RFILESYS);
    mount("/", "root:", RFILESYS);
    mount("", NULLSTR, LFILESYS);
    mount(".", NULLSTR, LFILESYS);
    return OK;
}