#include <kernel.h>
#include <stdarg.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  fprintf  -  Print a formatted message on specified device (file).
 *			Return 0 if the output was printed successfully,
 *			and -1 if an error occurred.
 *------------------------------------------------------------------------
 */

int fprintf(int dev, const char * fmt, ...)
{
    va_list ap;
    syscall putc(did32, char);

    va_start(ap, fmt);
    _fdoprnt((char *)fmt, ap, putc, dev);
    va_end(ap);
    return 0;
}