/************************************************************************/
/*									*/
/*               Local File System Data Structures			*/
/*									*/
/*   A local file system uses a random-access disk composed of 512-byte	*/
/* sectors numbered 0 through N-1.  We assume disk hardware can read or	*/
/* write any sector at random, but must transfer an entire sector.	*/
/* Thus, to write a few bytes, the file system must read the sector,	*/
/* replace the bytes, and then write the sector back to disk.  Xinu's	*/
/* local file system divides the disk as follows: sector 0 is a 	*/
/* directory, the next K sectors constitute an index area, and the	*/
/* remaining sectors comprise a data area. The data area is easiest to	*/
/* understand: each sector holds one data block (d-block) that stores	*/
/* contents from one of the files (or is on a free list of unused data	*/
/* blocks).  We think of the index area as holding an array of index	*/
/* blocks (i-blocks) numbered 0 through I-1.  A given sector in the	*/
/* index area holds 7 of the index blocks, which are each 72 bytes	*/
/* long.  Given an i-block number, the file system must calculate the	*/
/* disk sector in which the i-block is located and the byte offset	*/
/* within the sector at which the i-block resides.  Internally, a file	*/
/* is known by the i-block index of the first i-block for the file.	*/
/* The directory contains a list of file names and the	i-block number	*/
/* of the first i-block for the file.  The directory also holds the	*/
/* i-block number for a list of free i-blocks and a data block number	*/
/* of the first data block on a list of free data blocks.		*/
/*									*/
/************************************************************************/


#pragma once
#include <kernel.h>
#include <conf.h>
#include <file.h>

#ifndef	Nlfl
#define	Nlfl	1
#endif

#define	LF_NUM_DIR_ENT	20		/* Num. of files in a directory	*/
#define	LF_NAME_LEN	16		/* Length of name plus null	*/

#define	LFS_ID		0x58696E75U	/* ID for Xinu Local File System */
#define	LF_AREA_DIR	0		/* First sector of directory	*/
#define	LF_AREA_IB	1		/* First sector of i-blocks	*/

#define	LF_IBLEN	16		/* Data block ptrs per i-block */
#define	LF_BLKSIZ	512		/* Assumes 512-byte disk blocks	*/
#define	LF_IDATA	8192		/* Bytes of data indexed by a single index block */

#define	LF_INULL	(ibid32) -1	/* Index block null pointer	*/
#define	LF_DNULL	(dbid32) -1	/* Data block null pointer	*/

/* Structure of an index block on disk */

struct lfiblk {		/* Format of index block	*/
    ibid32  ib_next;	/* Address of next index block	*/
	uint32  ib_offset;	/* First data byte of the file indexed by this i-block */
	dbid32  ib_dba[LF_IBLEN];/* Ptrs to data blocks indexed	*/
}; // 72 bytes

/* Conversion functions below assume 7 index blocks per disk block */
/* Conversion between index block number and disk sector number */

#define	ib2sect(ib)	(((ib)/7) + LF_AREA_IB)

/* Conversion between index block number and the relative offset within	*/
/*	a disk sector */

#define	ib2disp(ib)	(((ib)%7) * sizeof(struct lfiblk))

/* Structure used in each directory entry for the local file system */

struct ldentry { /* Description of entry for one file in the directory */
	uint32	ld_size;    /* Curr. size of file in bytes	*/
	ibid32	ld_ilist;   /* ID of first i-block for file or IB_NULL for empty file */
	char	ld_name[LF_NAME_LEN];	/* Null-terminated file name */
};

/* Format of the file system directory, either on disk or in memory */

#pragma pack(2)
struct lfdir { //exactly 512 bytes that fit into a disk sector (first)
    uint32  lfd_fsysid;     /* File system ID */
    int16   lfd_vers;       /* File system version */
    int16   lfd_subvers;    /* File system subversion */
    uint32  lfd_allzeros;   /* All 0 bits */
    uint32	lfd_allones;    /* All 1 bits */
    dbid32	lfd_dfree;		/* List of free d-blocks on disk */
	ibid32	lfd_ifree;		/* List of free i-blocks on disk */
	int32	lfd_nfiles;		/* Current number of files	*/
	struct	ldentry lfd_files[LF_NUM_DIR_ENT]; /* Set of files	*/
	uint32	lfd_revid;		/* fsysid in reverse byte order	*/
};

#pragma pack()

/* Global data used by local file system */

