/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1994, 1995
 *	Keith Bostic.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: ex_args.c,v 9.4 1995/01/11 16:15:16 bostic Exp $ (Berkeley) $Date: 1995/01/11 16:15:16 $";
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

/*
 * ex_next -- :next [+cmd] [files]
 *	Edit the next file, optionally setting the list of files.
 *
 * !!!
 * The :next command behaved differently from the :rewind command in
 * historic vi.  See nvi/docs/autowrite for details, but the basic
 * idea was that it ignored the force flag if the autowrite flag was
 * set.  This implementation handles them all identically.
 */
int
ex_next(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	ARGS **argv;
	FREF *frp;
	int noargs;
	char **ap;

	if (file_m1(sp, F_ISSET(cmdp, E_FORCE), FS_ALL | FS_POSSIBLE))
		return (1);

	/* Any other arguments are a replacement file list. */
	if (cmdp->argc) {
		/* Free the current list. */
		if (!F_ISSET(sp, S_ARGNOFREE) && sp->argv != NULL) {
			for (ap = sp->argv; *ap != NULL; ++ap)
				free(*ap);
			free(sp->argv);
		}
		F_CLR(sp, S_ARGNOFREE | S_ARGRECOVER);
		sp->cargv = NULL;

		/* Create a new list. */
		CALLOC_RET(sp,
		    sp->argv, char **, cmdp->argc + 1, sizeof(char *));
		for (ap = sp->argv,
		    argv = cmdp->argv; argv[0]->len != 0; ++ap, ++argv)
			if ((*ap =
			    v_strdup(sp, argv[0]->bp, argv[0]->len)) == NULL)
				return (1);
		*ap = NULL;

		/* Switch to the first one. */
		sp->cargv = sp->argv;
		if ((frp = file_add(sp, *sp->cargv)) == NULL)
			return (1);
		noargs = 0;
	} else {
		if (sp->cargv == NULL || sp->cargv[1] == NULL) {
			msgq(sp, M_ERR, "120|No more files to edit");
			return (1);
		}
		if ((frp = file_add(sp, sp->cargv[1])) == NULL)
			return (1);
		if (F_ISSET(sp, S_ARGRECOVER))
			F_SET(frp, FR_RECOVER);
		noargs = 1;
	}

	if (file_init(sp, frp, NULL,
	    FS_SETALT | (F_ISSET(cmdp, E_FORCE) ? FS_FORCE : 0)))
		return (1);
	(void)msg_status(sp, sp->lno, 0);
	if (noargs)
		++sp->cargv;
	return (0);
}

/*
 * ex_prev -- :prev
 *	Edit the previous file.
 */
int
ex_prev(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	FREF *frp;

	if (file_m1(sp, F_ISSET(cmdp, E_FORCE), FS_ALL | FS_POSSIBLE))
		return (1);

	if (sp->cargv == sp->argv) {
		msgq(sp, M_ERR, "121|No previous files to edit");
		return (1);
	}
	if ((frp = file_add(sp, sp->cargv[-1])) == NULL)
		return (1);

	if (file_init(sp, frp, NULL,
	    FS_SETALT | (F_ISSET(cmdp, E_FORCE) ? FS_FORCE : 0)))
		return (1);
	(void)msg_status(sp, sp->lno, 0);
	--sp->cargv;

	return (0);
}

/*
 * ex_rew -- :rew
 *	Re-edit the list of files.
 */
int
ex_rew(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	FREF *frp;

	/*
	 * !!!
	 * Historic practice -- you can rewind to the current file.
	 */
	if (sp->argv == NULL) {
		msgq(sp, M_ERR, "122|No previous files to rewind");
		return (1);
	}

	if (file_m1(sp, F_ISSET(cmdp, E_FORCE), FS_ALL | FS_POSSIBLE))
		return (1);

	/*
	 * !!!
	 * Historic practice, all files start at the beginning of the
	 * file.
	 */
	for (frp = sp->frefq.cqh_first;
	    frp != (FREF *)&sp->frefq; frp = frp->q.cqe_next)
		F_CLR(frp, FR_CURSORSET | FR_FNONBLANK);

	/* Switch to the first one. */
	sp->cargv = sp->argv;
	if ((frp = file_add(sp, *sp->cargv)) == NULL)
		return (1);
	if (file_init(sp, frp, NULL,
	    FS_SETALT | (F_ISSET(cmdp, E_FORCE) ? FS_FORCE : 0)))
		return (1);
	(void)msg_status(sp, sp->lno, 0);
	return (0);
}

/*
 * ex_args -- :args
 *	Display the list of files.
 */
int
ex_args(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	int cnt, col, len, sep;
	char **ap;

	if (sp->argv == NULL) {
		(void)msgq(sp, M_ERR, "263|No file list to display");
		return (0);
	}

	col = len = sep = 0;
	for (cnt = 1, ap = sp->argv; *ap != NULL; ++ap) {
		col += len = strlen(*ap) + sep + (ap == sp->cargv ? 2 : 0);
		if (col >= sp->cols - 1) {
			col = len;
			sep = 0;
			(void)ex_printf(EXCOOKIE, "\n");
		} else if (cnt != 1) {
			sep = 1;
			(void)ex_printf(EXCOOKIE, " ");
		}
		++cnt;

		if (ap == sp->cargv)
			(void)ex_printf(EXCOOKIE, "[%s]", *ap);
		else
			(void)ex_printf(EXCOOKIE, "%s", *ap);
		if (INTERRUPTED(sp))
			break;
	}
	if (!INTERRUPTED(sp))
		(void)ex_printf(EXCOOKIE, "\n");
	F_SET(sp, S_SCR_EXWROTE);

	return (0);
}
