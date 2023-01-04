#include <lfs.h>

/*------------------------------------------------------------------------
 * lfgetmode  -  Parse mode argument and generate integer of mode bits
 *------------------------------------------------------------------------
 */

int32 lfgetmode(char * mode)
{
    int32 mbits; /* Mode bits to return */
    char ch;

    mbits = 0;

    /* Mode string specifies:			*/
	/*	r - read						*/
	/*	w - write						*/
	/*	o - old (file must exist)		*/
	/*	n - new (create a new file)		*/

    while ((ch = *mode++) != NULLCH){
        switch (ch)
        {
        case 'r':
            if (mbits & LF_MODE_R){
                return SYSERR;
            }
            mbits |= LF_MODE_R;
            continue;
        case 'w':
            if (mbits & LF_MODE_W){
                return SYSERR;
            }
            mbits |= LF_MODE_W;
            continue;
        case 'o':
            if (mbits & LF_MODE_O || mbits & LF_MODE_N){
                return SYSERR;
            }
            mbits |= LF_MODE_O;
            continue;
        case 'n':
            if (mbits & LF_MODE_O || mbits & LF_MODE_N){
                return SYSERR;
            }
            mbits |= LF_MODE_N;
            continue;        
        default:
            return SYSERR;
        }
    }

    /* If neither read nor write specified, allow both */

    if ((mbits & LF_MODE_RW) == 0){
        mbits |= LF_MODE_RW;
    }
    return mbits;
}