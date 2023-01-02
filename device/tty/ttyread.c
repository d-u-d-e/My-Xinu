#include <tty.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  ttyread  -  Read character(s) from a tty device (interrupts disabled)
 *------------------------------------------------------------------------
 */

devcall	ttyread(struct dentry * devptr, char * buff, int32 count)
{
    int32 avail, nread, firstch;

    if (count < 0){
        return SYSERR;
    }

    struct ttycblk * typtr = &ttytab[devptr->dvminor];

    if (typtr->tyimode != TY_IMCOOKED){

        /* For count of zero, return all available characters */
        if (count == 0){
            avail = semcount(typtr->tyisem);
            if (avail == 0){
                return 0;
            }
            else{
                count = avail;
            }
        }

        for (nread = 0; nread < count; nread++){
            *buff++ = (char) ttygetc(devptr);
        }
        return nread;
    }

    /* Block until input arrives */

    firstch = ttygetc(devptr);

    /* Check for End-Of-File */
    if (firstch == EOF){
        return EOF;
    }

    /* Read up to a line */
    char ch = (char) firstch;
    *buff++ = ch;
    nread = 1;
    while ( (nread < count) && (ch != TY_NEWLINE) && (ch != TY_RETURN)){
        ch = ttygetc(devptr);
        *buff++ = ch;
        nread++;
    }
    return nread;
}