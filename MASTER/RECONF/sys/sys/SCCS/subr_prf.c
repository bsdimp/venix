/*
 * subr_prf.c: version 6.2 of 10/21/85
 */
# ifdef SCCS
static char *sccsid = "@(#)subr_prf.c	6.2 (DSC) 10/21/85";
# endif

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/kernel.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/reboot.h"
#include "../h/msgbuf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/tty.h"

#ifndef LOGICIAN
#include "../machine/pte.h"
#include "../machine/vm.h"
#endif LOGICIAN
#include "../machine/reg.h"
#ifndef LOGICIAN
#include "../machine/panic.h"
#else LOGICIAN
#include "../machine/dkrn_err.h"
#include "../machine/dkrn_ext.h"
#include "../machine/shl.h"
#include "../machine/gio.h"

extern language_plm short scr_emg_msg();

#ifdef	KSCRPRINTF
int	log_kern_printf = 0;
extern	language_plm short kscr_kemg_kmsg();
#endif	KSCRPRINTF

#endif LOGICIAN

#define	PRT_ALL		0		/* printf logs and types */
#define	PRT_LOG		1		/* printf only logs output */
#define	PRT_TYPE	2		/* printf only types to console */

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */
#ifndef LOGICIAN
char	*panicstr;		/* panic string */
#endif

int	printhow;		/* current printf action */

#ifndef ANGIE
int	wconsflag;
#else
extern	int wconsflag;		/* nonzero if console is window, else is sio */
#endif ANGIE

#ifndef LOGICIAN
struct	panic	pblk;		/* panic block */
#endif LOGICIAN

extern	int	kdebug;		/* nonzero if debugging */

#ifndef GEM
static	int	in_a_panic = 0;
#endif GEM

/*
 * Scaled down version of C Library printf.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.  Printf should not be used for chit-chat.
 *
 * One additional format: %b is supported to decode error registers.
 * Usage is:
 *	printf("reg=%b\n", regval, "<base><arg>*");
 * Where <base> is the output base expressed as a control character,
 * e.g. \10 gives octal; \20 gives hex.  Each arg is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the next characters (up to a control character, i.e.
 * a character <= 32), give the name of the register.  Thus
 *	printf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 * would produce output:
 *	reg=2<BITTWO,BITONE>
 */
/*VARARGS1*/
printf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	prf(fmt, &x1, 0, PRT_ALL);
}

#ifdef LOGICIAN
#ifdef	KSCRPRINTF
/*VARARGS1*/
kprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	log_kern_printf = 1;
	prf(fmt, &x1, 0, PRT_ALL);
	log_kern_printf = 0;
}
#endif	KSCRPRINTF
#endif LOGICIAN

/* Write a message in the log file, but not on the console.
 * Used to log recovered disk errors without messing up the console.
 */
/*VARARGS1*/
lprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	prf(fmt, &x1, 0, PRT_LOG);
}

/* Write a message to the console, but not to the log file.
 * Used by information typeout routines.
 */
/*VARARGS1*/
cprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	prf(fmt, &x1, 0, PRT_TYPE);
}

/*
 * Uprintf prints to the current user's terminal,
 * guarantees not to sleep (so can be called by interrupt routines)
 * and does no watermark checking - (so no verbose messages).
 */
/*VARARGS1*/
uprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	if (u.u_ttyp) prf(fmt, &x1, u.u_ttyp, 0);
}

/*
 * Tprintf is like uprintf, except that output can be sent to the designated
 * terminal line.
 */
/*VARARGS1*/
tprintf(tp, fmt, x1)
	struct tty *tp;		/* terminal to send to */
	char *fmt;		/* format string */
	unsigned x1;		/* argument list */
{
	if (tp) prf(fmt, &x1, tp, 0);
}

prf(fmt, adx, tp, how)
	register char *fmt;
	register u_int *adx;
	register struct tty *tp;
{
	register char c;
	register char *s;
	int oldhow,b,i,any;

	oldhow = printhow;	/* save old value in case of nested calls */
	printhow = how;
loop:	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			printhow = oldhow;
			putchar('\0', tp);	/* flush console output */
			return;
		}
		putchar(c, tp);
	}
