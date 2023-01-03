#include <kernel.h>
#include <shell.h>

/*------------------------------------------------------------------------
 * lexan  -  Ad hoc lexical analyzer to divide command line into tokens
 *------------------------------------------------------------------------
 */

int32 lexan(char * line, int32 len, char * tokbuf, int32 * tlen, int32 tok[], int32 toktyp[])
{
    uint32 ntok = 0;
    char * p = line;
    int32 tbindex = 0; /* Index into tokbuf	*/
    char ch, quote;

    /* While not yet at end of line, get next token */

    while ((*p != NULLCH) && (*p != SH_NEWLINE)){
        /* If too many tokens, return error */

        if (ntok >= SHELL_MAXTOK){
            return SYSERR;
        }

        /* Skip whitespace before token */

        while ((*p == SH_BLANK) || (*p == SH_TAB)){
            p++;
        }

        /* Stop parsing at end of line (or end of string) */

        ch = *p;
        if ((ch == SH_NEWLINE) || (ch == NULLCH)){
            *tlen = tbindex;
            return ntok;
        }

        /* Set next entry in tok array to be an index to the	*/
		/* current location in the token buffer		*/

        tok[ntok] = tbindex;

        switch (ch)
        {
        case SH_AMPER: 
            toktyp[ntok] = SH_TOK_AMPER;
            tokbuf[tbindex++] = ch;
            tokbuf[tbindex++] = NULLCH;
            ntok++;
            p++;
            continue;
        case SH_LESS:
            toktyp[ntok] = SH_TOK_LESS;
            tokbuf[tbindex++] = ch;
            tokbuf[tbindex++] = NULLCH;
            ntok++;
            p++;
            continue;
        case SH_GREATER:
            toktyp[ntok] = SH_TOK_GREATER;
            tokbuf[tbindex++] = ch;
            tokbuf[tbindex++] = NULLCH;
            ntok++;
            p++;
            continue;
        default:
            toktyp[ntok] = SH_TOK_OTHER;
            break;
        }

        /* Handle quoted string (single or double quote) */

        if ((ch == SH_SQUOTE) || (ch == SH_DQUOTE)){
            quote = ch; /* remember opening quote */
            p++; /* Move past starting quote */

            while (((ch = *p++) != quote) && (ch != SH_NEWLINE) && (ch != NULLCH)){
                tokbuf[tbindex++] = ch;
            }
            if (ch != quote){ /* string missing end quote	*/
                return SYSERR;
            }

            /* Finished string - count token and go on */

            tokbuf[tbindex++] = NULLCH; /* terminate token */
            ntok++; /* count string as one token */
            continue; /* go to next token */
        }

        /* Handle a token other than a quoted string */

        tokbuf[tbindex++] = ch;	/* put first character in buffer */
        p++;

        while (((ch = *p) != SH_NEWLINE) && (ch != NULLCH) && (ch != SH_LESS)
                && (ch != SH_AMPER) && (ch != SH_GREATER) && (ch != SH_BLANK)
                && (ch != SH_SQUOTE) && (ch != SH_DQUOTE) && (ch != SH_TAB))
        {
                tokbuf[tbindex++] = ch;
                p++;
        }

        /* Report error if other token is appended */

        if ((ch == SH_SQUOTE) || (ch == SH_DQUOTE) || (ch == SH_LESS)
        || (ch == SH_GREATER)){
            return SYSERR;
        }

        tokbuf[tbindex++] = NULLCH;
        ntok++;
    }
    *tlen = tbindex;
    return ntok;
}

