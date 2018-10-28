/*	Console & multiple screen terminal driver.
*		Version 86/2.0		Mar 29, 1983
*			Edited:	6/1/84
*
*****************************************************************
*								*
*	(C)Copyright by VenturCom, Inc. 1982, 1983, 1984	*
*								*
*	All rights reserved: VENTURCOM INC. 1982,1983,1984	*
*								*
*	This source listing is supplied in accordance with	*
*	the Software Agreement you have with VenturCom and	*
*	the Western Electric Company.				*
*								*
*****************************************************************
*
*    1.	The BIOS routine calls are used via the "disp?, getchar"
*	subroutines located in low.s.
*
*    2.	The DEC VT52 terminal is emulated (with extra character
*	attributes and features).
*
*    3.	This driver is capable of driving up to 4 screens on a
*	IBM/PC with graphics capabilities.  ALT 1,2,3,4 shifts
*	the display between the 4 screens.
*
* Define DEBUG and a dec. 7 char (alt 7 on num keypad) prints out
# system status information.
*/
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/proc.h>
#ifdef	DEBUG
#include <sys/reg.h>
#endif

struct	tty console[4];

struct	scrn {
	char		esc;	/* escape sequence processing */
	char		atr;	/* character atributes */
	unsigned char	x,y;	/* current cursor location */
} screen[4];

#define	NROW	25
#define	NCOL	80

#define	UNDL	0x01	/* underline */
#define	NORM	0x07	/* normal */
#define	REV	0x70	/* reverse video */
#define	HIGH	0x08	/* high light */
#define	BLINK	0x80	/* blink */


int curpage = 0;	/* current active page */
int altenbl = 0;	/* set if alternate screens are enabled */
int inbios  = 0;	/* Already printing (BIOS enables interrupts) */

cslopen(dev){
	register struct tty *tp;
	register int i;
	int j;
	extern	cslstart();

	if( (i = minor(dev)) > 3 ){
		u.u_error = ENXIO;
		return;
	}
	if( (j = ttopen( tp = &console[i])) != 0 ){
		if( j < 0 )
			return;
		tp->t_dev = dev;
		tp->t_addr = (char *)cslstart;
		tp->t_flags &= ~XTABS;
		screen[i].atr = NORM;
		screen[i].esc = 0;
		if( i != 0 )
			altenbl = 1;
	}
}

cslclose(dev){
	register struct tty *tp;

	tp = &console[minor(dev)];
	wflushtty(tp);
	tp->t_state = 0;
}

cslread(dev){

	ttread(&console[minor(dev)]);
}

cslwrite(dev){

	ttwrite(&console[minor(dev)]);
}

cslstart(atp) struct tty *atp; {
	register struct tty *tp;
	register struct scrn *sp;
	int i, c;

	tp = atp;
	sp = &screen[ i = minor(tp->t_dev) ];
	if( (tp->t_state&XOFF) == 0 && inbios == 0 ){
		inbios++;
		while( (c = getc(&tp->t_outq)) >= 0 )
			cslout(c, sp, i);
		inbios--;
		if( tp->t_state&ASLEEP ){
			tp->t_state &= ~ASLEEP;
			wakeup(&tp->t_outq);
		}
	}
}

putchar(c) char c; {		/* for kernel printf routine */
	register int tmp;

	if( inbios ){
		putc(c, &console[0].t_outq);
		return;
	}
	if( (tmp = screen[0].atr) == 0 ){	/* kernel intro */
		tmp = NORM;
		disp2(0,0,0);
		disp6( 0, NORM, 0, ((NROW-1)<<8)|(NCOL-1) );
	}
	inbios++;
	screen[0].atr = NORM|HIGH;
	cslout(c, &screen[0], 0);
	screen[0].atr = tmp;
	inbios = 0;
}