again:
	switch (c = *fmt++) {

	case 'x': case 'X':
		b = 16;
		goto number;

	case 'd': case 'D':
	case 'u':		/* what a joke */
		b = 10;
		goto number;

	case 'o': case 'O':
		b = 8;
number:		printn((u_long)*adx, b, tp);
		break;

	case 'c':
		b = *adx;
#ifndef LOGICIAN
		for (i = 24; i >= 0; i -= 8)
#else
		for (i = 0; i > 24; i += 8)
#endif
			if (c = (b >> i) & 0x7f)
				putchar(c, tp);
		break;

	case 'b':
		b = *adx++;
		s = (char *)*adx;
		printn((u_long)b, *s++, tp);
		any = 0;
		if (b) {
			putchar('<', tp);
			while (i = *s++) {
				if (b & (1 << (i-1))) {
					if (any)
						putchar(',', tp);
					any = 1;
					for (; (c = *s) > 32; s++)
						putchar(c, tp);
				} else
					for (; *s > 32; s++)
						;
			}
			if (any)
				putchar('>', tp);
		}
		break;

	case 'r':		/* recursive printf for panic to use */
		prf((char *)*adx, adx[1], tp);
		adx++;
		break;

	case 's':
		s = (char *)*adx;
		while (c = *s++)
			putchar(c, tp);
		break;

	case '%':
		putchar('%', tp);
		break;
	}
	adx++;
	goto loop;
}

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernel stacks.
 */
printn(n, b, tp)
	u_long n;
	struct tty *tp;
{
	char prbuf[11];
	register char *cp;

	if (b == 10 && (int)n < 0) {
		putchar('-', tp);
		n = (unsigned)(-(int)n);
	}
	cp = prbuf;
	do {
		*cp++ = "0123456789abcdef"[n%b];
		n /= b;
	} while (n);
	do
		putchar(*--cp, tp);
	while (cp > prbuf);
}

/*
 * Panic is called on unresolvable fatal errors.  It syncs, prints
 * "panic: mesg" and then loops.  The "real" panic routine first saves
 * useful data into the panic structure pblk, and then comes here.
 * The panic string can be formatted like printf to include useful data.
 * If we are called twice, then we avoid trying to sync the disks as
 * this often leads to recursive panics.
 */

#ifdef LOGICIAN
panic(s,args)
	char	*s;
	char	*args;
{
	cpanic(s,&args);
}
#endif LOGICIAN

static	char	*ctime();

cpanic(s, adx)
	register char *s;
	u_int	*adx;
{
	int bootopt = panicstr ? RB_AUTOBOOT|RB_NOSYNC : RB_AUTOBOOT;

#ifndef GEM
	int	was_in_panic = in_a_panic;
	
	in_a_panic = 1;
#endif	GEM

	panicstr = s;
	
	printf("\npanic: %s : %r\n", ctime(), s, adx);

#ifndef LOGICIAN
	if (kdebug) breakpoint();	/* stop now if debugging */
#endif
	(void) spl0();
#ifndef LOGICIAN
	boot(RB_PANIC, bootopt);
#endif

#ifndef GEM
	in_a_panic = was_in_panic;
#endif	GEM
}

/* START of internal ascii time generation */

