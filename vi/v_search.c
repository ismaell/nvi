/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "$Id: v_search.c,v 8.12 1993/10/28 12:31:13 bostic Exp $ (Berkeley) $Date: 1993/10/28 12:31:13 $";
#endif /* not lint */

#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "vi.h"
#include "vcmd.h"

static int bcorrect __P((SCR *, EXF *, VICMDARG *, MARK *, MARK *, u_int));
static int fcorrect __P((SCR *, EXF *, VICMDARG *, MARK *, MARK *, u_int));
static int getptrn __P((SCR *, EXF *, int, char **));

/*
 * v_searchn -- n
 *	Repeat last search.
 */
int
v_searchn(sp, ep, vp, fm, tm, rp)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *tm, *rp;
{
	int flags;

	flags = SEARCH_MSG;
	if (F_ISSET(vp, VC_C | VC_D | VC_Y))
		flags |= SEARCH_EOL;
	switch (sp->searchdir) {
	case BACKWARD:
		if (b_search(sp, ep, fm, rp, NULL, NULL, &flags))
			return (1);
		if (F_ISSET(vp, VC_C | VC_D | VC_Y | VC_SH) &&
		    bcorrect(sp, ep, vp, fm, rp, flags))
			return (1);
		break;
	case FORWARD:
		if (f_search(sp, ep, fm, rp, NULL, NULL, &flags))
			return (1);
		if (F_ISSET(vp, VC_C | VC_D | VC_Y| VC_SH) &&
		    fcorrect(sp, ep, vp, fm, rp, flags))
			return (1);
		break;
	case NOTSET:
		msgq(sp, M_ERR, "No previous search pattern.");
		return (1);
	default:
		abort();
	}
	return (0);
}

/*
 * v_searchN -- N
 *	Reverse last search.
 */
int
v_searchN(sp, ep, vp, fm, tm, rp)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *tm, *rp;
{
	int flags;

	flags = SEARCH_MSG;
	if (F_ISSET(vp, VC_C | VC_D | VC_Y))
		flags |= SEARCH_EOL;
	switch (sp->searchdir) {
	case BACKWARD:
		if (f_search(sp, ep, fm, rp, NULL, NULL, &flags))
			return (1);
		if (F_ISSET(vp, VC_C | VC_D | VC_Y | VC_SH) &&
		    fcorrect(sp, ep, vp, fm, rp, flags))
			return (1);
		break;
	case FORWARD:
		if (b_search(sp, ep, fm, rp, NULL, NULL, &flags))
			return (1);
		if (F_ISSET(vp, VC_C | VC_D | VC_Y | VC_SH) &&
		    bcorrect(sp, ep, vp, fm, rp, flags))
			return (1);
		break;
	case NOTSET:
		msgq(sp, M_ERR, "No previous search pattern.");
		return (1);
	default:
		abort();
	}
	return (0);
}

/*
 * v_searchw -- [count]^A
 *	Search for the word under the cursor.
 */
int
v_searchw(sp, ep, vp, fm, tm, rp)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *tm, *rp;
{
	size_t blen, len;
	u_int flags;
	int rval;
	char *bp;

	len = vp->kbuflen + sizeof(RE_WSTART) + sizeof(RE_WSTOP);
	GET_SPACE(sp, bp, blen, len);
	(void)snprintf(bp, blen, "%s%s%s", RE_WSTART, vp->keyword, RE_WSTOP);
		
	flags = SEARCH_MSG;
	rval = f_search(sp, ep, fm, rp, bp, NULL, &flags);

	FREE_SPACE(sp, bp, blen);
	if (rval)
		return (1);
	if (F_ISSET(vp, VC_C | VC_D | VC_Y | VC_SH) &&
	    fcorrect(sp, ep, vp, fm, rp, flags))
		return (1);
	return (0);
}

/*
 * v_searchb -- [count]?RE[? offset]
 *	Search backward.
 */
