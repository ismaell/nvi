/*-
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	$Id: exf.h,v 8.39 1994/10/14 18:14:54 bostic Exp $ (Berkeley) $Date: 1994/10/14 18:14:54 $
 */
					/* Undo direction. */
/*
 * exf --
 *	The file structure.
 */
struct _exf {
	int	 refcnt;		/* Reference count. */

					/* Underlying database state. */
	DB	*db;			/* File db structure. */
	char	*c_lp;			/* Cached line. */
	size_t	 c_len;			/* Cached line length. */
	recno_t	 c_lno;			/* Cached line number. */
	recno_t	 c_nlines;		/* Cached lines in the file. */

	DB	*log;			/* Log db structure. */
	char	*l_lp;			/* Log buffer. */
	size_t	 l_len;			/* Log buffer length. */
	recno_t	 l_high;		/* Log last + 1 record number. */
	recno_t	 l_cur;			/* Log current record number. */
	MARK	 l_cursor;		/* Log cursor position. */
	enum direction lundo;		/* Last undo direction. */

	LIST_HEAD(_markh, _lmark) marks;/* Linked list of file MARK's. */

	/*
	 * XXX
	 * Mtime should be a struct timespec, but time_t is more portable.
	 */
	dev_t	 mdev;			/* Device. */
	ino_t	 minode;		/* Inode. */
	time_t	 mtime;			/* Last modification time. */

	int	 fcntl_fd;		/* Fcntl locking fd; see exf.c. */

	/*
	 * Recovery in general, and these fields specifically, are described
	 * in recover.c.
	 */
#define	RCV_PERIOD	120		/* Sync every two minutes. */
	char	*rcv_path;		/* Recover file name. */
	char	*rcv_mpath;		/* Recover mail file name. */
	int	 rcv_fd;		/* Locked mail file descriptor. */
	struct timeval rcv_tod;		/* ITIMER_REAL: recovery time-of-day. */

#define	F_FIRSTMODIFY	0x001		/* File not yet modified. */
#define	F_MODIFIED	0x002		/* File is currently dirty. */
#define	F_MULTILOCK	0x004		/* Multiple processes running, lock. */
#define	F_NOLOG		0x008		/* Logging turned off. */
#define	F_RCV_NORM	0x010		/* Don't delete recovery files. */
#define	F_RCV_ON	0x020		/* Recovery is possible. */
#define	F_UNDO		0x040		/* No change since last undo. */
	u_int8_t flags;
};

#define	GETLINE_ERR(sp, lno) {						\
	msgq(sp, M_ERR,							\
	    "025|Error: %s/%d: unable to retrieve line %u",		\
	    tail(__FILE__), __LINE__, lno);				\
}

/* EXF routines. */
FREF	*file_add __P((SCR *, CHAR_T *));
int	 file_end __P((SCR *, EXF *, int));
int	 file_m1 __P((SCR *, EXF *, int, int));
int	 file_m2 __P((SCR *, EXF *, int));
int	 file_m3 __P((SCR *, EXF *, int));

enum lockt { LOCK_FAILED, LOCK_SUCCESS, LOCK_UNAVAIL };
enum lockt
	 file_lock __P((char *, int *, int, int));

#define	FS_ALL		0x001	/* Write the entire file. */
#define	FS_APPEND	0x002	/* Append to the file. */
#define	FS_FORCE	0x004	/* Force is set. */
#define	FS_OPENERR	0x008	/* Open failed, try it again. */
#define	FS_POSSIBLE	0x010	/* Force could have been set. */
#define	FS_SETALT	0x020	/* Set alternate file name. */
int	 file_init __P((SCR *, FREF *, char *, int));
int	 file_write __P((SCR *, EXF *, MARK *, MARK *, char *, int));

/* Recovery routines. */
int	 rcv_init __P((SCR *, EXF *));
int	 rcv_list __P((SCR *));
int	 rcv_on __P((SCR *, EXF *));
int	 rcv_read __P((SCR *, FREF *));

#define	RCV_EMAIL	0x01	/* Send the user email, IFF file modified. */
#define	RCV_ENDSESSION	0x02	/* End the file session. */
#define	RCV_PRESERVE	0x04	/* Preserve backup file, IFF file modified. */
#define	RCV_SNAPSHOT	0x08	/* Snapshot the recovery, and send email. */
int	 rcv_sync __P((SCR *, EXF *, u_int));
int	 rcv_tmp __P((SCR *, EXF *, char *));

/* DB interface routines */
int	 file_aline __P((SCR *, EXF *, int, recno_t, char *, size_t));
int	 file_dline __P((SCR *, EXF *, recno_t));
char	*file_gline __P((SCR *, EXF *, recno_t, size_t *));
int	 file_iline __P((SCR *, EXF *, recno_t, char *, size_t));
int	 file_lline __P((SCR *, EXF *, recno_t *));
char	*file_rline __P((SCR *, EXF *, recno_t, size_t *));
int	 file_sline __P((SCR *, EXF *, recno_t, char *, size_t));