cslout(c,sp,i)
register int c;
register struct scrn *sp;
{
	switch (sp->esc)
	{
		case 1:
			switch (c)
			{
				case 'H':			/* home cursor */
					sp->x = sp->y = 0;
					break;

				case 'A':			/* up cursor */
					if (sp->y > 0)
						sp->y--;
					break;

				case 'B':			/* down cursor */
					if (sp->y < (NROW - 1))
						sp->y++;
					break;

				case 'C':			/* right cursor */
					if (sp->x < (NCOL - 1))
						sp->x++;
					break;

				case 'D':			/* left cursor */
					if (sp->x > 0)
						sp->x--;
					break;

				case 'Y':		/* direct cursor address */
					sp->esc++;
					return;

				case 'J':		/* clear to end of page */
					if (i != curpage)
						disp5(i);
					if (sp->y < (NROW - 1))
					disp6(0,NORM,((sp->y + 1) << 8),
						((NROW - 1) << 8) | (NCOL - 1));
					if (i != curpage)
						disp5(curpage);

				case 'K':		/* clear to end of line */
					disp9(' ',i,NORM,(NCOL - sp->x));
					break;

				case 'I':		/* reverse scroll */
					if (sp->y == 0)
					{
						if (i != curpage)
							disp5(i);
						disp7(1,NORM,0,
							((NROW - 1) << 8) | (NCOL - 1));
						if (i != curpage)
							disp5(curpage);
						break;
					}
					sp->y--;
					break;

				case 'B'&037:			/* start blink */
					sp->atr |= BLINK;
					break;

				case 'A'&037:			/* end blink */
					sp->atr &= ~BLINK;
					break;

				case 'F'&037:			/* start high light */
					sp->atr |= HIGH;
					break;

				case 'E'&037:			/* end high light */
					sp->atr &= ~HIGH;
					break;

				case 'D'&037:			/* start underline */
					sp->atr &= (BLINK|HIGH);
					sp->atr |= UNDL;
					break;

				case 'C'&037:		/* end underline */
					sp->atr &= (BLINK|HIGH);
					sp->atr |= NORM;
					break;

				case 'H'&037:		/* start reverse video */
					sp->atr &= (BLINK|HIGH);
					sp->atr |= REV;
					break;

				case 'G'&037:		/* end reverse video */
					sp->atr &= (BLINK|HIGH);
					sp->atr |= NORM;
					break;

				case 'N'&037:		/* reset all to normal */
					sp->atr = NORM;
					break;

				case 'L':		/* insert line */
					if (i != curpage)
						disp5(i);
					disp7(1,NORM,(sp->y << 8),
						((NROW - 1) << 8) | (NCOL - 1));
					if (i != curpage)
						disp5(curpage);
					break;

				case 'M':		/* delete line */
					if (i != curpage)
						disp5(i);
					disp6(1,NORM,(sp->y << 8),
						((NROW - 1) << 8) | (NCOL - 1));
					if (i != curpage)
						disp5(curpage);
					break;
			}
			sp->esc = 0;
			break;

		case 2:
			c -= ' ';
			sp->y = (c < (NROW - 1)) ? c : (NROW - 1);
			sp->esc++;
			return;

		case 3:
			c -= ' ';
			sp->x = (c < (NCOL - 1)) ? c : (NCOL - 1);
			sp->esc = 0;
			break;

		case 0:
			switch (c)
			{
				case 033:
					sp->esc++;

				case 0:
					return;

				case 07:			/* bell */
					disp14( 07, 0);
					return;

				case '\t':
					if ((sp->x += 8) >= NCOL)
						sp->x = NCOL;
					else
						sp->x &= ~07;
					break;

				case '\b':
					if (sp->x > 0)
						sp->x--;
					break;

				case '\r':
					sp->x = 0;
					break;

				case '\n':
					if (sp->y >= (NROW - 1))
					{
						if (i != curpage)
							disp5(i);
						disp6(1,NORM,0,
							((NROW - 1) << 8) |
							(NCOL - 1));
						if (i != curpage)
							disp5(curpage);
						return;
					}
					sp->y++;
					break;

				default:
					if (sp->x >= NCOL)
					{
						if (sp->y >= (NROW - 1))
						{
							if (i != curpage)
								disp5(i);
							disp6(1,NORM,0,
							((NROW - 1) << 8) |
								(NCOL - 1));
							if (i != curpage)
								disp5(curpage);
						}
						else
							sp->y++;
						sp->x = 0;
						disp2(i,sp->y,0);
					}
					disp9(c,i,sp->atr,1);
					sp->x++;
			}
			break;
	}
	if (sp->x >= NCOL)
		disp2(i,sp->y,(NCOL - 1));
	else
		disp2(i,sp->y,sp->x);
}

