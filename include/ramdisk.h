#pragma once
#include <kernel.h>
#include <conf.h>

/* ramdisk.h - definitions for a ram disk (for testing) */

/* Ram disk block size */

#define	RM_BLKSIZ	512		/* block size			*/
#define	RM_BLKS		200		/* number of blocks		*/

struct ramdisk {
	char disk[RM_BLKSIZ * RM_BLKS];
};

extern struct ramdisk Ram;

devcall ramclose(struct dentry * devptr);
devcall ramread(struct dentry * devptr, char * buff, int32 blk);
devcall ramwrite(struct dentry * devptr, char * buff, int32 blk);
devcall raminit(struct dentry * devptr);
devcall ramopen(struct dentry * devptr, char * name, char * mode);