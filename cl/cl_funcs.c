/*-
 * Copyright (c) 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1994, 1995
 *	Keith Bostic.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: cl_funcs.c,v 10.4 1995/06/15 14:51:03 bostic Exp $ (Berkeley) $Date: 1995/06/15 14:51:03 $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <ctype.h>
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "compat.h"
#include <db.h>
#include <regex.h>

#include "common.h"
#include "cl.h"
#include "vi.h"

static int cl_lline_copy __P((SCR *, size_t *, CHAR_T **, size_t *));

/*
 * cl_addstr --
 *	Add the string at the cursor, advancing the cursor.
 *
 * PUBLIC: int cl_addstr __P((SCR *, const char *));
 */
int
cl_addstr(sp, str)
	SCR *sp;
	const char *str;
{
	return (cl_addnstr(sp, str, strlen(str)));
}

/*
 * cl_addnstr --
 *	Add len bytes from the string at the cursor, advancing the cursor.
 *
 * PUBLIC: int cl_addnstr __P((SCR *, const char *, size_t));
 */
int
cl_addnstr(sp, str, len)
	SCR *sp;
	const char *str;
	size_t len;
{
	CL_PRIVATE *clp;
	size_t oldx, oldy;
	int iv, rval;

	EX_ABORT(sp);
	VI_INIT_IGNORE(sp);

	/*
	 * If the last line:
	 *	If a busy message already there, discard the busy message.
	 *	if a split screen, use inverse video.
	 */
	iv = 0;
	clp = CLP(sp);
	getyx(stdscr, oldy, oldx);
	if (oldy == RLNO(sp, INFOLINE(sp))) {
		if (clp->busy_state == BUSY_ON)
			clp->busy_state = BUSY_SILENT;
		if (IS_SPLIT(sp)) {
			iv = 1;
			F_SET(clp, CL_LLINE_IV);
		}
	}
	if (iv)
		(void)standout();
	if (rval = (addnstr(str, len) == ERR))
		msgq(sp, M_ERR, "Error: addstr/addnstr: %.*s", (int)len, str);
	if (iv)
		(void)standend();
	return (rval);
}

/*
 * cl_attr --
 *	Toggle a screen attribute on/off.
 *
 * PUBLIC: int cl_attr __P((SCR *, scr_attr_t, int));
 */
int
cl_attr(sp, attribute, on)
	SCR *sp;
	scr_attr_t attribute;
	int on;
{
	CL_PRIVATE *clp;

	EX_INIT_IGNORE(sp);
	VI_INIT_IGNORE(sp);

	clp = CLP(sp);
	switch (attribute) {
	case SA_INVERSE:
		if (F_ISSET(sp, S_EX)) {
			if (clp->SO == NULL)
				return (1);
			if (on)
				(void)tputs(clp->SO, 1, cl_putchar);
			else
				(void)tputs(clp->SE, 1, cl_putchar);
		} else
			return (on ? standout() == ERR : standend() == ERR);
		break;
	default:
		abort();
	}
	return (0);
}

/*
 * cl_bell --
 *	Ring the bell or flash the screen.
 *
 * PUBLIC: int cl_bell __P((SCR *));
 */
int
cl_bell(sp)
	SCR *sp;
{
	if (F_ISSET(sp, S_EX)) {
		(void)write(STDOUT_FILENO, "\07", 1);		/* \a */
		return (0);
	}

	VI_INIT_IGNORE(sp);

#ifdef SYSV_CURSES
	if (O_ISSET(sp, O_FLASH))
		flash();
	else
		beep();
#else
	if (O_ISSET(sp, O_FLASH) && CLP(sp)->VB != NULL) {
		(void)tputs(CLP(sp)->VB, 1, cl_putchar);
		(void)fflush(stdout);
	} else
		(void)write(STDOUT_FILENO, "\07", 1);		/* \a */
#endif
	return (0);
}

/*
 * cl_busy --
 *	Put up, update or clear a busy message.
 *
 * PUBLIC: int cl_busy __P((SCR *, const char *, int));
 */
