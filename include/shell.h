#pragma once
#include <tty.h>

#define SHELL_MAXTOK	32		/* Maximum tokens per line	*/

/* Shell banner (assumes VT100) */

#define	SHELL_BAN0	"\033[31;1m"
#define SHELL_BAN1      "------------------------------------------"
#define SHELL_BAN2      "   __    __   _____    _   _    _    _    "
#define SHELL_BAN3      "   \\ \\  / /  |__ __|  | \\ | |  | |  | |   "
#define SHELL_BAN4      "    \\ \\/ /     | |    |  \\| |  | |  | |   "
#define SHELL_BAN5      "    / /\\ \\    _| |_   | \\   |  | |  | |   "
#define SHELL_BAN6      "   / /  \\ \\  |     |  | | \\ |  \\  --  /   "
#define SHELL_BAN7      "   --    --   -----    -   -     ----     "
#define SHELL_BAN8      "------------------------------------------"
#define	SHELL_BAN9	"\033[0m\n"

#define SHELL_STRTMSG	"Welcome to Xinu!\n"    /* Welcome message */
#define SHELL_EXITMSG	"Shell closed\n"    /* Shell exit message */
#define SHELL_PROMPT	"xsh $ "	/* Command prompt		*/
#define SHELL_SYNERRMSG	"Syntax error\n" /* Syntax error message		*/

#define SHELL_BUFLEN	TY_IBUFLEN + 1	/* Length of input buffer	*/

/* Structure of an entry in the table of shell commands */

struct cmdent {			/* Entry in command table	*/
	char *  cname;			/* Name of command		*/
	bool8	cbuiltin;		/* Is this a builtin command?	*/
	int32	(*cfunc)(int32, char*[]); /* Function for command	*/
};


/* Constants used for lexical analysis */
#define	SH_NEWLINE	'\n'		/* New line character		*/
#define	SH_EOF		'\04'		/* Control-D is EOF		*/
#define	SH_AMPER	'&'		/* Ampersand character		*/
#define	SH_BLANK	' '		/* Blank character		*/
#define	SH_TAB		'\t'		/* Tab character		*/
#define	SH_SQUOTE	'\''		/* Single quote character	*/
#define	SH_DQUOTE	'"'		/* Double quote character	*/
#define	SH_LESS		'<'		/* Less-than character	*/
#define	SH_GREATER	'>'		/* Greater-than character	*/

/* Token types */

#define	SH_TOK_AMPER	0		/* Ampersand token		*/
#define	SH_TOK_LESS		1		/* Less-than token		*/
#define	SH_TOK_GREATER	2		/* Greater-than token		*/
#define	SH_TOK_OTHER	3		/* Token other than those listed above */


