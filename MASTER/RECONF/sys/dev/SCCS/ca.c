/*	Driver for the Asynchronous Communications Adapter.
*		Version 86/2.0		Mar 6, 1983
*			Edited:	4/18/84
*
*****************************************************************
*								*
*	(C)Copyright by VenturCom, Inc. 1982, 1983, 1984	*
*								*
*	All rights reserved. VENTURCOM INC. 1982,1983,1984	*
*								*
*	This source listing is supplied in accordance with	*
*	the Software Agreement you have with VenturCom and	*
*	the Western Electric Company.				*
*								*
*****************************************************************
*
* 4 flavors of comm line are supported:
*	normal (dumb terminal hookup)
*	modem (remote dial-in; waits for CD)
*	dial-out (priority line - causes all other comm i/o to suspend)
*	modem dial-out (priority line, but fails on open if CD already found)
*/
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/proc.h>

#define	NCA	2	/* Number of ports */
#define	MDM	0100	/* Bit in minor device # for modem control */
#define	PRI	0200	/* Bit in minor device # for dial-out line */
#define	OBIT(x)	(1<<(((unsigned)x)>>5))		/* converts three top bits */
			/* of line to unique bit */

/*
 * See mouse.c for explanation
 */
#define	USE_NO	0			/* serial port is not used	*/
#define	USE_MS	1			/* serial port used by mouse	*/
#define	USE_OT	2			/* serial port used by other	*/

extern	int use_port[NCA];

/*
 * Information table and interrupt xfer vectors.
 */
int	caint0(), caint1();
char	cavec0[9], cavec1[9];

struct
{
	unsigned	vector;
	struct ca	*base;
	int 		(*intr)();
	char		*cavec;
	unsigned char	mask;
} cainfo[NCA] = {
		0x30, (struct ca *)0x3f8, caint0, cavec0, ~(01<<4),
		0x2c, (struct ca *)0x2f8, caint1, cavec1, ~(01<<3),
		};

/*
 * Device i/o bus addresses and definitions.
 */
struct ca
{
	unsigned char rbuf, dlm, iir, lcr, mcr, lsr, msr;
};

#define WLS0		1		/* LCR bits */
#define WLS1		2
#define STB		4
#define PEN		010
#define EPS		020
#define STICKP		040
#define SBRK		0100
#define DLAB		0200

#define L7BIT		(WLS1)
#define L8BIT		(WLS0|WLS1)
#define TWOSB		STB

#define DR		1		/* Line status register */
#define OR 		2	
#define PERR		4
#define FERR		010
#define BI		020
#define THRE		040
#define TSRE		0100

#define	DTR		1		/* Modem control register */
#define RTS		2
#define OUT1		4
#define OUT2		010
#define	MIE		010
#define	LOOP		020

#define DCTS		1		/* Modem status register */
#define DDSR		2
#define TERI		4
#define DRLSD		010
#define CTS		020
#define DSR		040
#define RI		0100
#define RLSD		0200

struct	tty catty[NCA];
char	capri[NCA], caostat[NCA];

/*
 * The following table translates ioctl speeds into values
 * for the divisor latch in the INS8250 baud rate generator.
 */
int camap[16] = {
	0,	2304,	1536,	1047,
	857,	768,	384/3,	384,
	192,	96,	64, 	48,
	24,	12,	0,	0
};

caopen(dev, flag)
dev_t dev;
{
	register struct tty *tp;
	register int i;
	int j;
	int castart();

	if( (i = dev&07) >= NCA ){
		u.u_error = ENXIO;
		return;
	}
	if( (j = ttopen( tp = &catty[i])) ){
		if( j < 0 )
			return;
		setiva(cainfo[i].vector, cainfo[i].intr, cainfo[i].cavec);
		tp->t_dev = dev;
		tp->t_addr = (char *)castart;
		caparm(i);
		io_outb( 0x21, io_inb( 0x21)&cainfo[i].mask );
	}
	if( (j = dev&(PRI|MDM)) == (MDM|PRI) ){
		if( tp->t_state&CARR_ON ){	/* someone else is talking */
			u.u_error = ENXIO;
			if( !caostat[i] )	/* make sure ISOPEN updated */
				tp->t_state &= ~ISOPEN;
			return;
		}
	}

	/*
	 * It is now safe for caostat to record this flavor of line open.
	 */
	caostat[i] |= OBIT(j);

	if( j == MDM ){
		spl5();
		while( (tp->t_state&CARR_ON) == 0 )
			sleep(&tp->t_outq, TTIPRI);
		spl0();
	}
	if( j&PRI )
		capri[i] = 1;
	else
		while( capri[i] )	/* wait for priority user to finish */
			sleep(&capri[i], TTIPRI);
}