int
cl_busy(sp, msg, on)
	SCR *sp;
	const char *msg;
	int on;
{
	static const char flagc[] = "|/-|-\\";
	CL_PRIVATE *clp;
	struct timeval tv;
	size_t len, lno, notused;
	const char *p;

	clp = CLP(sp);

	/* Check for ex batch mode. */
	if (F_ISSET(sp, S_EX_SILENT))
		return (0);

	/*
	 * If on is set:
	 *	If no busy message is currently displayed, put one up.
	 *	If a busy message already displayed, update it.
	 * If on is not set:
	 *	Close down any displayed busy message.  It's okay to clear
	 *	a non-existent busy message, as it makes the calling code
	 *	simpler.
	 */
	if (F_ISSET(sp, S_EX | S_EX_CANON)) {
		if (on)
			switch (clp->busy_state) {
			case BUSY_OFF:
				if (msg == NULL)
					clp->busy_state = BUSY_SILENT;
				else {
					clp->busy_state = BUSY_ON;
					p = msg_cat(sp, msg, &len);
					(void)write(STDOUT_FILENO, p, len);
				}
				break;
			case BUSY_ON:
			case BUSY_SILENT:
				break;
			default:
				abort();
			}
		else
			switch(clp->busy_state) {
			case BUSY_OFF:
			case BUSY_ON:
				(void)write(STDOUT_FILENO, "\n", 1);
				/* FALLTHROUGH */
			case BUSY_SILENT:
				clp->busy_state = BUSY_OFF;
				break;

			}
		return (0);
	}

	VI_INIT_IGNORE(sp);

	lno = RLNO(sp, INFOLINE(sp));
	if (on)
		switch (clp->busy_state) {
		case BUSY_OFF:
			getyx(stdscr, clp->busy_y, clp->busy_x);
			(void)move(lno, 0);
			getyx(stdscr, notused, clp->busy_fx);

			/* If there's no message, just rest the cursor. */
			if (msg == NULL) {
				clp->busy_state = BUSY_SILENT;
				refresh();
				break;
			}

			/* Save a copy of whatever is currently there. */
			if (cl_lline_copy(sp,
			    &clp->lline_len, &clp->lline, &clp->lline_blen))
				return (1);

			/* Display the busy message. */
			p = msg_cat(sp, msg, &len);
			(void)addnstr(p, len);
			getyx(stdscr, notused, clp->busy_fx);
			(void)clrtoeol();
			(void)move(lno, clp->busy_fx);

			/* Set up for updates. */
			clp->busy_ch = 0;
			(void)gettimeofday(&clp->busy_tv, NULL);

			/* Update the state. */
			clp->busy_state = BUSY_ON;
			refresh();
			break;
		case BUSY_ON:
			/* Update no more than every 1/4 of a second. */
			(void)gettimeofday(&tv, NULL);
			if (((tv.tv_sec - clp->busy_tv.tv_sec) * 1000000 +
			    (tv.tv_usec - clp->busy_tv.tv_usec)) < 4000)
				return (0);

			/* Display the update. */
			(void)move(lno, clp->busy_fx);
			if (clp->busy_ch == sizeof(flagc))
				clp->busy_ch = 0;
			(void)addnstr(flagc + clp->busy_ch++, 1);
			(void)move(lno, clp->busy_fx);

			refresh();
			break;
		case BUSY_SILENT:
			break;
		}
	else
		switch (clp->busy_state) {
		case BUSY_OFF:
			break;
		case BUSY_ON:
			/* Restore the contents of the line. */
			move(lno, 0);
			if (clp->lline_len == 0)
				clrtoeol();
			else {
				if (F_ISSET(clp, CL_LLINE_IV))
					(void)standout();
				(void)addnstr(clp->lline, clp->lline_len);
				if (F_ISSET(clp, CL_LLINE_IV))
					(void)standend();
				clp->lline_len = 0;
			}
			/* FALLTHROUGH */
		case BUSY_SILENT:
			clp->busy_state = BUSY_OFF;
			(void)move(clp->busy_y, clp->busy_x);
			(void)refresh();
		}
	return (0);
}

/*
 * cl_canon --
 *	Enter/leave tty canonical mode.
 *
 * XXX
 * This need not be supported by any screen model not supporting full ex
 * canonical mode.
 *
 * PUBLIC: int cl_canon __P((SCR *, int));
 */
