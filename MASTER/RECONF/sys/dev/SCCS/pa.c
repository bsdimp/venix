
/*	Driver for the Parallel Printer Adapter.
*		Version 86/1.2		July 7,1983
*			Edited: 3/7/84
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
****************************************************************/
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/proc.h>

#define	NPA	2			/* Number of ports */
#define IOINT	7			/* interrupt request */
#define VECTOR  (0x20+(4*IOINT))	/* vector address */

/*
 * Interrupt transfer vector.
 */
int	paintr();
char	pavec[9];

/*
 * Potential device i/o bus addresses.
 */
struct pa *paadrs[] = {
	(struct pa *)0x3bc,
	(struct pa *)0x378,
	(struct pa *)0x278,
	(struct pa *)0,
};
struct pa **paf = paadrs;
struct pa *pap[NPA];

/*
 * Device registers.
 */
struct pa {
	unsigned char data, status, ctrl;
};

#define PBUSY	0200			/* status bits */
#define ACKL	0100
#define NOPAPER	 040
#define SELECTD	 020
#define ERRORL	 010

#define STROBE	   1			/* control bits */
#define AUTOFEED   2
#define INITL	   4
#define SELECTL	 010
#define IE	 020

struct tty patty[NPA];
int patflg = 0;			/* Is watch dog timer running? */

struct pa* pafind(unit) register int unit; {
	register struct pa *p;

	if( pap[unit] == 0 )
	  while( p = *paf ){
		paf++;
		io_outb( p, 0252);
		if (io_inb( p ) == 0252){
			pap[unit] = p;
			break;
		}
	  }
	return( pap[unit] );
}

paopen(dev, flag) dev_t dev; {
	register struct tty *tp;
	register struct pa* PA;
	int i;
	int pastart();

	if( ((i =  minor(dev)) >= NPA) || ((PA = pafind(i)) == 0) ){
		u.u_error = ENXIO;
		return;
	}
	if( i = ttopen(tp = &patty[i]) ){
		if( i < 0 )
			return;
		setiva( VECTOR, paintr, pavec);
		tp->t_dev = dev;
		tp->t_addr = (char *)pastart;
		tp->t_flags = RAW;
		io_outb(&PA->ctrl, SELECTL);
		for(i = 0; i < 100; i++)		/* stall */
			;
		io_outb(&PA->ctrl, SELECTL|INITL);
		io_outb( 0x21,io_inb(0x21) & ~(1<<IOINT) );	/* IE system */
	}
}

paclose(dev){
	register struct tty *tp;

	tp = &patty[minor(dev)];
	if( patflg > 2 )
		flushtty(tp);
	else
		wflushtty(tp);
	tp->t_state &= ~(ISOPEN|XCLUDE);
}

pawrite(dev){
	ttwrite( &patty[minor(dev)] );
}

paintr(){
	register int i;

	io_outb( 0x20, 0x20);
	for( i = 0; i < NPA; i++ )
		if( pap[i] )
			pastart(&patty[i]);
}

patimer(tp) register struct tty *tp; {

	if( (io_inb(&(pap[minor(tp->t_dev)]->status))&ERRORL) == 0 ){
		if( patflg == 3 )	/* check recursive timeout bug */
			return;
		if( patflg++ == 2 ){
			printf("Printer needs attention!\n");
			patflg++;
		}
		timeout( patimer, tp, 3*60);
		return;
	}
	patflg = 0;
	pastart(tp);
}

pastart(tp) register struct tty *tp; {
	register struct pa *PA;
	int i;

	PA = pap[minor(tp->t_dev)];
	while( tp->t_outq.c_cc ){
		i = 0;
		while( (io_inb(&PA->status)&PBUSY) == 0 ){
			if( i++ > 40 ){	   /* Up from 10 for fast processors */
				if( patflg == 0 ){
					patflg++;
					timeout( patimer, tp, 1);
				}
				return;
			}
		}
		if( patflg ){
			timecancel( patimer, tp);
			patflg = 0;
		}
		io_outb(&PA->data, getc(&tp->t_outq));
		io_outb(&PA->ctrl, STROBE|SELECTL|INITL|IE);
		io_outb(&PA->ctrl, SELECTL|INITL|IE);
		if( tp->t_outq.c_cc < TTLOWAT && (tp->t_state&ASLEEP) ){
			tp->t_state &= ~ASLEEP;
			wakeup(&tp->t_outq);
		}
	}
}

paioctl(dev, com, addr) char *addr; {

	ttioctl( com, &patty[minor(dev)], addr, dev);
}
