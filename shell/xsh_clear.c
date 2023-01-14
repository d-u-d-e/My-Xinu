#include <kernel.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_clear - clear the display window (assumes xterm / VT100)
 *------------------------------------------------------------------------
 */
shellcmd xsh_clear(int nargs, char * args[])
{
    if (nargs > 1){
        fprintf(stderr, "usage: %s\n", args[0]);
        return 1;
    }
    printf(CONSOLE_RESET);
    return 0;
}