int
v_searchb(sp, ep, vp, fm, tm, rp)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *tm, *rp;
{
	int flags;
	char *ptrn;

	if (F_ISSET(vp, VC_ISDOT))
		ptrn = NULL;
	else {
		if (getptrn(sp, ep, '?', &ptrn))
			return (1);
		if (ptrn == NULL)
			return (0);
	}

	flags = SEARCH_MSG | SEARCH_PARSE | SEARCH_SET | SEARCH_TERM;
	if (F_ISSET(vp, VC_C | VC_D | VC_Y))
		flags |= SEARCH_EOL;
	if (b_search(sp, ep, fm, rp, ptrn, NULL, &flags))
		return (1);
	if (F_ISSET(vp, VC_C | VC_D | VC_Y | VC_SH) &&
	    bcorrect(sp, ep, vp, fm, rp, flags))
		return (1);
	return (0);
}

/*
 * v_searchf -- [count]/RE[/ offset]
 *	Search forward.
 */
int
v_searchf(sp, ep, vp, fm, tm, rp)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *tm, *rp;
{
	int flags;
	char *ptrn;

	if (F_ISSET(vp, VC_ISDOT))
		ptrn = NULL;
	else {
		if (getptrn(sp, ep, '/', &ptrn))
			return (1);
		if (ptrn == NULL)
			return (0);
	}

	flags = SEARCH_MSG | SEARCH_PARSE | SEARCH_SET | SEARCH_TERM;
	if (F_ISSET(vp, VC_C | VC_D | VC_Y))
		flags |= SEARCH_EOL;
	if (f_search(sp, ep, fm, rp, ptrn, NULL, &flags))
		return (1);
	if (F_ISSET(vp, VC_C | VC_D | VC_Y | VC_SH) &&
	    fcorrect(sp, ep, vp, fm, rp, flags))
		return (1);
	return (0);
}

/*
 * getptrn --
 *	Get the search pattern.
 */
static int
getptrn(sp, ep, prompt, storep)
	SCR *sp;
	EXF *ep;
	int prompt;
	char **storep;
{
	TEXT *tp;

	if (sp->s_get(sp, ep, &sp->bhdr, prompt,
	    TXT_BS | TXT_CR | TXT_ESCAPE | TXT_PROMPT) != INP_OK)
		return (1);

	/* Len is 0 if backspaced over the prompt, 1 if only CR entered. */
	tp = sp->bhdr.next;
	if (tp->len == 0)
		*storep = NULL;
	else
		*storep = tp->lb;
	return (0);
}

/*
 * bcorrect --
 *	Handle command with a backward search as the motion.
 *
 * !!!
 * Historically, commands didn't affect the line searched to if the pattern
 * match was the start or end of the line.  It did, however, become a line
 * mode operation, even if it ended up affecting only a single line, if the
 * cursor started at the beginning of the line or any delta was specified
 * to the search pattern.
 */
static int
bcorrect(sp, ep, vp, fm, rp, flags)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *rp;
	u_int flags;
{
	size_t len;
	char *p;

	if (LF_ISSET(SEARCH_DELTA)) {
		F_SET(vp, VC_LMODE);
		return (0);
	}

	if ((p = file_gline(sp, ep, rp->lno + 1, &len)) == NULL) {
		GETLINE_ERR(sp, rp->lno);
		return (1);
	}
	if (len == 0 || rp->cno >= len) {
		if (fm->cno == 0)
			F_SET(vp, VC_LMODE);
		++rp->lno;
		rp->cno = 0;
	}
	return (0);
}

/*
 * fcorrect --
 *	Handle command with a forward search as the motion.
 */
static int
fcorrect(sp, ep, vp, fm, rp, flags)
	SCR *sp;
	EXF *ep;
	VICMDARG *vp;
	MARK *fm, *rp;
	u_int flags;
{
	size_t len;
	char *p;

	if (LF_ISSET(SEARCH_DELTA)) {
		F_SET(vp, VC_LMODE);
		return (0);
	}

	if (rp->cno != 0)
		return (0);

	if ((p = file_gline(sp, ep, --rp->lno, &len)) == NULL) {
		GETLINE_ERR(sp, rp->lno);
		return (1);
	}
	if (fm->cno == 0)
		F_SET(vp, VC_LMODE);
	rp->cno = len;
	return (0);
}
