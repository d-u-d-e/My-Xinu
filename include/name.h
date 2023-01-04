#pragma once
#include <kernel.h>
#include <conf.h>

/* Constants that define the namespace mapping table sizes */

#define	NM_PRELEN	64		/* Max size of a prefix string	*/
#define	NM_REPLLEN	96		/* Maximum size of a replacement*/
#define	NM_MAXLEN	256		/* Maximum size of a file name	*/
#define	NNAMES		128		/* Number of prefix definitions	*/


struct nmentry{
    char nprefix[NM_PRELEN];
    char nreplace[NM_REPLLEN];
    did32 ndevice;
};

extern struct nmentry nametab[]; /* Table of name mappings */
extern int32 nnames; /* Number of entries allocated	*/

status naminit(void);
devcall namopen(struct dentry * devptr, char * name, char * mode);
syscall mount(char * prefix, char * replace, did32 device);
devcall nammap(char * name, char newname[NM_MAXLEN], did32 namdev);
int32 namlen(char * name, int32 maxlen);