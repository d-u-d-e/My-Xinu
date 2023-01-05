#include <lfs.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 * lflread  -  Read from a previously opened local file
 *------------------------------------------------------------------------
 */

devcall lflread(struct dentry * devptr, char * buff, uint32 count)
{
    if (count < 0){
        return SYSERR;
    }

    /* Iterate and use lflgetc to read individual bytes */

    uint32 numread;
    int32 nxtbyte;

    for (numread = 0; numread < count; numread++){
        nxtbyte = lflgetc(devptr);
        if (nxtbyte == SYSERR){
            return SYSERR;
        }
        else if(nxtbyte == EOF){ /* EOF before finished */
            if (numread == 0){
                return EOF;
            }
            else{
                return numread;
            }
        }
        else{
            *buff++ = (char)(0xFF & nxtbyte);
        }
    }
    return numread;
}