int
cl_canon(sp, enter)
	SCR *sp;
	int enter;
{
	EX_NOOP(sp);
	VI_INIT_IGNORE(sp);

	if (enter) {
		/*
		 * Move to the bottom of the screen, but don't clear the
		 * line, it may have valid contents, e.g. :set|file|append.
		 */
		(void)move(O_VAL(sp, O_LINES) - 1, 0);
		(void)refresh();
		return (cl_ex_tinit(sp));
	} else
		return (cl_ex_tend(sp));
}

/*
 * cl_clear --
 *	Clear the screen.
 *
 * PUBLIC: int cl_clear __P((SCR *));
 */
int
cl_clear(sp)
	SCR *sp;
{
	EX_NOOP(sp);
	VI_INIT_IGNORE(sp);

	return (clear() == ERR);
}

/*
 * cl_clrtoeol --
 *	Clear from the current cursor to the end of the line.
 *
 * PUBLIC: int cl_clrtoeol __P((SCR *));
 */
int
cl_clrtoeol(sp)
	SCR *sp;
{
	EX_NOOP(sp);
	VI_INIT_IGNORE(sp);

	return (clrtoeol() == ERR);
}

/*
 * cl_cursor --
 *	Return the current cursor position.
 *
 * PUBLIC: int cl_cursor __P((SCR *, size_t *, size_t *));
 */
int
cl_cursor(sp, yp, xp)
	SCR *sp;
	size_t *yp, *xp;
{
	size_t oldy;

	EX_ABORT(sp);
	VI_INIT_IGNORE(sp);

	/*
	 * Adjust to be relative to the current screen -- this allows vi to
	 * call relative to the current screen.
	 */
	getyx(stdscr, oldy, *xp);
	*yp = oldy - sp->woff;
	return (0);
}

/*
 * cl_deleteln --
 *	Delete the current line, scrolling all lines below it.  The bottom
 *	line becomes blank.
 *
 * PUBLIC: int cl_deleteln __P((SCR *));
 */
int
cl_deleteln(sp)
	SCR *sp;
{
	CHAR_T *p;
	CL_PRIVATE *clp;
	size_t len, llen, oldy, oldx;

	EX_ABORT(sp);
	VI_INIT_IGNORE(sp);

	clp = CLP(sp);

	/*
	 * If the bottom line was in use for a busy message:
	 *
	 *	Get a copy of the busy message.
	 *	Replace it with whatever was there previously.
	 *	Scroll the screen.
	 *	Restore the busy message.
	 */ 
	if (clp->busy_state == BUSY_ON) {
		getyx(stdscr, oldy, oldx);
		p = NULL;
		len = llen = 0;
		if (cl_lline_copy(sp, &len, &p, &llen))
			return (1);
		if (clp->lline_len == 0)
			clrtoeol();
		else {
			(void)move(RLNO(sp, INFOLINE(sp)), 0);
			(void)addnstr(clp->lline, clp->lline_len);
			clp->lline_len = 0;
		}
		if (deleteln() == ERR)
			return (1);
		if (len != 0) {
			(void)move(RLNO(sp, INFOLINE(sp)), 0);
			(void)addnstr(p, len);
		}
		(void)move(oldy, oldx);
		return (0);
	}

	/*
	 * If the bottom line was in reverse video, rewrite it in normal
	 * video.
	 */
	if (F_ISSET(clp, CL_LLINE_IV)) {
		if (cl_lline_copy(sp,
		    &clp->lline_len, &clp->lline, &clp->lline_blen))
			return (1);

		if (clp->lline_len == 0)
			clrtoeol();
		else {
			getyx(stdscr, oldy, oldx);
			(void)move(RLNO(sp, INFOLINE(sp)), 0);
			(void)addnstr(clp->lline, clp->lline_len);
			(void)move(oldy, oldx);
			clp->lline_len = 0;
		}
		F_CLR(clp, CL_LLINE_IV);
	}
	return (deleteln() == ERR);
}

/*
 * cl_discard --
 *	Discard a screen.
 *
 * PUBLIC: int cl_discard __P((SCR *, SCR **));
 */
