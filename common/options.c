/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: options.c,v 5.13 1992/05/04 11:52:25 bostic Exp $ (Berkeley) $Date: 1992/05/04 11:52:25 $";
#endif /* not lint */

#include <sys/param.h>
#include <errno.h>
#include <curses.h>
#include <stdlib.h>
#include <strings.h>
#include <paths.h>
#include <stdio.h>

#include "vi.h"
#include "excmd.h"
#include "exf.h"
#include "options.h"
#include "tty.h"
#include "extern.h"

static int opts_abbcmp __P((const void *, const void *));
static int opts_cmp __P((const void *, const void *));
static int opts_print __P((struct _option *));

/*
 * First array slot is the current value, second and third are the low and
 * and high ends of the range.
 *
 * XXX
 * Some of the limiting values are clearly randomly chosen, and have no
 * meaning.  How make O_REPORT just shut up?
 */
static long o_columns[3] = {80, 32, 255};
static long o_keytime[3] = {2, 0, 50};
static long o_lines[3] = {25, 2, 66};
static long o_report[3] = {5, 1, 127};
static long o_scroll[3] = {12, 1, 127};
static long o_shiftwidth[3] = {8, 1, 255};
static long o_sidescroll[3] = {16, 1, 64};
static long o_tabstop[3] = {8, 1, 40};
static long o_taglength[3] = {0, 0, 30};
static long o_window[3] = {24, 1, 24};
static long o_wrapmargin[3] = {0, 0, 255};