caclose(dev){
	register struct tty *tp;
	register int i;
	int j, oldstate, capickup();

	i = dev&07;
	tp = &catty[i];

	/*
	 * Exclusive use is turned off here.  The assumption is made that
	 * the process closing is the process that enabled exclusive use;
	 * it is possible, however, that a previous process is closing,
	 * in which case the process that enabled exclusive use will
	 * unexpectedly lose it.  This bug should be fixed if it causes
	 * problems.
	 */
	oldstate = (tp->t_state &= ~XCLUDE);
	if( (caostat[i] &= ~OBIT(minor(dev)&(MDM|PRI))) == 0 ){
		tp->t_state &= ~(ISOPEN|HUPCLS);
		j = 0;			/* no more, turn off interrupts */
	} else
		j = MIE;		/* more flavors open */
	wflushtty(tp);

	/*
	 * A modem hang up is done if HUPCLS bit is set, regardless of whether
	 * this is a "modem" line or not.  Note that once any flavor of line
	 * sets HUPCLS, hang up is done for all subsequent closes of all
	 * flavors of line, until all flavors are closed.
	 */
	if( oldstate & HUPCLS ){
		if( caostat[i] )
			timeout( capickup, i, 60*5);
	} else
		j |= DTR|RTS;
	io_outb( &(cainfo[i].base->mcr), j);

	if( dev&PRI ){		/* wakeup anyone waiting for */
		capri[i] = 0;	/* priority user to finish */
		wakeup(&capri[i]);
	}
}

/*
 * Reassert DTR.  This is called to pickup a line again if one process closed
 * it but another process still had it open; e.g., virtual terminal program
 * finished with modem, but login process still has it open.
 */
capickup(unit) register int unit; {

	if( catty[unit].t_state&ISOPEN )
		io_outb( &(cainfo[unit].base->mcr), DTR|RTS|MIE);
}

caread(dev){
	register int i;

	i = dev&07;
	while( capri[i] && (dev&PRI)==0 )
		sleep(&capri[i],TTIPRI);
	ttread( &catty[i]);
}

cawrite(dev){
	register int i;

	i = dev&07;
	while( capri[i] && (dev&PRI)==0 )
		sleep(&capri[i],TTIPRI);
	ttwrite( &catty[i]);
}

castart(tp) register struct tty *tp; {

	if( io_inb( &(cainfo[tp->t_dev&07].base->lsr) ) & THRE )
		caxint(tp);
}

caint0() { caintr(0); }
caint1() { caintr(1); }

caintr(unit){
	register struct tty *tp = &catty[unit];
	register struct ca* CA = cainfo[unit].base;
	char z;

	while( (z = io_inb(&CA->iir)) != 1) /* while there are pending ints */
	{
		switch(z)		/*dispatch them */
		{
			case 6: case 4: carint(tp); break;
			case 2: 	caxint(tp); break;
			case 0:		camint(tp); break;

		}
	}
	io_outb( 0x20, 0x20);		/* reset interrupt controller */
}

carint(tp)
register struct tty *tp;
{
	register struct ca* CA = cainfo[tp->t_dev&07].base;
	char c, status;

	while( (status = io_inb(&CA->lsr)) & DR ) /* while there's a char */
	{
		c = io_inb(&CA->rbuf);
		if( (tp->t_state&CARR_ON) == 0 )
			continue;
		if( status&PERR )
			continue;
		if( status&FERR )
			signal(tp,SIGINT);
		else
			ttyinput(c,tp);
	}
}

caxint(tp)
register struct tty *tp;
{
	register struct ca* CA = cainfo[tp->t_dev&07].base;
	int c;

	if( (tp->t_state&XOFF)==0 && (c = getc(&tp->t_outq))>=0 ){
		io_outb(&CA->rbuf,c);
		if( (tp->t_outq.c_cc)<TTLOWAT && (tp->t_state&ASLEEP) ){
			tp->t_state &= ~ASLEEP;
			wakeup(&tp->t_outq);
		}
	}
}

camint(tp)
register struct tty *tp;
{
	register struct ca* CA = cainfo[tp->t_dev&07].base;

	if( (io_inb(&CA->msr)&RLSD) == 0 ){	/* carry detect ? */
		io_outb(&CA->dlm,010);		/* modem control IE */
		if( tp->t_state&CARR_ON ){
			tp->t_state &= ~CARR_ON;
			if( tp->t_state&ISOPEN ){
				signal(tp,SIGHUP);
				flushtty(tp);
			}
		}
	} else {
		if( (tp->t_state&(CARR_ON|ISOPEN)) == ISOPEN ){
			tp->t_state |= CARR_ON;
			io_outb(&CA->dlm,017);	/* all IE */
			wakeup(&tp->t_outq);
		}
	}
}

caioctl(dev, com, addr) register int dev; char *addr; {
	register int i;

	i = dev&07;
	if( ttioctl( com, &catty[i], addr, dev) == 0 )
		caparm(i);
}

caparm(unit){
	register struct tty *tp = &catty[unit];
	register struct ca* CA = cainfo[unit].base;
	int i, x;

	i =  camap[tp->t_speeds&017];
	if( i == 0 ){
		io_outb(&CA->mcr, MIE);
		return;
	} else
		io_outb(&CA->mcr, MIE|DTR|RTS);
	x = spl6();
	camint(tp);
	io_outb(&CA->lcr, DLAB);	/* point to divisor latches */
	io_outb(&CA->rbuf, i);
	io_outb(&CA->dlm, i>>8);	
	switch( tp->t_flags&(EVENP|ODDP|RAW) ){
	case EVENP:
		i = L7BIT|PEN|EPS;
		break;
	case ODDP:
		i = L7BIT|PEN;
		break;
	case ODDP|RAW:
		i = L8BIT|PEN;
		break;
	case EVENP|RAW:
		i = L8BIT|PEN|EPS;
		break;
	default:
		i = L8BIT;
	}
	if( (tp->t_speeds&017) == 3 )
		i |= TWOSB;
	io_outb(&CA->lcr, i);
	splx(x);		
}
