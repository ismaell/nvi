/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	$Id: common.h,v 8.34 1994/02/25 18:55:45 bostic Exp $ (Berkeley) $Date: 1994/02/25 18:55:45 $
 */

/* System includes. */
#include <sys/queue.h>		/* Required by screen.h. */
#include <sys/time.h>		/* Required by screen.h. */

#include <bitstring.h>		/* Required by screen.h. */
#include <limits.h>		/* Required by screen.h. */
#include <signal.h>		/* Required by screen.h. */
#include <stdio.h>		/* Required by screen.h. */
#include <termios.h>		/* Required by gs.h. */

/*
 * Required by screen.h.  This is the first include that can pull
 * in "compat.h".  Should be after every other system include.
 */
#include <regex.h>

/*
 * Forward structure declarations.  Not pretty, but the include files
 * are far too interrelated for a clean solution.
 */
typedef struct _cb		CB;
typedef struct _ch		CH;
typedef struct _chname		CHNAME;
typedef struct _excmdarg	EXCMDARG;
typedef struct _exf		EXF;
typedef struct _fref		FREF;
typedef struct _gs		GS;
typedef struct _ibuf		IBUF;
typedef struct _lmark		LMARK;
typedef struct _mark		MARK;
typedef struct _msg		MSG;
typedef struct _option		OPTION;
typedef struct _optlist		OPTLIST;
typedef struct _scr		SCR;
typedef struct _script		SCRIPT;
typedef struct _seq		SEQ;
typedef struct _tag		TAG;
typedef struct _tagf		TAGF;
typedef struct _text		TEXT;

/*
 * Fundamental character types.
 *
 * CHAR_T	An integral type that can hold any character.
 * ARG_CHAR_T	The type of a CHAR_T when passed as an argument using
 *		traditional promotion rules.  It should also be able
 *		to be compared against any CHAR_T for equality without
 *		problems.
 * MAX_CHAR_T	The maximum value of any character.
 *
 * If no integral type can hold a character, don't even try the port.
 */
typedef	u_char		CHAR_T;	
typedef	u_int		ARG_CHAR_T;
#define	MAX_CHAR_T	0xff

/* The maximum number of columns any character can take up on a screen. */
#define	MAX_CHARACTER_COLUMNS	4

/*
 * Local includes.
 */
#include <db.h>			/* Required by exf.h; includes compat.h. */

#include "search.h"		/* Required by screen.h. */
#include "args.h"		/* Required by options.h. */
#include "options.h"		/* Required by screen.h. */
#include "term.h"		/* Required by screen.h. */

#include "msg.h"		/* Required by gs.h. */
#include "cut.h"		/* Required by gs.h. */
#include "gs.h"			/* Required by screen.h. */
#include "screen.h"		/* Required by exf.h. */
#include "mark.h"		/* Required by exf.h. */
#include "exf.h"		
#include "log.h"
#include "mem.h"

#if FWOPEN_NOT_AVAILABLE	/* See PORT/clib/fwopen.c. */
#define	EXCOOKIE	sp
int	 ex_fflush __P((SCR *));
int	 ex_printf __P((SCR *, const char *, ...));
FILE	*fwopen __P((SCR *, void *));
#else
#define	EXCOOKIE	sp->stdfp
#define	ex_fflush	fflush
#define	ex_printf	fprintf
#endif

/* Macros to set/clear/test flags. */
#define	F_SET(p, f)	(p)->flags |= (f)
#define	F_CLR(p, f)	(p)->flags &= ~(f)
#define	F_ISSET(p, f)	((p)->flags & (f))

#define	LF_INIT(f)	flags = (f)
#define	LF_SET(f)	flags |= (f)
#define	LF_CLR(f)	flags &= ~(f)
#define	LF_ISSET(f)	(flags & (f))

/*
 * XXX
 * MIN/MAX have traditionally been in <sys/param.h>.  Don't
 * try to get them from there, it's just not worth the effort.
 */
#ifndef	MAX
#define	MAX(_a,_b)	((_a)<(_b)?(_b):(_a))
#endif
#ifndef	MIN
#define	MIN(_a,_b)	((_a)<(_b)?(_a):(_b))
#endif

/* Function prototypes that don't seem to belong anywhere else. */
u_long	 baud_from_bval __P((SCR *));
char	*charname __P((SCR *, ARG_CHAR_T));
void	 busy_off __P((SCR *));
void	 busy_on __P((SCR *, int, char const *));
int	 nonblank __P((SCR *, EXF *, recno_t, size_t *));
void	 set_alt_name __P((SCR *, char *));
int	 set_window_size __P((SCR *, u_int, int));
int	 status __P((SCR *, EXF *, recno_t, int));
char	*tail __P((char *));
CHAR_T	*v_strdup __P((SCR *, CHAR_T *, size_t));

#ifdef DEBUG
void	TRACE __P((SCR *, const char *, ...));
#endif

/* Digraphs (not currently real). */
int	digraph __P((SCR *, int, int));
int	digraph_init __P((SCR *));
void	digraph_save __P((SCR *, int));
