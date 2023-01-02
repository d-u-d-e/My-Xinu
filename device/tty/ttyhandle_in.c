#include <tty.h>
#include <uart.h>
#include <semaphore.h>

/*------------------------------------------------------------------------
 *  eputc  -  Put one character in the echo queue
 *------------------------------------------------------------------------
 */
static void eputc(char ch, struct ttycblk * typtr, struct uart_csreg * csrptr)
{
	*typtr->tyetail++ = ch;

	/* Wrap around buffer, if needed */

	if (typtr->tyetail >= &typtr->tyebuff[TY_EBUFLEN]) {
		typtr->tyetail = typtr->tyebuff;
	}

	ttykickout(csrptr);
	return;
}


/*------------------------------------------------------------------------
 *  echoch  -  Echo a character with visual and output crlf options
 *------------------------------------------------------------------------
 */
static void	echoch(char	ch, struct ttycblk * typtr,	struct uart_csreg * csrptr)
{
	if ((ch == TY_NEWLINE || ch == TY_RETURN) && typtr->tyecrlf) {
		eputc(TY_RETURN, typtr, csrptr);
		eputc(TY_NEWLINE, typtr, csrptr);
	} else if ((ch < TY_BLANK || ch == 0177) && typtr->tyevis) { 
        // control chars come before BLANK (0177 is DEL char)
		eputc(TY_UPARROW, typtr, csrptr);/* print ^x */
		eputc(ch + 0100, typtr, csrptr);	/* Make it printable */
	} else {
		eputc(ch, typtr, csrptr);
	}
}

/*------------------------------------------------------------------------
 *  erase1  -  Erase one character honoring erasing backspace
 *------------------------------------------------------------------------
 */

static void erase1(struct ttycblk * typtr,	struct uart_csreg * csrptr)
{ 
    char ch; /* Character to erase */

    if ((--typtr->tyitail) < typtr->tyibuff) {
		typtr->tyitail += TY_IBUFLEN;
	}

    /* Pick up char to erase */
	ch = *typtr->tyitail;

    if (typtr->tyiecho){ /* Are we echoing?	*/
        if ((ch < TY_BLANK || ch == 0177) && (typtr->tyevis)){ /* Nonprintable + visual */
            eputc(TY_BACKSP, typtr, csrptr); // delete char following arrow
            if (typtr->tyieback){ /* erase the character */
                eputc(TY_BLANK, typtr, csrptr);
                eputc(TY_BACKSP, typtr, csrptr);
            }
        }
        eputc(TY_BACKSP, typtr, csrptr); // if above was true delete the caret
        if (typtr->tyieback){ /* erase the character */
            eputc(TY_BLANK, typtr, csrptr);
            eputc(TY_BACKSP, typtr, csrptr);
        }
    }
}

/*------------------------------------------------------------------------
 *  ttyhandle_in  -  Handle one arriving char (interrupts disabled)
 *------------------------------------------------------------------------
 */

