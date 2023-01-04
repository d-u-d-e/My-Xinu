/* kernel.h */
#pragma once

/* Boolean constants */

#define FALSE	0		/* Boolean False			*/
#define TRUE	1		/* Boolean True				*/

/* Null pointer, character, and string definintions */

#define NULL	0		/* null pointer for linked lists	*/
#define NULLCH	'\0'	/* null character			*/
#define	NULLSTR	""		/* null string				*/

typedef	unsigned char       byte;
typedef	unsigned char       uint8;
typedef	int                 int32;
typedef	short               int16;
typedef	unsigned int        uint32;
typedef	unsigned short      uint16;
typedef	unsigned long long  uint64;


/* Xinu-specific types */

typedef	int32	pid32;		/* process ID */
typedef	int16	pri16;		/* process priority	*/
typedef	int32	sid32;		/* semaphore ID	*/
typedef	uint32	umsg32;		/* message passed among processes */
typedef	byte	bool8;		/* Boolean type	*/
typedef	uint32	intmask;	/* saved interrupt mask */
typedef	int16	qid16;		/* queue ID	*/
typedef	int32	bpid32;		/* buffer pool ID */
typedef	int32	did32;		/* device ID */
typedef	int32	ibid32;		/* index block ID (used in file system)	*/
typedef	int32	dbid32;		/* data block ID (used in file system)	*/
typedef int32	shellcmd;	/* shell command declaration		*/

/* Function declaration return types */

typedef int32   syscall;	/* system call declaration */
typedef int32	devcall;	/* device call declaration */
typedef	void    interrupt;	/* interrupt procedure			*/
typedef	int32	status;		/* returned status value (OK/SYSERR)	*/
typedef int32	process;	/* top-level function of a process	*/

/* Universal return constants */

#define OK	( 1)		/* normal system call return		*/
#define	SYSERR	(-1)		/* system call failed			*/
#define	EOF	(-2)		/* End-of-file (usually from read)	*/
#define	TIMEOUT	(-3)		/* system call timed out		*/

/* Size of the stack for the null process */

#define	NULLSTK		8192	/* stack size for null process		*/
#define	MINSTK	400		/* minimum stack size in bytes		*/

extern qid16 readylist;	/* global ID for list of ready processes */

#define	QUANTUM	2		/* time slice in milliseconds */


void enable(void);
intmask disable(void);
void initevec(void);
void restore(intmask);
syscall kprintf(char *fmt, ...);