cslintr()
{
register unsigned int c;
register struct tty *tp;

	tp = &console[curpage];
	while ((c = getchar()) != -1)
	{
		/*
		 * Check for special key codes.
		 */
		if ((c & 0377) == 0)
		{
			/*
			 * Check for a <BREAK>.
			 */
			if ((c >>= 8) == 0)
			{
				signal(tp, SIGINT);
				continue;
			}
			/*
			 * Check for a change in the displayed page.
			 */
			if ((altenbl != 0) && (c >= 120) && (c <= 123))
			{
				disp5(curpage = c - 120);
				continue;
			}
			/*
			 * Check for a <NULL>.
			 */
			if (c == 3)
				c = 0;
			else
				ttyinput(033, tp);
		}
#ifdef	DEBUG
		if (c == ('G' & 037))
		{
			struct	proc *pp;

			pp = (struct proc *)u.u_procp;
printf("\n\nPrint System Information (Debugging aid)\n");
printf("\nProc table variables:\n");
printf("\tPID   = %u\tPPID = %u\tUID   = %u\tPRI  = %u\n",
		pp->p_pid, pp->p_ppid, pp->p_uid, pp->p_pri);
printf("\tTIME  = %u\tCPU  = %u\tFLAG  = 0x%x\tSTAT = 0x%x\n",
		pp->p_time, pp->p_cpu, pp->p_flag, pp->p_stat);
printf("\tNICE  = 0x%x\tWAIT = 0x%x\tSIG   = 0x%x\n",
		pp->p_nice, pp->p_wchan, pp->p_sig);
printf("\tDSIZE = 0x%x\tTOD  = 0x%x\tSSIZE = 0x%x\n",
		pp->p_dsize, pp->p_tod, pp->p_ssize);

printf("\nUser area variables:\n");
printf("\tCS    = 0x%x\tDS    = 0x%x\tES     = 0x%x\n",
		u.u_cs, u.u_ds, u.u_es);
printf("\tCSO   = 0x%x\tDSO   = 0x%x\tESO    = 0x%x\n",
		u.u_cso, u.u_dso, u.u_eso);
printf("\tERR   = %u\tFP    = %u\tSEG    = %u\tMASK   = 0%o\n",
		u.u_error, u.u_fpsaved, u.u_segflg, u.u_mask);
printf("\tEUID  = %u\tEGID  = %u\tRUID   = %u\tRGID   = %u\n",
		u.u_uid, u.u_gid, u.u_ruid, u.u_rgid);
printf("\tUTIME = %u\tSTIME = %u\tCUTIME = %u\tCSTIME = %u\n",
		u.u_utime, u.u_stime, u.u_cutime, u.u_cstime);
printf("\tPATH  = %s\n", u.u_dbuf);
printf("\tCDIR  = %s\n", u.u_dent.u_name);

printf("\nUser registers:\n");
printf("\tAX = 0x%x\tBX = 0x%x\tCX = 0x%x\tDX = 0x%x\n",
		u.u_areg[AX], u.u_areg[BX], u.u_areg[CX], u.u_areg[DX]);
printf("\tBP = 0x%x\tSI = 0x%x\tDI = 0x%x\tSP = 0x%x\n",
		u.u_areg[BP], u.u_areg[SI], u.u_areg[DI], u.u_areg[SP]);
printf("\tPC = 0x%x\tCS = 0x%x\tDS = 0x%x\tES = 0x%x\n",
		u.u_areg[PC], u.u_areg[CS], u.u_areg[DS], u.u_areg[ES]);
printf("\tPS = 0x%x\tMODE = 0x%x\n", u.u_areg[RPS], u.u_areg[MODE]);
		}
		else
#endif
			ttyinput(c, tp);
	}
}

cslioctl(dev, com, addr)
char	*addr;
{

	ttioctl(com, &console[dev&03], addr, dev);
}
