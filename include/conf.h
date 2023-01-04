#pragma once
#include <kernel.h>

/* Device table entry */
struct dentry	{
    int32   dvnum;
	int32   dvminor;
	char *  dvname;
	devcall (*dvinit) (struct dentry *);
	devcall (*dvopen) (struct dentry *, char *, char *);
	devcall (*dvclose)(struct dentry *);
	devcall (*dvread) (struct dentry *, void *, uint32);
	devcall (*dvwrite)(struct dentry *, void *, uint32);
	devcall (*dvseek) (struct dentry *, int32);
	devcall (*dvgetc) (struct dentry *);
	devcall (*dvputc) (struct dentry *, char);
	devcall (*dvcntl) (struct dentry *, int32, int32, int32);
	void *  dvcsr;
	void    (*dvintr)(void);
	byte    dvirq;
};

extern struct dentry devtab[]; /* one entry per device */

/* Device name definitions */

#define NDEVS       11
#define CONSOLE     0	/* type tty */
#define NULLDEV     1	/* type null     */
#define RAM0        2	/* type ram */
#define LFILESYS    3	/* type lfs      */
#define NAMESPACE   4	/* type nam      */
#define RFILESYS    6	/* type rfs      */

#define	Ntty	1
#define	Nlfl	6 // local files