int
cl_discard(sp, addp)
	SCR *sp, **addp;
{
	SCR *nsp;

	EX_ABORT(sp);

	/*
	 * Discard screen sp, and return the screen that got its real-estate.
	 * Try to add into a previous screen and then into a subsequent screen,
	 * as they're the closest to the current screen in curses.  If that
	 * doesn't work, there was no screen to join.
	 */
	if ((nsp = sp->q.cqe_prev) != (void *)&sp->gp->dq) {
		nsp->rows += sp->rows;
		*addp = nsp;
		return (0);
	} else if ((nsp = sp->q.cqe_next) != (void *)&sp->gp->dq) {
		nsp->woff = sp->woff;
		nsp->rows += sp->rows;
		*addp = nsp;
	} else
		*addp = NULL;
	return (0);
}

/* 
 * cl_exadjust --
 *	Adjust the screen for ex.  All special purpose, all special case.
 *
 * XXX
 * This need not be supported by any screen model not supporting full ex
 * canonical mode.
 *
 * PUBLIC: int cl_exadjust __P((SCR *, exadj_t));
 */
int
cl_exadjust(sp, action)
	SCR *sp;
	exadj_t action;
{
	CL_PRIVATE *clp;
	int cnt;

	VI_ABORT(sp);

	clp = CLP(sp);
	switch (action) {
	case EX_TERM_SCROLL:
		/* Move the cursor up one line if that's possible. */
		if (clp->UP != NULL)
			(void)tputs(clp->UP, 1, cl_putchar);
		else if (clp->CM != NULL)
			(void)tputs(tgoto(clp->CM,
			    0, O_VAL(sp, O_LINES) - 2), 1, cl_putchar);
		else
			return (0);
		/* FALLTHROUGH */
	case EX_TERM_CE:
		if (clp->CE != NULL) {
			(void)putchar('\r');
			(void)tputs(clp->CE, 1, cl_putchar);
		} else {
			/*
			 * !!!
			 * Historically, ex didn't erase the line, so, if the
			 * displayed line was only a single glyph, and <eof>
			 * was more than one glyph, the output would not fully
			 * overwrite the user's input.  To fix this, output
			 * the maxiumum character number of spaces.  Note,
			 * this won't help if the user entered extra prompt
			 * or <blank> characters before the command character.
			 * We'd have to do a lot of work to make that work, and
			 * it's not worth the effort.
			 */
			for (cnt = 0; cnt < MAX_CHARACTER_COLUMNS; ++cnt)
				(void)putchar('\b');
			for (cnt = 0; cnt < MAX_CHARACTER_COLUMNS; ++cnt)
				(void)putchar(' ');
			(void)putchar('\r');
			(void)fflush(stdout);
		}
		break;
	default:
		abort();
	}
	return (0);
}

/*
 * cl_getkey --
 *	Get a single terminal key (NOT event, terminal key).
 *
 * PUBLIC: int cl_getkey __P((SCR *, CHAR_T *));
 */
int
cl_getkey(sp, chp)
	SCR *sp;
	CHAR_T *chp;
{
	CL_PRIVATE *clp;
	int nr;

	clp = CLP(sp);
	switch (cl_read(sp, clp->ibuf, sizeof(clp->ibuf), &nr, NULL)) {
	case INP_OK:
		*chp = clp->ibuf[0];
		if (--nr) {
			memmove(clp->ibuf, clp->ibuf + 1, nr);
			clp->icnt = nr;
		}
		return (0);
	case INP_INTR:
	case INP_EOF:
	case INP_ERR:
		break;
	default:
		abort();
	}
	return (1);
}

/*
 * cl_insertln --
 *	Push down the current line, discarding the bottom line.  The current
 *	line becomes blank.
 *
 * PUBLIC: int cl_insertln __P((SCR *));
 */
int
cl_insertln(sp)
	SCR *sp;
{
	EX_ABORT(sp);
	VI_INIT_IGNORE(sp);

	return (insertln() == ERR);
}

/* 
 * cl_interrupt --
 *	Check for interrupts.
 *
 * PUBLIC: int cl_interrupt __P((SCR *));
 */
int
cl_interrupt(sp)
	SCR *sp;
{
	CL_PRIVATE *clp;

	/*
	 * XXX
	 * This is nasty.  If ex/vi asks about interrupts we can assume that
	 * the appropriate messages have been displayed and there's no need
	 * to post an interrupt event later.  Else, the screen code must post
	 * an interrupt event.
	 */
	clp = CLP(sp);
	if (F_ISSET(clp, CL_SIGINT)) {
		F_CLR(clp, CL_SIGINT);
		return (1);
	}
	return (0);
}

