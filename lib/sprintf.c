#include <kernel.h>
#include <stdarg.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  sprintf  -  Format arguments and place output in a string.
 *------------------------------------------------------------------------
 */

static int sprntf(int, char);

extern void _fdoprnt(char * fmt, va_list ap, int (*func)(int, char), int farg);

int sprintf(char * str, char * fmt, ...)
{
    va_list ap;
    char * s = str;
    va_start(ap, fmt);
    _fdoprnt(fmt, ap, sprntf, (int)&s);
    va_end(ap);
    *s++ = '\0';
    return ((int)str);
}

/*------------------------------------------------------------------------
 *  sprntf  -  Routine called by _doprnt to handle each character.
 *------------------------------------------------------------------------
 */

int sprntf(int acpp, char ac)
{
    char ** cpp = (char **)acpp;
    return (*(*cpp)++ = ac);
}