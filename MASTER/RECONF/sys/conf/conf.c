
/*
 *	Version 86/2.0		May 27, 1983
 *		Edited:	6/13/84
 *
 * (C)Copyright by VenturCom Inc. 1982,1983,1984
 * All rights reserved: VENTURCOM INC. 1982,1983,1984
 *
 * Configuration file to:
 *	1) define table entries to device drivers,
 *	2) define root, swap, and pipe devices,
 *	3) allocate space for several adjustable tables,
 *	4) and set several user modifiable parameters.
 */
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/text.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/systm.h>
#include <sys/tty.h>

	extern int ttyopen(), ttyread(), ttywrite(), ttyioctl();
	extern int mmread(), mmwrite(), nulldev(), nodev();
	extern int cslopen(), cslclose(), cslread(), cslwrite(), cslioctl();
#ifdef	FLP
	extern int flpread(), flpwrite(), flpstrategy(), flptab;
#endif
#ifdef	ACA
	extern int caopen(), caclose(), caread(), cawrite(), caioctl();
#endif
#ifdef	PA
	extern int paopen(), paclose(),  pawrite(), paioctl();
#endif
#ifdef	WIN
	extern int xtread(), xtwrite(), xtstrategy(), xttab, xtioctl();
#endif	WIN
#ifdef	IOPORT
	extern int ioopen(), ioclose(), ioread(), iowrite(), ioioctl();
#endif
#ifdef	NPIPE
	extern int npopen(), npclose(), npread(), npwrite(), npioctl();
#endif
#ifdef	MOUSE
	extern int msopen(), msclose(), msread(), mswrite(), msioctl();
#endif
#ifdef	GRAPH
	extern int gropen(), grclose(), grread(), grwrite(), grioctl();
#endif

/*
 * Block (buffered; mountable) type devices.
 *	OPEN		CLOSE		STRATEGY	TABLE
 */
struct	bdevsw	bdevsw[] = {
#ifdef	FLP
	nulldev,	nulldev,	flpstrategy,	&flptab,/* 0 floppy */
#else
	nodev,		nodev,		nodev,		0,	/* 0 floppy */
#endif
#ifdef	WIN
	nulldev,	nulldev,	xtstrategy,	&xttab,	/* 1 winnie */
#endif	WIN
	0
};

/*
 * Character (terminals and `raw'(unbuffered)) type devices.
 *	OPEN	   CLOSE      READ	 WRITE	    IOCTL
 */
struct	cdevsw	cdevsw[] = {
	ttyopen,   nulldev,   ttyread,   ttywrite,  ttyioctl,	/* 0 tty */
	nulldev,   nulldev,   mmread,    mmwrite,   nodev,	/* 1 mem */
	cslopen,   cslclose,  cslread,   cslwrite,  cslioctl,	/* 2 console */
#ifdef	ACA
	caopen,    caclose,   caread,    cawrite,   caioctl,	/* 3 async */
#else
	nodev,	   nodev,     nodev,	 nodev,     nodev,	/* 3 */
#endif
#ifdef	FLP
	nulldev,   nulldev,   flpread,   flpwrite,  nodev,	/* 4 floppy */
#else
	nodev,	   nodev,     nodev,     nodev,     nodev,	/* 4 */
#endif
#ifdef	WIN
	nulldev,   nulldev,   xtread,    xtwrite,   xtioctl,	/* 5 winnie */
#else	WIN
        nodev,	   nodev,     nodev,	 nodev,     nodev,	/* 5 */
#endif	WIN
#ifdef	PA
	paopen,    paclose,   nodev,     pawrite,   paioctl,	/* 6 printer */
#else
	nodev,	   nodev,     nodev,	 nodev,     nodev,	/* 6 */
#endif
#ifdef	IOPORT
	ioopen,    ioclose,   ioread,    iowrite,   ioioctl,	/* 7 ioport */
#else
	nodev,	   nodev,     nodev,	 nodev,     nodev,	/* 7 */
#endif
#ifdef	NPIPE
	npopen,    npclose,   npread,    npwrite,   npioctl,	/* 8 npipe */
#else
	nodev,	   nodev,     nodev,	 nodev,     nodev,	/* 8 */
#endif
#ifdef	MOUSE
	msopen,    msclose,   msread,    mswrite,   msioctl,	/* 9 mouse */
#else
	nodev,	   nodev,     nodev,	 nodev,     nodev,	/* 9 */
#endif
#ifdef	GRAPH
	gropen,    grclose,   grread,    grwrite,   grioctl,	/* 10 graphic */
#endif
	0
};

/*
 * Map major device numbers to names.
 */
struct devname {
	int	maj;
	char	*msg;
} devname[] = {
	0,	"Floppy",
	1,	"Winchester",
	4,	"Floppy (unbuffered)",
	5,	"Winchester (unbuffered)",
	-1,	"Unknown device",
};

/*
 * If nswap is 0, then the kernel determins swplo from the
 * root file system size (this means that rootdev & swapdev
 * must be the same), and nswap by reading and testing for
 * errors in 50 block increments.
 */
int		swplo		= -1;		/* doesn't really matter */
int		nswap		= 0;

#ifdef	S_FLP
int		pipedev		= makedev(0,0);
int		rootdev		= makedev(0,0);
int		swapdev		= makedev(0,0);
#endif
#ifdef	S_WIN
int		pipedev		= makedev(1,1);
int		rootdev		= makedev(1,0);
int		swapdev		= makedev(1,0);
#endif

int		coremap[CMAPSIZ];
int		swapmap[SMAPSIZ];
struct callo	callout[NCALL];
struct mount	mount[NMOUNT];
struct buf	buf[NBUF];
char		buffers[NBUF][BSIZE+BSLOP];
struct buf	rawbuf[NRBUF];
struct text	text[NSEG];
struct cblock	cfree[NCLIST];
struct inode	inode[NINODE];
struct proc	proc[NPROC];
struct file	file[NFILE];
#ifndef	NO_RLOCK
struct locklist locklist[NFLOCKS];
#endif

#ifdef	PARAM
int		nproc		= NPROC;
int		maxuprc 	= MAXUPRC;
int		timezone	= TIMEZONE;
int		dstflag		= DSTFLAG;
int		hz		= HZ;
struct callo	*lcallout	= &callout[NCALL-1];
struct mount 	*lmount		= &mount[NMOUNT];
struct buf	*lbuf		= &buf[NBUF];
struct buf	*lrawbuf	= &rawbuf[NRBUF];
struct text	*ltext		= &text[NSEG];
struct cblock	*lcfree		= &cfree[NCLIST-1];
struct inode	*linode		= &inode[NINODE];
struct proc	*lproc		= &proc[NPROC];
struct proc	*lprocm		= &proc[NPROC-1];
struct file	*lfile		= &file[NFILE];
#ifndef	NO_RLOCK
struct locklist *llocklist	= &locklist[NFLOCKS];
#endif
#endif