/*
 * cl_move --
 *	Move the cursor.
 *
 * PUBLIC: int cl_move __P((SCR *, size_t, size_t));
 */
int
cl_move(sp, lno, cno)
	SCR *sp;
	size_t lno, cno;
{
	EX_ABORT(sp);
	VI_INIT_IGNORE(sp);

	if (move(RLNO(sp, lno), cno) != ERR)
		return (0);

	msgq(sp, M_ERR, "Error: move: l(%u) c(%u) o(%u)", lno, cno, sp->woff);
	return (1);
}

/*
 * cl_refresh --
 *	Refresh the screen.
 *
 * PUBLIC: int cl_refresh __P((SCR *));
 */
int
cl_refresh(sp)
	SCR *sp;
{
	EX_NOOP(sp);
	VI_INIT_IGNORE(sp);

	return (refresh() == ERR);
}

/*
 * cl_repaint --
 *	Repaint the screen as of its last appearance.
 *
 * PUBLIC: int cl_repaint __P((SCR *));
 */
int
cl_repaint(sp)
	SCR *sp;
{
	EX_NOOP(sp);
	VI_INIT_IGNORE(sp);

	return (wrefresh(curscr) == ERR);
}

/*
 * cl_resize --
 *	Resize a screen.
 *
 * PUBLIC: int cl_resize __P((SCR *, long, long, SCR *, long, long));
 */
int
cl_resize(a, a_sz, a_off, b, b_sz, b_off)
	SCR *a, *b;
	long a_sz, a_off, b_sz, b_off;
{
	EX_ABORT(a);

	/*
	 * X_sz is the signed, change in the total size of the split screen,
	 * X_off is the signed change in the offset of the split screen in
	 * the curses screen.
	 */
	a->rows += a_sz;
	a->woff += a_off;
	b->rows += b_sz;
	b->woff += b_off;
	return (0);
}

/*
 * cl_split --
 *	Split a screen.
 *
 * PUBLIC: int cl_split __P((SCR *, SCR *, int));
 */
int
cl_split(old, new, to_up)
	SCR *old, *new;
	int to_up;
{
	size_t half;

	EX_ABORT(old);

	half = old->rows / 2;
	if (to_up) {				/* Old is bottom half. */
		new->rows = old->rows - half;	/* New. */
		new->woff = old->woff;
		old->rows = half;		/* Old. */
		old->woff += new->rows;
	} else {				/* Old is top half. */
		new->rows = old->rows - half;	/* New. */
		new->woff = old->woff + half;
		old->rows = half;		/* Old. */
	}
	return (0);
}

/*
 * cl_lline_copy --
 *	Get a copy of the current last line.  We could save a copy each time
 *	the last line is written, but it would be a lot more work and I don't
 *	expect to do this very often.
 */
static int
cl_lline_copy(sp, lenp, bufpp, blenp)
	SCR *sp;
	size_t *lenp, *blenp;
	CHAR_T **bufpp;
{
	CHAR_T *p, ch;
	size_t col, lno,oldx, oldy, spcnt;

	/* Allocate enough memory to hold the line. */
	BINC_RET(sp, *bufpp, *blenp, O_VAL(sp, O_COLUMNS));

	/*
	 * Walk through the line, retrieving each character.  Since curses
	 * has no EOL marker, keep track of strings of spaces, and copy the
	 * trailing spaces only if there's another non-space character.
	 *
	 * XXX
	 * This is a major kluge; it would be nice if there was an interface
	 * that let us change attributes on a per line basis.
	 */
	getyx(stdscr, oldy, oldx);
	lno = RLNO(sp, INFOLINE(sp));
	for (p = *bufpp, col = spcnt = 0;;) {
		(void)move(lno, col);
		ch = winch(stdscr);
		if (isblank(ch))
			++spcnt;
		else {
			for (; spcnt > 0; --spcnt)
				*p++ = ' ';
			*p++ = ch;
		}
		if (++col >= sp->cols)
			break;
	}
	(void)move(oldy, oldx);

	*lenp = p - *bufpp;
	return (0);
}

#ifdef DEBUG
/*
 * gdbrefresh --
 *	Stub routine so can flush out screen changes using gdb.
 */
int
gdbrefresh()
{
	refresh();
	return (0);		/* XXX Convince gdb to run it. */
}
#endif
