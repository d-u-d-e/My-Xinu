/* Definintion of standard input/ouput/error used with shell commands */

#include <process.h>

#define	stdin	((proctab[currpid]).prdesc[0])
#define	stdout	((proctab[currpid]).prdesc[1])
#define	stderr	((proctab[currpid]).prdesc[2])

syscall write(did32 descrp, char * buff, uint32 count);
syscall read(did32 descrp, char * buff, uint32 count);
syscall close(did32 descrp);

int fprintf(int dev, const char * fmt, ...);
int printf(const char * fmt, ...);