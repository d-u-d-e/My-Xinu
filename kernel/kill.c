#include <process.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */

syscall kill(pid32 pid)
{
    return OK;
}