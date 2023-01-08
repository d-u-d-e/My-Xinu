#include <kernel.h>

/*------------------------------------------------------------------------
 *  getticks  -  Retrieve the number of clock ticks since CPU reset
 *------------------------------------------------------------------------
 */

uint32 getticks()
{
    uint32 ret;
    asm volatile ("mrc p15, 0, %0, c9, c13, 0\n\t" : "=r"(ret));
    return ret;
}