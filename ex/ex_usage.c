/*-
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: ex_usage.c,v 9.1 1994/11/09 18:41:16 bostic Exp $ (Berkeley) $Date: 1994/11/09 18:41:16 $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#include "compat.h"
#include <db.h>
#include <regex.h>

#include "vi.h"
#include "excmd.h"
#include "../vi/vcmd.h"

/*
 * ex_help -- :help
 *	Display help message.
 */
int
ex_help(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	F_SET(sp, S_SCR_EXWROTE);
	(void)ex_printf(EXCOOKIE,
	    "To see the list of vi commands, enter \":viusage<CR>\"\n");
	(void)ex_printf(EXCOOKIE,
	    "To see the list of ex commands, enter \":exusage<CR>\"\n");
	(void)ex_printf(EXCOOKIE,
	    "For an ex command usage statement enter \":exusage [cmd]<CR>\"\n");
	(void)ex_printf(EXCOOKIE,
	    "For a vi key usage statement enter \":viusage [key]<CR>\"\n");
	(void)ex_printf(EXCOOKIE, "To exit, enter \":q!\"\n");
	return (0);
}

/*
 * ex_usage -- :exusage [cmd]
 *	Display ex usage strings.
 */
int
ex_usage(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	ARGS *ap;
	EXCMDLIST const *cp;
	char *name;

	switch (cmdp->argc) {
	case 1:
		F_SET(sp, S_SCR_EXWROTE);
		ap = cmdp->argv[0];
		for (cp = cmds; cp->name != NULL &&
		    memcmp(ap->bp, cp->name, ap->len); ++cp);
		if (cp->name == NULL)
			(void)ex_printf(EXCOOKIE,
			    "The %.*s command is unknown",
			    (int)ap->len, ap->bp);
		else {
			(void)ex_printf(EXCOOKIE,
			    "Command: %s\n  Usage: %s\n", cp->help, cp->usage);
			/*
			 * !!!
			 * The "visual" command has two modes, one from ex,
			 * one from the vi colon line.  Don't ask.
			 */
			if (cp != &cmds[C_VISUAL_EX] &&
			    cp != &cmds[C_VISUAL_VI])
				break;
			if (cp == &cmds[C_VISUAL_EX])
				cp = &cmds[C_VISUAL_VI];
			else
				cp = &cmds[C_VISUAL_EX];
			(void)ex_printf(EXCOOKIE,
			    "Command: %s\n  Usage: %s\n", cp->help, cp->usage);
		}
		break;
	case 0:
		F_SET(sp, S_INTERRUPTIBLE | S_SCR_EXWROTE);
		for (cp = cmds; cp->name != NULL; ++cp) {
			/* The ^D command has an unprintable name. */
			if (cp == &cmds[C_SCROLL])
				name = "^D";
			else
				name = cp->name;
			(void)ex_printf(EXCOOKIE,
			    "%*s: %s\n", MAXCMDNAMELEN, name, cp->help);
		}
		break;
	default:
		abort();
	}
	return (0);
}

/*
 * ex_viusage -- :viusage [key]
 *	Display vi usage strings.
 */
int
ex_viusage(sp, cmdp)
	SCR *sp;
	EXCMDARG *cmdp;
{
	VIKEYS const *kp;
	int key;

	switch (cmdp->argc) {
	case 1:
		F_SET(sp, S_SCR_EXWROTE);
		if (cmdp->argv[0]->len != 1) {
			ex_message(sp, cmdp->cmd, EXM_USAGE);
			return (1);
		}
		key = cmdp->argv[0]->bp[0];
		if (key > MAXVIKEY)
			goto nokey;

		/* Special case: '[' and ']' commands. */
		if ((key == '[' || key == ']') && cmdp->argv[0]->bp[1] != key)
			goto nokey;

		/* Special case: ~ command. */
		if (key == '~' && O_ISSET(sp, O_TILDEOP))
			kp = &tmotion;
		else
			kp = &vikeys[key];

		if (kp->func == NULL)
nokey:			(void)ex_printf(EXCOOKIE,
			    "The %s key has no current meaning",
			    KEY_NAME(sp, key));
		else
			(void)ex_printf(EXCOOKIE,
			    "  Key:%s%s\nUsage: %s\n",
			    isblank(*kp->help) ? "" : " ", kp->help, kp->usage);
		break;
	case 0:
		F_SET(sp, S_INTERRUPTIBLE | S_SCR_EXWROTE);
		for (key = 0; key <= MAXVIKEY; ++key) {
			/* Special case: ~ command. */
			if (key == '~' && O_ISSET(sp, O_TILDEOP))
				kp = &tmotion;
			else
				kp = &vikeys[key];
			if (kp->help != NULL)
				(void)ex_printf(EXCOOKIE, "%s\n", kp->help);
		}
		break;
	default:
		abort();
	}
	return (0);
}
