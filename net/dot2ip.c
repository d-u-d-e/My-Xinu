#include <ip.h>

/*------------------------------------------------------------------------
 * dot2ip  -  Convert a string of dotted decimal to an unsigned integer
 *------------------------------------------------------------------------
 */

status dot2ip(char * dotted, uint32 * result) /* result in host byte order */
{
    uint32 ipaddr = 0;
    int32 seg, val, nch;
    char ch;
    for (seg = 0; seg < 4; seg++){
        val = 0;
        for (nch = 0; nch < 4; nch++){
            ch = *dotted++;

            if (ch == NULLCH || ch == '.'){
                if (nch == 0){
                    return SYSERR;
                }
                else{
                    break;
                }
            }

            /* Non digit is an error */

            if (ch < '0' || ch > '9'){
                return SYSERR;
            }

            val = 10 * val + (ch - '0');
        }

        if (val > 255){
            return SYSERR;
        }
        ipaddr = (ipaddr << 8) | val;

        if (ch == NULLCH){ /* end of segments */
            break;
        }
    }
    if (seg != 3 || ch != NULLCH){
        return SYSERR;
    }
    *result = ipaddr;
    return OK;
}