/* START_SED_INCLUDE */
OPTIONS opts[] = {
#define	O_AUTOINDENT	0
	"autoindent",	NULL,		OPT_0BOOL,
#define	O_AUTOPRINT	1
	"autoprint",	NULL,		OPT_1BOOL,
#define	O_AUTOTAB	2
	"autotab",	NULL,		OPT_1BOOL,
#define	O_AUTOWRITE	3
	"autowrite",	NULL,		OPT_0BOOL,
#define	O_BEAUTIFY	4
	"beautify",	NULL,		OPT_0BOOL,
#define	O_CC		5
	"cc",		"cc -c",	OPT_STR,
#define	O_COLUMNS	6
	"columns",	&o_columns,	OPT_NOSAVE|OPT_NUM|OPT_REDRAW,
#define	O_DIGRAPH	7
	"digraph",	NULL,		OPT_0BOOL,
#define	O_DIRECTORY	8
	"directory",	_PATH_TMP,	OPT_NOSAVE|OPT_STR,
#define	O_EDCOMPATIBLE	9
	"edcompatible",	NULL,		OPT_0BOOL,
#define	O_EQUALPRG	10
	"equalprg",	"fmt",		OPT_STR,
#define	O_ERRORBELLS	11
	"errorbells",	NULL,		OPT_1BOOL,
#define	O_EXRC		12
	"exrc",		NULL,		OPT_0BOOL,
#define	O_EXREFRESH	13
	"exrefresh",	NULL,		OPT_1BOOL,
#define	O_FLASH		14
	"flash",	NULL,		OPT_1BOOL,
#define	O_IGNORECASE	15
	"ignorecase",	NULL,		OPT_0BOOL,
#define	O_KEYTIME	16
	"keytime",	&o_keytime,	OPT_NUM,
#define	O_LINES		17
	"lines",	&o_lines,	OPT_NOSAVE|OPT_NUM|OPT_REDRAW,
#define	O_LIST		18
	"list",		NULL,		OPT_0BOOL|OPT_REDRAW,
#define	O_MAGIC		19
	"magic",	NULL,		OPT_1BOOL,
#define	O_MAKE		20
	"make",		"make",		OPT_STR,
#define	O_MESG		21
	"mesg",		NULL,		OPT_1BOOL,
#define	O_NUMBER	22
	"number",	NULL,		OPT_0BOOL|OPT_REDRAW,
#define	O_PARAGRAPHS	23
	"paragraphs",	"PPppIPLPQP",	OPT_STR,
#define	O_PROMPT	24
	"prompt",	NULL,		OPT_1BOOL,
#define	O_READONLY	25
	"readonly",	NULL,		OPT_0BOOL,
#define	O_REPORT	26
	"report",	&o_report,	OPT_NUM,
#define	O_RULER		27
	"ruler",	NULL,		OPT_0BOOL,
#define	O_SCROLL	28
	"scroll",	&o_scroll,	OPT_NUM,
#define	O_SECTIONS	29
	"sections",	"NHSHSSSEse",	OPT_STR,
#define	O_SHELL		30
	"shell",	_PATH_BSHELL,	OPT_STR,
#define	O_SHIFTWIDTH	31
	"shiftwidth",	&o_shiftwidth,	OPT_NUM,
#define	O_SHOWMATCH	32
	"showmatch",	NULL,		OPT_0BOOL,
#define	O_SHOWMODE	33
	"showmode",	NULL,		OPT_0BOOL,
#define	O_SIDESCROLL	34
	"sidescroll",	&o_sidescroll,	OPT_NUM,
#define	O_SYNC		35
	"sync",		NULL,		OPT_0BOOL,
#define	O_TABSTOP	36
	"tabstop",	&o_tabstop,	OPT_NUM|OPT_REDRAW,
#define	O_TAGLENGTH	37
	"taglength",	&o_taglength,	OPT_NUM,
#define	O_TERM		38
	"term",		"unknown",	OPT_NOSAVE|OPT_STR,
#define	O_TERSE		39
	"terse",	NULL,		OPT_0BOOL,
#define	O_TIMEOUT	40
	"timeout",	NULL,		OPT_0BOOL,
#define	O_VBELL		41
	"vbell",	NULL,		OPT_0BOOL,
#define	O_WARN		42
	"warn",		NULL,		OPT_1BOOL,
#define	O_WINDOW	43
	"window",	&o_window,	OPT_NUM|OPT_REDRAW,
#define	O_WRAPMARGIN	44
	"wrapmargin",	&o_wrapmargin,	OPT_NUM,
#define	O_WRAPSCAN	45
	"wrapscan",	NULL,		OPT_1BOOL,
#define	O_WRITEANY	46
	"writeany",	NULL,		OPT_0BOOL,
	NULL,
};
#define	O_OPTIONCOUNT	47
/* END_SED_INCLUDE */

typedef struct abbrev {
        char *name;
        int offset;
} ABBREV;

static ABBREV abbrev[] = {
	"ai",	O_AUTOINDENT,
	"ap",	O_AUTOPRINT,
	"at",	O_AUTOTAB,
	"aw",	O_AUTOWRITE,
	"bf",	O_BEAUTIFY,
	"cc",	O_CC,
	"co",	O_COLUMNS,
	"dig",	O_DIGRAPH,
	"dir",	O_DIRECTORY,
	"eb",	O_ERRORBELLS,
	"ed",	O_EDCOMPATIBLE,
	"ep",	O_EQUALPRG,
	"er",	O_EXREFRESH,
	"fl",	O_VBELL,
	"ic",	O_IGNORECASE,
	"kt",	O_KEYTIME,
	"li",	O_LIST,
	"ls",	O_LINES,
	"ma",	O_MAGIC,
	"me",	O_MESG,
	"mk",	O_MAKE,
	"nu",	O_NUMBER,
	"pa",	O_PARAGRAPHS,
	"pr",	O_PROMPT,
	"re",	O_REPORT,
	"ro",	O_READONLY,
	"ru",	O_RULER,
	"sc",	O_SCROLL,
	"se",	O_SECTIONS,
	"sh",	O_SHELL,
	"sm",	O_SHOWMATCH,
	"ss",	O_SIDESCROLL,
	"sw",	O_SHIFTWIDTH,
	"sy",	O_SYNC,
	"te",	O_TERM,
	"tl",	O_TAGLENGTH,
	"to",	O_KEYTIME,
	"tr",	O_TERSE,
	"ts",	O_TABSTOP,
	"vb",	O_VBELL,
	"wa",	O_WARN,
	"wi",	O_WINDOW,
	"wm",	O_WRAPMARGIN,
	"wr",	O_WRITEANY,
	"ws",	O_WRAPSCAN,
	NULL,
};

