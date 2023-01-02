#include <tty.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  ttycontrol  -  Control a tty device by setting modes
 *------------------------------------------------------------------------
 */

devcall ttycontrol(struct dentry * devptr, int32 func, int32 arg1, int32 arg2)
{
    struct ttycblk * typtr;

    typtr = &ttytab[devptr->dvminor];

    /* Process the request */

    switch (func)
    {
    case TC_MODER:
		typtr->tyimode = TY_IMRAW;
		return (devcall)OK;

	case TC_MODEC:
		typtr->tyimode = TY_IMCOOKED;
		return (devcall)OK;

	case TC_MODEK:
		typtr->tyimode = TY_IMCBREAK;
		return (devcall)OK;

    case TC_ICHARS:
		return(semcount(typtr->tyisem));

	case TC_ECHO:
		typtr->tyiecho = TRUE;
		return (devcall)OK;

	case TC_NOECHO:
		typtr->tyiecho = FALSE;
		return (devcall)OK;
    
    default:
        return (devcall)SYSERR;
    }

}