/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	$Id: key.h,v 8.18 1993/11/28 16:58:04 bostic Exp $ (Berkeley) $Date: 1993/11/28 16:58:04 $
 */

/* Structure for a key input buffer. */
struct _ibuf {
	char	*buf;		/* Buffer itself. */
	int	 cnt;		/* Count of characters. */
	int	 len;		/* Buffer length. */
	int	 next;		/* Offset of next character. */
};
				/* Flush keys from expansion buffer. */
#define	TERM_FLUSH(ibp)		(ibp)->cnt = (ibp)->next = 0
				/* Return if more keys in expansion buffer. */
#define	TERM_MORE(ibp)		((ibp)->cnt)

/*
 * Structure to name a character.  Used both as an interface to the screen
 * and to name objects referenced by characters in error messages.
 */
struct _chname {
	char	*name;		/* Character name. */
	u_char	 len;		/* Length of the character name. */
};

/* The maximum number of columns any character can take up on a screen. */
#define	MAX_CHARACTER_COLUMNS	4

/*
 * Routines that return a key as a side-effect return:
 *
 *	INP_OK		Returning a character; must be 0.
 *	INP_EOF		EOF.
 *	INP_ERR		Error.
 *
 * Routines that return a confirmation return:
 *
 *	CONF_NO		User answered no.
 *	CONF_QUIT	User answered quit, eof or an error.
 *	CONF_YES	User answered yes.
 *
 * The vi structure depends on the key routines being able to return INP_EOF
 * multiple times without failing -- eventually enough things will end due to
 * INP_EOF that vi will reach the command level for the screen, at which point
 * the exit flags will be set and vi will exit.
 */
enum confirm	{ CONF_NO, CONF_QUIT, CONF_YES };
enum input	{ INP_OK=0, INP_EOF, INP_ERR };

/*
 * Ex/vi commands are generally separated by whitespace characters.  We
 * can't use the standard isspace(3) macro because it returns true for
 * characters like ^K in the ASCII character set.  The 4.4BSD isblank(3)
 * macro does exactly what we want, but it's not portable yet.
 *
 * XXX
 * Note side effect, ch is evaluated multiple times.
 */
#ifndef isblank
#define	isblank(ch)	((ch) == ' ' || (ch) == '\t')
#endif

/* Special character lookup values. */
#define	K_CARAT		 1
#define	K_CNTRLR	 2
#define	K_CNTRLT	 3
#define	K_CNTRLZ	 4
#define	K_COLON	 	 5
#define	K_CR		 6
#define	K_ESCAPE	 7
#define	K_FORMFEED	 8
#define	K_NL		 9
#define	K_RIGHTBRACE	10
#define	K_RIGHTPAREN	11
#define	K_TAB		12
#define	K_VEOF	 	13
#define	K_VERASE	14
#define	K_VKILL		15
#define	K_VLNEXT	16
#define	K_VWERASE	17
#define	K_ZERO		18

/* Various special characters, messages. */
#define	CURSOR_CH	' '			/* Cursor character. */
#define	END_CH		'$'			/* End of a range. */
#define	HEX_CH		'x'			/* Leading hex number. */
#define	NOT_DIGIT_CH	'a'			/* A non-isdigit() character. */
#define	NO_CH		'n'			/* No. */
#define	QUIT_CH		'q'			/* Quit. */
#define	YES_CH		'y'			/* Yes. */
#define	CONFSTRING	"confirm? [ynq]"
#define	CONTMSG		"Enter return to continue: "
#define	CONTMSG_I	"Enter return to continue [q to quit]: "

/* Flags describing how input is handled. */
#define	TXT_AICHARS	0x000001	/* Leading autoindent chars. */
#define	TXT_ALTWERASE	0x000002	/* Option: altwerase. */
#define	TXT_APPENDEOL	0x000004	/* Appending after EOL. */
#define	TXT_AUTOINDENT	0x000008	/* Autoindent set this line. */
#define	TXT_BEAUTIFY	0x000010	/* Only printable characters. */
#define	TXT_BS		0x000020	/* Backspace returns the buffer. */
#define	TXT_CNTRLT	0x000040	/* Control-T is an indent special. */
#define	TXT_CR		0x000080	/* CR returns the buffer. */
#define	TXT_EMARK	0x000100	/* End of replacement mark. */
#define	TXT_ESCAPE	0x000200	/* Escape returns the buffer. */
#define	TXT_MAPCOMMAND	0x000400	/* Apply the command map. */
#define	TXT_MAPINPUT	0x000800	/* Apply the input map. */
#define	TXT_MAPNODIGIT	0x001000	/* Return to a digit. */
#define	TXT_NLECHO	0x002000	/* Echo the newline. */
#define	TXT_OVERWRITE	0x004000	/* Overwrite characters. */
#define	TXT_PROMPT	0x008000	/* Display a prompt. */
#define	TXT_RECORD	0x010000	/* Record for replay. */
#define	TXT_REPLACE	0x020000	/* Replace; don't delete overwrite. */
#define	TXT_REPLAY	0x040000	/* Replay the last input. */
#define	TXT_RESOLVE	0x080000	/* Resolve the text into the file. */
#define	TXT_SHOWMATCH	0x100000	/* Option: showmatch. */
#define	TXT_TTYWERASE	0x200000	/* Option: ttywerase. */
#define	TXT_WRAPMARGIN	0x400000	/* Option: wrapmargin. */

#define	TXT_VALID_EX							\
	(TXT_BEAUTIFY | TXT_CR | TXT_NLECHO | TXT_PROMPT)

#define	TXT_GETKEY_MASK							\
	(TXT_MAPCOMMAND | TXT_MAPINPUT)

/* Support keyboard routines. */
int	term_init __P((SCR *));
enum input
	term_key __P((SCR *, CHAR_T *, u_int));
enum input
	term_user_key __P((SCR *, CHAR_T *));
int	term_push __P((SCR *, IBUF *, char *, size_t));
int	term_waiting __P((SCR *));