/*
 * opts_init --
 *	Initialize some of the options.  Since the user isn't really "setting"
 *	these variables, we don't set their OPT_SET bits.
 */
void
opts_init()
{
	char *val;

	if (val = getenv("COLUMNS"))
		COLS = atoi(val);
	LVAL(O_COLUMNS) = COLS;

	if (val = getenv("LINES"))
		LINES = atoi(val);
	LVAL(O_LINES) = LINES;
	LVAL(O_SCROLL) = LINES / 2 - 1;
	LVAL(O_WINDOW) = LINES - 1;

	if (val = getenv("SHELL")) {
		PVAL(O_SHELL) = strdup(val);
		FSET(O_SHELL, OPT_ALLOCATED);
	}

	/* Disable the vbell option if we don't know how to do a vbell. */
	if (!VB) {
		FSET(O_FLASH, OPT_NOSET);
		FSET(O_VBELL, OPT_NOSET);
	}
}

/*
 * opts_set --
 *	Change the values of one or more options.
 */
void
opts_set(argv)
	char **argv;
{
	register char *p;
	ABBREV atmp, *ap;
	OPTIONS otmp, *op;
	long value;
	int all, ch, needredraw, off;
	char *ep, *equals, *name;
	
	/*
	 * Reset the upper limit of "window" option to lines - 1.
	 * XXX -- Why are we doing this?
	 */
	LVAL(O_WINDOW) = LINES - 1;

	for (all = needredraw = 0; *argv; ++argv) {
		/*
		 * The historic vi dumped the options for each occurrence of
		 * "all" in the set list.  Stupid.
		 */
		if (!strcmp(*argv, "all")) {
			all = 1;
			continue;
		}
			
		/* Find equals sign or end of set, skipping backquoted chars. */
		for (p = name = *argv, equals = NULL; ch = *p; ++p)
			switch(ch) {
			case '=':
				equals = p;
				break;
			case '\\':
				/* Historic vi just used the backslash. */
				if (p[1] == '\0')
					break;
				++p;
				break;
			}

		off = 0;
		op = NULL;
		if (equals)
			*equals++ = '\0';

		/* Check list of abbreviations. */
		atmp.name = name;
		if ((ap = bsearch(&atmp, abbrev,
		    sizeof(abbrev) / sizeof(ABBREV) - 1,
		    sizeof(ABBREV), opts_abbcmp)) != NULL) {
			op = opts + ap->offset;
			goto found;
		}

		/* Check list of options. */
		otmp.name = name;
		if ((op = bsearch(&otmp, opts,
		    sizeof(opts) / sizeof(OPTIONS) - 1,
		    sizeof(OPTIONS), opts_cmp)) != NULL)
			goto found;

		/* Try the name without any leading "no". */
		if (name[0] == 'n' && name[1] == 'o') {
			off = 1;
			name += 2;
		} else
			goto found;

		/* Check list of abbreviations. */
		atmp.name = name;
		if ((ap = bsearch(&atmp, abbrev,
		    sizeof(abbrev) / sizeof(ABBREV) - 1,
		    sizeof(ABBREV), opts_abbcmp)) != NULL) {
			op = opts + ap->offset;
			goto found;
		}

		/* Check list of options. */
		otmp.name = name;
		op = bsearch(&otmp, opts,
		    sizeof(opts) / sizeof(OPTIONS) - 1,
		    sizeof(OPTIONS), opts_cmp);

found:		if (op == NULL || off && !ISFSETP(op, OPT_0BOOL|OPT_1BOOL)) {
			msg("no option %s: 'set all' gives all option values",
			    name);
			continue;
		}

		/* Set name, value. */
		if (ISFSETP(op, OPT_NOSET)) {
			msg("%s: may not be set", name);
			continue;
		}

		switch (op->flags & OPT_TYPE) {
		case OPT_0BOOL:
		case OPT_1BOOL:
			if (equals)
				msg("set: option [no]%s is a boolean", name);
			else {
				FUNSETP(op, OPT_0BOOL | OPT_1BOOL);
				FSETP(op,
				    (off ? OPT_0BOOL : OPT_1BOOL) | OPT_SET);
				needredraw |= ISFSETP(op, OPT_REDRAW);
			}
			break;
		case OPT_NUM:
			if (!equals) {
				msg("set: option [no]%s requires a value",
				    name);
				break;
			}
			value = strtol(equals, &ep, 10);
			if (*ep && !isspace(*ep)) {
				msg("set %s: illegal number %s", name, equals);
				break;
			}
			if (value < MINLVALP(op)) {
				msg("set %s: min value is %ld",
				    name, MINLVALP(op));
				break;
			}
			if (value > MAXLVALP(op)) {
				msg("set %s: max value is %ld",
				    name, MAXLVALP(op));
				break;
			}
			LVALP(op) = value;
			FSETP(op, OPT_SET);
			needredraw |= ISFSETP(op, OPT_REDRAW);
			break;
		case OPT_STR:
			if (!equals) {
				msg("set: option [no]%s requires a value",
				    name);
				break;
			}
			if (ISFSETP(op, OPT_ALLOCATED))
				free(PVALP(op));
			PVALP(op) = strdup(equals);
			FSETP(op, OPT_ALLOCATED | OPT_SET);
			needredraw |= ISFSETP(op, OPT_REDRAW);
			break;
		}
	}

	/* Special processing. */

	/*
	 * If "readonly" then set the READONLY flag for this file.
	 * XXX
	 * Should set for all files?
	 */
	if (ISSET(O_READONLY))
		curf->flags |= F_RDONLY;

	/*
	 * Copy O_LINES and O_COLUMNS into LINES and COLS.
	 * XXX
	 * This isn't going to work if the screen is already in place.
	 * Need a window call to shut it down and restart it, but that
	 * means that we'll have to put these in the environment.
	 */
	LINES = LVAL(O_LINES);
	COLS = LVAL(O_COLUMNS);

	if (all)
		opts_dump(1);

	if (needredraw)
		touchwin(stdscr);

	/*
	 * That option may have affected the appearance of text.
	 * XXX
	 * Why isn't this set per variable, like redraw?
	 */
	++changes;
}

