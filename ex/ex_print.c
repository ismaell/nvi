/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: ex_print.c,v 5.7 1992/04/19 08:53:57 bostic Exp $ (Berkeley) $Date: 1992/04/19 08:53:57 $";
#endif /* not lint */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>

#include "vi.h"
#include "curses.h"
#include "excmd.h"
#include "options.h"
#include "extern.h"

static int print __P((EXCMDARG *, int));

/*
 * ex_list -- :[line [,line]] l[ist] [count] [flags]
 *	Display the addressed lines such that the output is unambiguous.
 *	The only valid flag is '#'.
 */
int
ex_list(cmdp)
	EXCMDARG *cmdp;
{
	int flags;

	flags = cmdp->flags & E_F_MASK;
	if (flags & ~E_F_HASH) {
		msg("Usage: %s.", cmdp->cmd->usage);
		return (1);
	}
	return (print(cmdp, E_F_LIST | flags));
}

/*
 * ex_number -- :[line [,line]] nu[mber] [count] [flags]
 *	Display the addressed lines with a leading line number.
 *	The only valid flag is 'l'.
 */
int
ex_number(cmdp)
	EXCMDARG *cmdp;
{
	int flags;

	flags = cmdp->flags & E_F_MASK;
	if (flags & ~E_F_LIST) {
		msg("Usage: %s.", cmdp->cmd->usage);
		return (1);
	}
	return (print(cmdp, E_F_HASH | flags));
}

/*
 * ex_print -- :[line [,line]] p[rint] [count] [flags]
 *	Display the addressed lines.
 *	The only valid flags are '#' and 'l'.
 */
int
ex_print(cmdp)
	EXCMDARG *cmdp;
{
	int flags;

	flags = cmdp->flags & E_F_MASK;
	if (flags & ~(E_F_HASH | E_F_LIST)) {
		msg("Usage: %s.", cmdp->cmd->usage);
		return (1);
	}
	return (print(cmdp, E_F_PRINT | flags));
}

/*
 * print --
 *	Print the selected lines.
 */
static int
print(cmdp, flags)
	EXCMDARG *cmdp;
	register int flags;
{
	register long cur, end;
	register int ch, col, rlen;
	size_t len;
	int cnt;
	u_char *p;
	char buf[10];

	for (cur = markline(cmdp->addr1), end = markline(cmdp->addr2);
	    cur <= end; ++cur) {

		/* Display the line number. */
		if (flags & E_F_HASH) {
			(void)snprintf(buf, sizeof(buf), "%7ld ", cur);
			qaddstr(buf);
			col = 8;
		} else
			col = 0;
	
#define	WCHECK(ch) { \
	if (!has_AM && col == COLS) { \
		qaddch('\n'); \
		qaddch('\r'); \
		col = 0; \
	} \
	qaddch(ch); \
	++col; \
}
		/*
		 * Display the line.  The format for E_F_PRINT isn't very good,
		 * especially in handling end-of-line tabs, but they're almost
		 * backward compatible.
		 */
		p = (u_char *)fetchline(cur, &len);
		for (rlen = len; rlen--;) {
			ch = *p++;
			if (flags & E_F_LIST)
				if (ch != '\t' && isprint(ch)) {
					WCHECK(ch);
				} else if (ch & 0x80) {
					len = snprintf(buf,
					    sizeof(buf), "\\%03o", ch);
					for (cnt = 0; cnt < len; ++cnt)
						WCHECK(buf[cnt]);
				} else {
					WCHECK('^');
					WCHECK(ch + 0x40);
				}
			else {
				ch &= 0x7f;
				if (ch == '\t') {
					while (col < COLS &&
					    ++col % LVAL(O_TABSTOP))
						qaddch(' ');
					if (col == COLS) {
						qaddch('\n');
						qaddch('\r');
						col = 0;
					}
				} else if (isprint(ch)) {
					WCHECK(ch);
				} else if (ch == '\n') {
					qaddch('\n');
					qaddch('\r');
					col = 0;
				} else {
					WCHECK('^');
					WCHECK(ch + 0x40);
				}
			}
		}
		if (flags & E_F_LIST)
			WCHECK('$');
		qaddch('\n');
		qaddch('\r');
	}
	ex_refresh();
	cursor = cmdp->addr2;
	return (0);
}
