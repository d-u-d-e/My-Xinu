#include <kernel.h>
#include <stdarg.h>
#include <stdio.h>

void _fdoprnt(char * fmt, va_list ap, int (*func)(int, char), int farg);

/*------------------------------------------------------------------------
 *  printf  -  standard C printf function
 *------------------------------------------------------------------------
 */

int printf(const char * fmt, ...)
{
    va_list ap;
    syscall putc(did32, char);

    va_start(ap, fmt);
    _fdoprnt((char *)fmt, ap, putc, stdout);
    va_end(ap);
    return 0;
}