/*
 * opt_dump --
 *	List the current values of selected options.
 */
void
opts_dump(all)
	int all;
{
	OPTIONS *op;
	int base, b_num, chcnt, cnt, col, colwidth, curlen, endcol, s_num;
	int numcols, numrows, row, termwidth;
	int b_op[O_OPTIONCOUNT], s_op[O_OPTIONCOUNT];
	char nbuf[20];

	/*
	 * Options are output in two groups -- those that fit at least two to
	 * a line and those that don't.  We do output on tab boundaries for no
	 * particular reason.   First get the set of options to list, keeping
	 * track of the length of each.  No error checking, because we know
	 * that O_TERM was set so at least one option has the OPT_SET bit on.
	 * Termwidth is the tab stop before half of the line in the first loop,
	 * and the full line length later on.
	 */
	colwidth = -1;
	termwidth = (COLS - 1) / 2 & ~(TAB - 1);
	for (b_num = s_num = 0, op = opts; op->name; ++op) {
		if (!all && !ISFSETP(op, OPT_SET))
			continue;
		cnt = op - opts;
		curlen = strlen(op->name);
		switch (op->flags & OPT_TYPE) {
		case OPT_0BOOL:
			curlen += 2;
			break;
		case OPT_1BOOL:
			break;
		case OPT_NUM:
			curlen +=
			    snprintf(nbuf, sizeof(nbuf), "%ld", LVAL(cnt));
			break;
		case OPT_STR:
			curlen += strlen(PVAL(cnt)) + 3;
			break;
		}
		if (curlen < termwidth) {
			if (colwidth < curlen)
				colwidth = curlen;
			s_op[s_num++] = cnt;
		} else
			b_op[b_num++] = cnt;
	}

	colwidth = (colwidth + TAB) & ~(TAB - 1);
	termwidth = COLS - 1;
	numcols = termwidth / colwidth;
	if (s_num > numcols) {
		numrows = s_num / numcols;
		if (s_num % numcols)
			++numrows;
	} else
		numrows = 1;

	EX_PRSTART(1);
	for (row = 0; row < numrows;) {
		endcol = colwidth;
		for (base = row, chcnt = col = 0; col < numcols; ++col) {
			chcnt += opts_print(&opts[s_op[base]]);
			if ((base += numrows) >= s_num)
				break;
			while ((cnt = (chcnt + TAB & ~(TAB - 1))) <= endcol) {
				(void)putchar('\t');
				chcnt = cnt;
			}
			endcol += colwidth;
		}
		if (++row < numrows)
			EX_PRNEWLINE;
	}

	for (row = 0; row < b_num;) {
		(void)opts_print(&opts[b_op[row]]);
		++row;
		if (numrows || row < b_num)
			EX_PRNEWLINE;
	}
	EX_PRTRAIL;
}