void ttyhandle_in(struct ttycblk * typtr, struct uart_csreg * csrptr)
{
    char ch; /* Next char from device */
    int32 avail; /* Chars available in buffer */

    ch = csrptr->buffer;

    /* Compute chars available */

    avail = semcount(typtr->tyisem);
    if (avail < 0){ /* One or more processes waiting */
        avail = 0;
    }

    /* Handle raw mode */

    if (typtr->tyimode == TY_IMRAW) {
        if (avail >= TY_IBUFLEN){ /* No space => ignore input */
            return;
        }

        /* Place char in buffer with no editing */

        *typtr->tyitail++ = ch;

        /* Wrap buffer pointer	*/
		if (typtr->tyitail >= &typtr->tyibuff[TY_IBUFLEN]) {
			typtr->tyitail = typtr->tyibuff;
		}

        /* Signal input semaphore and return */
        signal(typtr->tyisem);
        return;
    }

    /* Handle cooked and cbreak modes (common part) */
    
    if ((ch == TY_RETURN) && typtr->tyicrlf) {
		ch = TY_NEWLINE;
	}

    /* If flow control is in effect, handle ^S and ^Q */

	if (typtr->tyoflow) {
		if (ch == typtr->tyostart) {	    /* ^Q starts output	*/
			typtr->tyoheld = FALSE;
			ttykickout(csrptr); /* this is necessary, processes cannot call ttyputc again! */
			return;
		} else if (ch == typtr->tyostop) {  /* ^S stops	output	*/
			typtr->tyoheld = TRUE;
			return;
		}
	}

    typtr->tyoheld = FALSE;	/* Any other char starts output */

    if (typtr->tyimode == TY_IMCBREAK) { /* Just cbreak mode */
        /* If input buffer is full, send bell to user */
        if (avail >= TY_IBUFLEN){
            eputc(typtr->tyifullc, typtr, csrptr);
        }
        else{ /* Input buffer has space for this char */
            *typtr->tyitail++ = ch;

            if (typtr->tyitail == &typtr->tyibuff[TY_IBUFLEN]){
                typtr->tyitail = typtr->tyibuff;
            }

            if (typtr->tyiecho){ /* Are we echoing chars? */
                echoch(ch, typtr, csrptr);
            }
            signal(typtr->tyisem);
        }
        return;
    }
    else{ /* Just cooked mode (see common code above) */

        /* Line kill character arrives - kill entire line */
        if (ch == typtr->tyikillc && typtr->tyikill) {
            typtr->tyitail -= typtr->tyicursor;
            if (typtr->tyitail < typtr->tyibuff){
                typtr->tyitail += TY_IBUFLEN;
            }
            typtr->tyicursor = 0;
            eputc(TY_RETURN, typtr, csrptr);
			eputc(TY_NEWLINE, typtr, csrptr);
            return;
        }

        /* Erase (backspace) character */

        if (((ch == typtr->tyierasec) || (ch == typtr->tyierasec2)) && typtr->tyierase){
            if (typtr->tyicursor > 0){ // at least one character to erase
                typtr->tyicursor--;
                erase1(typtr, csrptr);
            }
            return;
        }

        /* End of line */

        if ((ch == TY_NEWLINE) || (ch == TY_RETURN)){
            if (typtr->tyiecho) {
				echoch(ch, typtr, csrptr);
			}
			*typtr->tyitail++ = ch;
			if (typtr->tyitail >= &typtr->tyibuff[TY_IBUFLEN]) {
				typtr->tyitail = typtr->tyibuff;
			}
			/* Make entire line (plus \n or \r) available */
			signaln(typtr->tyisem, typtr->tyicursor + 1);
			typtr->tyicursor = 0; 	/* Reset for next line	*/
			return;
        }

        /* Character to be placed in buffer - send bell if	*/
		/* buffer has overflowed */

        if ((avail + typtr->tyicursor) >= TY_IBUFLEN - 1) {
			eputc(typtr->tyifullc, typtr, csrptr);
			return;
		}

        /* EOF character: recognize at beginning of line, but	*/
		/*	print and ignore otherwise.			*/

		if (ch == typtr->tyeofch && typtr->tyeof) {
			if (typtr->tyiecho) {
				echoch(ch, typtr, csrptr);
			}
			if (typtr->tyicursor != 0) {
				return;
			}
			*typtr->tyitail++ = ch;
			signal(typtr->tyisem);
			return;
		}

        /* Echo the character */
		if (typtr->tyiecho) {
			echoch(ch, typtr, csrptr);
		}

        /* Insert in the input buffer */
		typtr->tyicursor++;
		*typtr->tyitail++ = ch;

        /* Wrap around if needed */

		if (typtr->tyitail >= &typtr->tyibuff[TY_IBUFLEN]) {
			typtr->tyitail = typtr->tyibuff;
		}
		return;
    }
}