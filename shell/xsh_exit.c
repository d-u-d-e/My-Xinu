#include "shproto.h"
#include <shell.h>

/*------------------------------------------------------------------------
 * xsh_exit  -  shell command returns the exit code causing shell exit
 *------------------------------------------------------------------------
 */

shellcmd xsh_exit(int nargs, char *args[])
{
	return SHELL_EXIT;
}