/*
 * opts_save --
 *	Write the current configuration to a file.
 */
void
opts_save(fp)
	FILE *fp;
{
	OPTIONS *op;

	for (op = opts; op->name; ++op) {
		if (ISFSETP(op, OPT_NOSAVE))
			continue;
		switch (op->flags & OPT_TYPE) {
		case OPT_0BOOL:
			(void)fprintf(fp, "set no%s\n", op->name);
			break;
		case OPT_1BOOL:
			(void)fprintf(fp, "set %s\n", op->name);
			break;
		case OPT_NUM:
			(void)fprintf(fp,
			    "set %s=%-3d\n", op->name, LVALP(op));
			break;
		case OPT_STR:
			(void)fprintf(fp,
			    "set %s=\"%s\"\n", op->name, PVALP(op));
			break;
		}
	}
}

/*
 * opt_print --
 *	Print out an option.
 */
static int
opts_print(op)
	OPTIONS *op;
{
	int curlen;
	char nbuf[20];

	curlen = 0;
	switch (op->flags & OPT_TYPE) {
	case OPT_0BOOL:
		curlen += 2;
		(void)putchar('n');
		(void)putchar('o');
		/* FALLTHROUGH */
	case OPT_1BOOL:
		curlen += printf("%s", op->name);
		break;
	case OPT_NUM:
		curlen += printf("%s", op->name);
		curlen += 1;
		(void)putchar('=');
		curlen += printf("%ld", LVALP(op));
		break;
	case OPT_STR:
		curlen += printf("%s", op->name);
		curlen += 1;
		(void)putchar('=');
		curlen += 1;
		(void)putchar('"');
		curlen += printf("%s", PVALP(op));
		curlen += 1;
		(void)putchar('"');
		break;
	}
	return (curlen);
}

opts_abbcmp(a, b)
        const void *a, *b;
{
        return(strcmp(((ABBREV *)a)->name, ((ABBREV *)b)->name));
}

opts_cmp(a, b)
        const void *a, *b;
{
        return(strcmp(((OPTIONS *)a)->name, ((OPTIONS *)b)->name));
}
