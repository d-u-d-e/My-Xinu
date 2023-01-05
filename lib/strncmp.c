#include <lib.h>

/*------------------------------------------------------------------------
 *  strncmp  -  Compare at most n bytes of two strings, returning
 *			>0 if s1>s2,  0 if s1=s2,  and <0 if s1<s2
 *------------------------------------------------------------------------
 */

int strncmp(char * s1, char * s2, int n)
{
    while (--n >= 0 && *s1 == *s2++){
        if (*s1++ == '\0'){
            return 0;
        }
    }
    return (n < 0? 0: *s1 - *--s2);
}