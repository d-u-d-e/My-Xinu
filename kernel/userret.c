#include <process.h>

/*------------------------------------------------------------------------
 *  userret  -  Called when a process returns from the top-level function
 *------------------------------------------------------------------------
 */

void userret(void)
{
    kill(getpid());
}