static	char	cbuf[26];
static	int	dmsize[12] =
	{
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
/*
 * The following table is used for 1974 and 1975 and
 * gives the day number of the first day after the Sunday of the
 * change.
 */
struct dstab {
	int	dayyr;
	int	daylb;
	int	dayle;
};
static struct dstab usdaytab[] = {
	1974,	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	1975,	58,	303,	/* 1975: Last Sun. in Feb - last Sun in Oct */
	0,	119,	303,	/* all other years: end Apr - end Oct */
};
static struct dstab ausdaytab[] = {
	1970,	400,	0,	/* 1970: no daylight saving at all */
	1971,	303,	0,	/* 1971: daylight saving from Oct 31 */
	1972,	303,	58,	/* 1972: Jan 1 -> Feb 27 & Oct 31 -> dec 31 */
	0,	303,	65,	/* others: -> Mar 7, Oct 31 -> */
};
/*
 * The European tables ... based on hearsay
 * Believed correct for:
 *	WE:	Great Britain, Ireland, Portugal
 *	ME:	Belgium, Luxembourg, Netherlands, Denmark, Norway,
 *		Austria, Poland, Czechoslovakia, Sweden, Switzerland,
 *		DDR, DBR, France, Spain, Hungary, Italy, Jugoslavia
 * Eastern European dst is unknown, we'll make it ME until someone speaks up.
 *	EE:	Bulgaria, Finland, Greece, Rumania, Turkey, Western Russia
 */
static struct dstab wedaytab[] = {
	1983,	86,	303,	/* 1983: end March - end Oct */
	1984,	86,	303,	/* 1984: end March - end Oct */
	1985,	86,	303,	/* 1985: end March - end Oct */
	0,	400,	0,	/* others: no daylight saving at all ??? */
};
static struct dstab medaytab[] = {
	1983,	86,	272,	/* 1983: end March - end Sep */
	1984,	86,	272,	/* 1984: end March - end Sep */
	1985,	86,	272,	/* 1985: end March - end Sep */
	0,	400,	0,	/* others: no daylight saving at all ??? */
};
static struct dayrules {
	int		dst_type;	/* number obtained from system */
	int		dst_hrs;	/* hours to add when dst on */
	struct	dstab *	dst_rules;	/* one of the above */
	enum {STH,NTH}	dst_hemi;	/* southern, northern hemisphere */
} dayrules [] = {
	DST_USA,	1,	usdaytab,	NTH,
	DST_AUST,	1,	ausdaytab,	STH,
	DST_WET,	1,	wedaytab,	NTH,
	DST_MET,	1,	medaytab,	NTH,
	DST_EET,	1,	medaytab,	NTH,	/* XXX */
	-1,
};

static	char	*ct_numb();
static	char	*ct_num();
static	char	*asctime();
static	struct	tm	*localtime();
static	struct	tm	*gmtime();

static	char *
ctime()
{
	return(asctime(localtime()));
}

static	struct tm *
localtime()
{
	register int dayno;
	register struct tm *ct;
	register dalybeg, daylend;
	register struct dayrules *dr;
	register struct dstab *ds;
	int year;
	unsigned long copyt;
	struct timeval curtime;
	struct timezone zone;
	int s;

/*
	The following 4 lines are the same as:
	gettimeofday(&curtime,&zone);
	which can't be called inside the kernel
*/
	s = spl7();
	curtime = time;
	splx(s);
	zone = tz;

	copyt = (unsigned long)curtime.tv_sec -
		(unsigned long)zone.tz_minuteswest*60;
	ct = gmtime(&copyt);
	dayno = ct->tm_yday;
	for (dr = dayrules; dr->dst_type >= 0; dr++)
		if (dr->dst_type == zone.tz_dsttime)
			break;
	if (dr->dst_type >= 0) {
		year = ct->tm_year + 1900;
		for (ds = dr->dst_rules; ds->dayyr; ds++)
			if (ds->dayyr == year)
				break;
		dalybeg = ds->daylb;	/* first Sun after dst starts */
		daylend = ds->dayle;	/* first Sun after dst ends */
		dalybeg = sunday(ct, dalybeg);
		daylend = sunday(ct, daylend);
		switch (dr->dst_hemi) {
		case NTH:
		    if (!(
		       (dayno>dalybeg || (dayno==dalybeg && ct->tm_hour>=2)) &&
		       (dayno<daylend || (dayno==daylend && ct->tm_hour<1))
		    ))
			    return(ct);
		    break;
		case STH:
		    if (!(
		       (dayno>dalybeg || (dayno==dalybeg && ct->tm_hour>=2)) ||
		       (dayno<daylend || (dayno==daylend && ct->tm_hour<2))
		    ))
			    return(ct);
		    break;
		default:
		    return(ct);
		}
	        copyt += dr->dst_hrs*60*60;
		ct = gmtime(&copyt);
		ct->tm_isdst++;
	}
	return(ct);
}

/*
 * The argument is a 0-origin day number.
 * The value is the day number of the first
 * Sunday on or after the day.
 */
static
sunday(t, d)
register struct tm *t;
register int d;
{
	if (d >= 58)
		d += dysize(t->tm_year) - 365;
	return(d - (d - t->tm_yday + t->tm_wday + 700) % 7);
}

static	struct tm *
gmtime(tim)
unsigned long *tim;
{
	register int d0, d1;
	unsigned long hms, day;
	register int *tp;
	static struct tm xtime;

	/*
	 * break initial number into days
	 */
	hms = *tim % 86400;
	day = *tim / 86400;
	if (hms<0) {
		hms += 86400;
		day -= 1;
	}
	tp = (int *)&xtime;

	/*
	 * generate hours:minutes:seconds
	 */
	*tp++ = hms%60;
	d1 = hms/60;
	*tp++ = d1%60;
	d1 /= 60;
	*tp++ = d1;

	/*
	 * day is the day number.
	 * generate day of the week.
	 * The addend is 4 mod 7 (1/1/1970 was Thursday)
	 */

	xtime.tm_wday = (day+7340036)%7;

	/*
	 * year number
	 */
	if (day>=0) for(d1=70; day >= dysize(d1); d1++)
		day -= dysize(d1);
	else for (d1=70; day<0; d1--)
		day += dysize(d1-1);
	xtime.tm_year = d1;
	xtime.tm_yday = d0 = day;

	/*
	 * generate month
	 */

	if (dysize(d1)==366)
		dmsize[1] = 29;
	for(d1=0; d0 >= dmsize[d1]; d1++)
		d0 -= dmsize[d1];
	dmsize[1] = 28;
	*tp++ = d0+1;
	*tp++ = d1;
	xtime.tm_isdst = 0;
	return(&xtime);
}

static	char *
asctime(t)
struct tm *t;
{
	register char *cp, *ncp;
	register int *tp;

	cp = cbuf;
	for (ncp = "Day Mon 00 00:00:00 1900"; *cp++ = *ncp++;);
	ncp = &"SunMonTueWedThuFriSat"[3*t->tm_wday];
	cp = cbuf;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp++;
	tp = &t->tm_mon;
	ncp = &"JanFebMarAprMayJunJulAugSepOctNovDec"[(*tp)*3];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp = ct_numb(cp, *--tp);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	if (t->tm_year>=100) {
		cp[1] = '2';
		cp[2] = '0' + t->tm_year >= 200;
	}
	cp += 2;
	cp = ct_numb(cp, t->tm_year+100);
	return(cbuf);
}

static
dysize(y)
{
	if((y%4) == 0)
		return(366);
	return(365);
}

static char *
ct_numb(cp, n)
register char *cp;
{
	cp++;
	if (n>=10)
		*cp++ = (n/10)%10 + '0';
	else
		*cp++ = ' ';
	*cp++ = n%10 + '0';
	return(cp);
}
/* END of internal ascii time generation */

/*
 * Warn that a system table is full.
 */
tablefull(tab)
	char *tab;
{

	printf("%s: table is full\n", tab);
}

/*
 * Hard error is the preface to plaintive error messages
 * about failing disk transfers.
 */
harderr(bp, cp)
	struct buf *bp;
	char *cp;
{

	printf("%s%d%c: hard error sn%d ", cp,
	    dkunit(bp), 'a'+(minor(bp->b_dev)&07), bp->b_blkno);
}

/*
 * Print a character on console or designated terminal.  If tp is null, then
 * the output might go to console, and the last MSGBUFS characters might be
 * saved in msgbuf for inspection later, as determined by printhow.
 */
/*ARGSUSED*/
putchar(c, tp)
	register char	c;
	register struct tty *tp;	/* terminal line, or NULL if console */
{
#ifndef LOGICIAN
	if (c == '\0') {
		if (tp) return;
		goto flushit;
	}

	if (tp) {
		if (tp->t_state&TS_CARR_ON) {
			register s = spl6();
			if (c == '\n')
				(void) ttyoutput('\r', tp);
			(void) ttyoutput(c, tp);
			ttstart(tp);
			splx(s);
		}
		return;
	}
	if ((printhow != PRT_TYPE) && (c != '\r') && (c != 0177)) {
		if (msgbuf.msg_magic != MSG_MAGIC) {

			msgbuf.msg_bufx = 0;
			msgbuf.msg_magic = MSG_MAGIC;
		}
		if (msgbuf.msg_bufx < 0 || msgbuf.msg_bufx >= MSG_BSIZE)
			msgbuf.msg_bufx = 0;
		msgbuf.msg_bufc[msgbuf.msg_bufx++] = c;
	}
	if (printhow == PRT_LOG)
		return;
flushit:
#ifdef ANGIE
	if (wconsflag)
	{
		if (in_a_panic)
			zioputchar(c);	/* write to normal tty */
		winputchar(c);		/* and write to 286 window */
	}
	else
		zioputchar(c);		/* write to normal tty */
#else ANGIE
	rsputchar(c);
#endif ANGIE

#else LOGICIAN
	struct msgbuf *mbp;

	if (c == '\0') return;

	if (tp) {
		register short s = spl7();

		splx(s);

	    	if (u.u_msgbuf == NULL) {
			if (s != 0) {
			    panic("putchar -- called when disabled with no msgbuf");
			};
			u.u_msgbuf = calloc((int) sizeof(msgbuf));
			if (u.u_msgbuf == NULL) {
			    panic("putchar -- calloc failed");
			};
			u.u_msgbuf->msg_magic = 0;
		};
		mbp = u.u_msgbuf;
	}
	else {
	    	mbp = &msgbuf;
	};  

	if (mbp->msg_magic != MSG_MAGIC) {
	    register int i;
	
	    mbp->msg_bufx = 0;
	    mbp->msg_magic = MSG_MAGIC;
	    for (i = 0; i < MSG_BSIZE; i++) {
		mbp->msg_bufc[i] = 0;
	    };
	};						     
	if (mbp->msg_bufx < 0 || mbp->msg_bufx > MSG_BSIZE) {
	    mbp->msg_bufx = 0;
	};

	if (c != '\n') {
	    if (mbp->msg_bufx < (MSG_BSIZE - 2)) {
		mbp->msg_bufc[mbp->msg_bufx++] = c;
	    };
	    mbp->msg_bufc[mbp->msg_bufx] = 0;
	    return;
	}
	else {				       
		/* *** temporary *** */
		/* -- need to use tty output if tp != null, as for 32016 */
	    if (mbp->msg_bufx == 0) {
		/* null */ ;
	    } else if (! in_a_panic) {
#ifdef	KSCRPRINTF
		if (log_kern_printf)
		{
			register s = spl7();

			kscr_kemg_kmsg((u_short)mbp->msg_bufx,
				&mbp->msg_bufc[0]);
			splx(s);
		}
		else
#endif	KSCRPRINTF
			scr_emg_msg((u_short)mbp->msg_bufx,
			    &mbp->msg_bufc[0]);
	    } else {
		    mbp->msg_bufc[mbp->msg_bufx++] = 0x0D; /* CR */
		    mbp->msg_bufc[mbp->msg_bufx++] = '\n';
		    dkrn_jmp_mntr((u_short) mbp->msg_bufx,
				  &mbp->msg_bufc[0]);
		    mbp->msg_bufc[mbp->msg_bufx - 2] = 0;
	    };
	};

	mbp->msg_bufx = 0;
#endif LOGICIAN

}

/* a dummy routine .. gem includes 'rs.c' with this routine */

#ifndef ANGIE
#ifndef GEM
rsputchar(c)
char c;
{
	c = c;
}
#endif GEM
#endif ANGIE

/* Type the status of the most recently running process on the terminal.
 * Format:  load pid (name) pc memory cpu state 
 */
pstat(tp)
	register struct	tty *tp;		/* terminal to output to */
{
	register struct	proc *p;		/* active process */
	struct	proc *ap;			/* auxillary proc pointer */
	register int	i;			/* random uses */
	int	group, state, load, min, sec;	/* random variables */
	int	pc, resmem, totmem;		/* process data */
	char	*str;				/* state string */

#ifndef TTY
	if (tp == 0) 
		return;
#else TTY
	if ((tp == 0) || (tp->t_outq.c_cc >= (TTHIWAT(tp) + 10))) {
		return;		/* don't overfill terminal buffer */
	}
#endif TTY

	/* search for the most recently running process in our terminal group */

	group = tp->t_pgrp;
	ap = NULL;
	i = 0;
	load = 0;
	for (p = proc; p < procNPROC; p++) {
		state = p->p_stat;
		if (state == 0) continue;
		if (state == SRUN) load++;
		if (p->p_pgrp != group) continue;
		if (p->p_infoage <= i) continue;
		i = p->p_infoage;
		ap = p;

	}
	p = ap;
	if (p == NULL) {			/* if no process found, quit */
		tprintf(tp, "load %d no current process\n", load);
		return;
	}

	/* collect data about the found process and type it */

	state = p->p_stat;		/* get state, last pc, and runtime */
	pc = p->p_infopc;
	min = p->p_infotime;
	if (u.u_procp == p) {		/* get new data if current proc */
#ifndef LOGICIAN
		pc = u.u_ar0[RPC];
#else LOGICIAN
		pc = u.u_ar0[CS_IP];
#endif LOGICIAN
		min = u.u_ru.ru_utime.tv_sec + u.u_ru.ru_stime.tv_sec;
		sec = u.u_ru.ru_utime.tv_usec + u.u_ru.ru_stime.tv_usec;
		min = (min * 10) + (sec / 100000);
	}
	sec = min % 600;		/* units are tenths of seconds */
	min /= 600;
	resmem = 0;			/* get current memory sizes */
	totmem = 0;
#ifndef LOGICIAN
	if (state != SZOMB) {
		i = p->p_spti;
		resmem = spt[i].spt_mempages;
		totmem = spt[i].spt_usedpages;
		if (resmem > totmem) totmem = resmem;
	}
#endif LOGICIAN
	switch (state) {		/* get state string */
		case SRUN:
			str = "run";
			if (u.u_procp == p) str = "running";
			break;
		case SSLEEP:
			str = "sleep";
			if (p->p_pri <= PZERO) str = "io wait";
			i = (int) p->p_wchan;
			if ((i >= (int)tp) && (i < (int)(tp+1)))
				str = "tty wait";
#ifndef LOGICIAN
			if ((i >= (int)cst) && (i < (int)cstNCST))
				str = "page wait";
#endif LOGICIAN
			if ((i >= (int)proc) && (i < (int)procNPROC))
				str = "proc wait";
			if (i == (int)&p->p_msg) str = "server wait";
			if (i == (int)&u) str = "pause";
			break;
		case SSTOP:
			str = "stopped"; break;
		case SIDL:
			str = "creating"; break;
		case SZOMB:
			str = "zombie"; break;
		default:
			str = "unknown";
	}
	tprintf(tp, "load %d pid %d (%s) pc %x mem %d/%d cpu ",
		load, p->p_pid, p->p_infoname, pc, resmem, totmem);
	if (min) tprintf(tp, "%d%s", min, (sec>99)?":":":0");
	tprintf(tp, "%d.%d %s%s\n", sec/10, sec%10,
		(p->p_flag & SLOAD) ? "":"swapped ", str);
}

#ifdef LOGICIAN
/*
 *	boot
 *
 *		This procedure jumps to the LOGICIAN monitor to request
 *	a reboot.
 */

boot(reason,option)
	int	reason;
	int	option;
{
	in_a_panic = 1;
	printf("Please reboot:  reason = %d, option = %d.\n",reason,option);
}
#endif LOGICIAN
