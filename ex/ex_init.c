/*-
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1994, 1995
 *	Keith Bostic.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: ex_init.c,v 9.6 1995/01/11 16:15:34 bostic Exp $ (Berkeley) $Date: 1995/01/11 16:15:34 $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "compat.h"
#include <db.h>
#include <regex.h>

#include "vi.h"
#include "excmd.h"
#include "tag.h"

/*
 * ex_screen_copy --
 *	Copy ex screen.
 */
int
ex_screen_copy(orig, sp)
	SCR *orig, *sp;
{
	EX_PRIVATE *oexp, *nexp;

	/* Create the private ex structure. */
	CALLOC_RET(orig, nexp, EX_PRIVATE *, 1, sizeof(EX_PRIVATE));
	sp->ex_private = nexp;

	/* Initialize queues. */
	TAILQ_INIT(&nexp->tagq);
	TAILQ_INIT(&nexp->tagfq);
	TAILQ_INIT(&nexp->cdq);
	CIRCLEQ_INIT(&nexp->rangeq);

	if (orig == NULL) {
		nexp->at_lbuf_set = 0;
	} else {
		oexp = EXP(orig);

		nexp->at_lbuf = oexp->at_lbuf;
		nexp->at_lbuf_set = oexp->at_lbuf_set;

		if (oexp->lastbcomm != NULL &&
		    (nexp->lastbcomm = strdup(oexp->lastbcomm)) == NULL) {
			msgq(sp, M_SYSERR, NULL);
			return(1);
		}

		if (ex_tagcopy(orig, sp))
			return (1);
	}
	return (0);
}

/*
 * ex_screen_end --
 *	End a vi screen.
 */
int
ex_screen_end(sp)
	SCR *sp;
{
	EX_PRIVATE *exp;
	int rval;

	rval = 0;
	exp = EXP(sp);

	if (argv_free(sp))
		rval = 1;

	if (exp->ibp != NULL)
		FREE(exp->ibp, exp->ibp_len);

	if (exp->lastbcomm != NULL)
		FREE(exp->lastbcomm, strlen(exp->lastbcomm) + 1);

	if (ex_tagfree(sp))
		rval = 1;

	if (ex_cdfree(sp))
		rval = 1;

	/* Free private memory. */
	FREE(exp, sizeof(EX_PRIVATE));
	sp->ex_private = NULL;

	return (rval);
}

/*
 * ex_init --
 *	Initialize ex.
 */
int
ex_init(sp)
	SCR *sp;
{
	recno_t lno;
	size_t len;

	/*
	 * If the first visit to a file, check to see if we're skipping
	 * an initial comment.  Otherwise, make sure that the cursor
	 * position is a legal one.
	 */
	if (F_ISSET(sp->frp, FR_CURSORSET))
		if (file_gline(sp, sp->lno, &len) == NULL) {
			if (file_lline(sp, &lno))
				return (1);
			sp->lno = 1;
			sp->cno = 0;
		} else if (sp->cno >= len) {
			sp->cno = 0;
			if (nonblank(sp, sp->lno, &sp->cno))
				return (1);
		}
	else
		if (O_ISSET(sp, O_COMMENT) && ex_comment(sp))
			return (1);

	sp->stdfp = stdout;

	return (0);
}

/*
 * ex_end --
 *	End ex session.
 */
int
ex_end(sp)
	SCR *sp;
{
	return (0);
}

/*
 * ex_optchange --
 *	Handle change of options for vi.
 */
int
ex_optchange(sp, opt)
	SCR *sp;
	int opt;
{
	switch (opt) {
	case O_CDPATH:
		return (ex_cdalloc(sp, O_STR(sp, O_CDPATH)));
	case O_TAGS:
		return (ex_tagalloc(sp, O_STR(sp, O_TAGS)));
	}
	return (0);
}
