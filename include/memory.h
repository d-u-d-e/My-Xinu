#pragma once
#include <kernel.h>

struct memblk	{
	struct memblk * mnext;	/* Ptr to next free memory blk	*/
	uint32 mlength;	/* Size of blk (includes memblk)*/
	};


/* Added by linker */
extern int	text;	/* Start of text segment	*/
extern int	etext;	/* End of text segment		*/
extern int	data;	/* Start of data segment	*/
extern int	edata;	/* End of data segment		*/
extern int	bss;		/* Start of bss segment		*/
extern int	ebss;	/* End of bss segment		*/
extern int	end;		/* End of program		*/

extern void * minheap;		/* Start of heap		*/
extern void	* maxheap;		/* Highest valid heap address	*/

extern struct memblk memlist;	/* Head of free memory list	*/

/*----------------------------------------------------------------------
 * roundmb, truncmb - Round or truncate address to memory block size
 *----------------------------------------------------------------------
 */
#define	roundmb(x)	(unsigned char *)( (7 + (uint32)(x)) & (~7) )
#define	truncmb(x)	(unsigned char *)( ((uint32)(x)) & (~7) )

syscall freemem(char * blkaddr, uint32 nbytes);

#define	freestk(p,len)	freemem((char *)((uint32)(p) \
				- ((uint32)roundmb(len)) \
				+ (uint32)sizeof(uint32)), \
				(uint32)roundmb(len) )

void meminit(void);
char * getstk(uint32 nbytes);
char * getmem(uint32 nbytes);

