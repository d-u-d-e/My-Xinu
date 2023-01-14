/* Definintion of standard input/ouput/error used with shell commands */

#include <process.h>

#define	stdin	((proctab[currpid]).prdesc[0])
#define	stdout	((proctab[currpid]).prdesc[1])
#define	stderr	((proctab[currpid]).prdesc[2])

int fprintf(int dev, const char * fmt, ...);
int printf(const char * fmt, ...);
int sprintf(char * str, char * fmt, ...);

syscall getc(did32 descrp);
syscall putc(did32 descrp, char ch);