struct lfdata { /* Local file system data */
    did32           lf_dskdev; /* Device ID of disk to use */
    sid32           lf_mutex; /* Mutex for the directory and index/data free lists */
    struct lfdir    lf_dir; /* In-memory copy of directory */
    bool8           lf_dirpresent; /* True when directory is in	memory */
    bool8           lf_dirdirty; /* Has the directory changed? */
};

/* Structure of a data block when on the free list on disk */

struct lfdbfree {
	dbid32  lf_nextdb;		/* Next data block on the list	*/
	char	lf_unused[LF_BLKSIZ - sizeof(dbid32)];
};

/* Control block for local file pseudo-device */

struct lflcblk { /* Local file control block (one for each open file) */
    byte    lfstate; /* Is entry free or used */
    did32   lfdev; /* Device ID of this device */
    sid32   lfmutex; /* Mutex for this file		*/
    struct ldentry * lfdirptr; /* Ptr to file's entry in the in-memory directory */
    int32   lfmode; /* Mode (read/write/both) */
    uint32  lfpos; /* Byte position of next byte to read or write */
    char    lfname[LF_NAME_LEN]; /* Name of the file */
    ibid32  lfinum; /* ID of current index block in	lfiblock or LF_INULL */
    struct lfiblk lfiblock; /* In-mem copy of current index	block */
    dbid32  lfdnum; /* Number of current data block	or LF_DNULL */
    char    lfdblock[LF_BLKSIZ]; /* In-mem copy of current data block */
    char *  lfbyte; /* Ptr to byte in lfdblock or address one beyond lfdblock */
                    /* if current file pos lies outside lfdblock */
    bool8   lfibdirty; /* Has lfiblock changed?	*/
    bool8	lfdbdirty; /* Has lfdblock changed?	*/
};

extern struct lfdata    Lf_data;
extern struct lflcblk   lfltab[];

#define	LF_MODE_R	F_MODE_R	/* Mode bit for "read"		*/
#define	LF_MODE_W	F_MODE_W	/* Mode bit for "write"		*/
#define	LF_MODE_RW	F_MODE_RW	/* Mode bits for "read or write" */
#define	LF_MODE_O	F_MODE_O	/* Mode bit for "old"		*/
#define	LF_MODE_N	F_MODE_N	/* Mode bit for "new"		*/

#define	LF_FREE		0		/* Slave device is available	*/
#define	LF_USED		1		/* Slave device is in use	*/

/* Control functions */

#define	LF_CTL_DEL	    F_CTL_DEL	/* Delete a file		*/
#define	LF_CTL_TRUNC    F_CTL_TRUNC	/* Truncate a file		*/
#define LF_CTL_SIZE	    F_CTL_SIZE	/* Obtain the size of a file	*/

#define	LF_IMASK	0x00001fff	/* Mask for the data indexed by	*/
					/*   one index block (i.e.,	*/
					/*   bytes 0 through 8191).	*/

#define	LF_DMASK	0x000001ff	/* Mask for the data in a data	*/
/*   block (0 through 511)	*/

devcall lfsinit(struct dentry * devptr);
status  lfscreate(did32 disk, ibid32 lfiblks, uint32 dsiz);
devcall lfsopen(struct dentry * devptr, char * name, char * mode);
int32   lfgetmode(char * mode);
status  lfscheck(struct lfdir * dirptr);

void    lfibclear(struct lfiblk * ibptr, int32 offset);
status  lfibput(did32 diskdev, ibid32 inum, struct lfiblk * ibuff);
void    lfibget(did32 diskdev, ibid32 inum, struct lfiblk * ibuff);
ibid32  lfiballoc(void);
status  lfdbfree(did32 diskdev, dbid32 dnum);
dbid32  lfdballoc(struct lfdbfree * dbuff);
status  lftruncate(struct lflcblk * lfptr);
status  lfflush(struct lflcblk * lfptr);
status  lfsetup(struct lflcblk * lfptr);

devcall	lflinit(struct dentry * devptr);
devcall	lflclose(struct dentry * devptr);
devcall lflread(struct dentry * devptr, char * buff, uint32 count);
devcall lflwrite(struct dentry * devptr, char * buff, uint32 count);
devcall lflseek(struct dentry * devptr, uint32 offset);
devcall lflputc(struct dentry * devptr, char ch);
devcall lflgetc(struct dentry * devptr);
devcall lflcontrol(struct dentry * devptr, int32 func, int32 arg1, int32 arg2);