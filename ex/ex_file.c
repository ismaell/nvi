/*-
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: ex_file.c,v 9.1 1994/11/09 18:40:43 bostic Exp $ (Berkeley) $Date: 1994/11/09 18:40:43 $";
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
 * ex_file -- :f[ile] [name]
 *	Change the file's name and display the status line.
 */
int
ex_file(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	CHAR_T *p;
	FREF *frp;

	NEEDFILE(sp, cmdp->cmd);

	switch (cmdp->argc) {
	case 0:
		break;
	case 1:
		frp = sp->frp;

		/* Make sure can allocate enough space. */
		if ((p = v_strdup(sp,
		    cmdp->argv[0]->bp, cmdp->argv[0]->len)) == NULL)
			return (1);

		/* If already have a file name, it becomes the alternate. */
		if (!F_ISSET(frp, FR_TMPFILE))
			set_alt_name(sp, frp->name);

		/* Free the previous name. */
		free(frp->name);
		frp->name = p;

		/*
		 * The read-only bit follows the file name; clear it.
		 * The file has a real name, it's no longer a temporary.
		 */
		F_CLR(frp, FR_RDONLY | FR_TMPFILE);

		/* Have to force a write if the file exists, next time. */
		F_SET(frp, FR_NAMECHANGE);
		break;
	default:
		abort();
	}
	return (msg_status(sp, sp->lno, 1));
}
