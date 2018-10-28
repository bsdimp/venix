/*
	unix window ansi interpreter
*/

extern unsigned short *syspat[];
extern unsigned short *winscreen;
extern long vmo;

int	anul(), aerror(), abell(), abacks();
int	aht(),  alf(),    acr(),   aso(), asi();

/*	control character dispatch	*/

typedef	int	(*fint)();

fint	ctrldisp[] =
	{
		anul,		/* 0/0	NUL	*/
		aerror,		/* 0/1	SOH	*/
		aerror,		/* 0/2	STX	*/
		aerror,		/* 0/3	ETX	*/
		aerror,		/* 0/4	EOT	*/
		aerror,		/* 0/5	ENQ	*/
		aerror,		/* 0/6	ACK	*/
		abell,		/* 0/7	BEL	*/
		abacks,		/* 0/8	BS	*/
		aht,		/* 0/9	HT	*/
		alf,		/* 0/10	LF	*/
		alf,		/* 0/11	VT	*/
		alf,		/* 0/12	FF	*/
		acr,		/* 0/13	CR	*/
		aso,		/* 0/14 SO	*/
		asi,		/* 0/15	SI	*/
		aerror,		/* 1/0	DLE	*/
		anul,		/* 1/1	DC1-XON	*/
		aerror,		/* 1/2	DC2	*/
		anul,		/* 1/3	DC3-XOF	*/
		aerror,		/* 1/4	DC4	*/
		aerror,		/* 1/5	NAK	*/
		aerror,		/* 1/6	SYN	*/
		aerror,		/* 1/7	ETB	*/
		aerror,		/* 1/8	CAN	*/
		aerror,		/* 1/9	EM	*/
		aerror,		/* 1/10	SUB	*/
		aerror,		/* 1/11 ESC (dispatch is not through table) */
		aerror,		/* 1/12	FS	*/
		aerror,		/* 1/13	GS	*/
		aerror,		/* 1/14	RS	*/
		aerror		/* 1/15	US	*/
	};


/*
    The escdisp table is used as the dispatch for sequences
    of the form "esc char".
*/

int	anel(), ari();

fint	escdisp[] =
	{
		aerror,			/* 4/0	@	*/
		aerror,			/* 4/1	A	*/
		aerror,			/* 4/2	B	*/
		aerror,			/* 4/3	C	*/
		alf,			/* 4/4	D  IND	*/
		anel,			/* 4/5	E  NEL	*/
		aerror,			/* 4/6	F	*/
		aerror,			/* 4/7	G	*/
		aerror,			/* 4/8	H	*/
		aerror,			/* 4/9	I	*/
		aerror,			/* 4/10	J	*/
		aerror,			/* 4/11	K	*/
		aerror,			/* 4/12	L	*/
		ari,			/* 4/13	M  RI	*/
		aerror,			/* 4/14 N	*/
		aerror,			/* 4/15	O	*/
		aerror,			/* 5/0	P	*/
		aerror,			/* 5/1	Q	*/
		aerror,			/* 5/2	R	*/
		aerror,			/* 5/3	S	*/
		aerror,			/* 5/4	T	*/
		aerror,			/* 5/5	U	*/
		aerror,			/* 5/6	V	*/
		aerror,			/* 5/7	W	*/
		aerror,			/* 5/8	X	*/
		aerror,			/* 5/9	Y	*/
		aerror,			/* 5/10	Z	*/
		aerror,			/* 5/11	[	*/
		aerror,			/* 5/12	backslash	*/
		aerror,			/* 5/13	]	*/
		aerror,			/* 5/14	^	*/
		aerror,			/* 5/15	_	*/
		aerror,			/* 6/0	`	*/
		aerror,			/* 6/1	a	*/
		aerror,			/* 6/2	b	*/
		aerror,			/* 6/3	c	*/
		aerror,			/* 6/4	d	*/
		aerror,			/* 6/5	e	*/
		aerror,			/* 6/6	f	*/
		aerror,			/* 6/7	g	*/
		aerror,			/* 6/8	h	*/
		aerror,			/* 6/9	i	*/
		aerror,			/* 6/10	j	*/
		aerror,			/* 6/11	k	*/
		aerror,			/* 6/12	l	*/
		aerror,			/* 6/13	m	*/
		aerror,			/* 6/14	n	*/
		aerror,			/* 6/15	o	*/
		aerror,			/* 7/0	p	*/
		aerror,			/* 7/1	q	*/
		aerror,			/* 7/2	r	*/
		aerror,			/* 7/3	s	*/
		aerror,			/* 7/4	t	*/
		aerror,			/* 7/5	u	*/
		aerror,			/* 7/6	v	*/
		aerror,			/* 7/7	w	*/
		aerror,			/* 7/8	x	*/
		aerror,			/* 7/9	y	*/
		aerror,			/* 7/10	z	*/
		aerror,			/* 7/11	{	*/
		aerror,			/* 7/12	|	*/
		aerror,			/* 7/13	}	*/
		aerror			/* 7/14	~	*/
	};

/*
    The csidisp table is used to dispatch sequences of
    the form esc [ char.  Note that "esc [" is known as
    csi in the ansi terminology.
*/

int	aich(), acuu(), acud(), acuf(), acub(), acup(), aed(), ael();
int	ail(),  adl(),  asi(),  asu(),  asd(),  asgr(), adch();

fint	csidisp[] =
	{
		aich,			/* 4/0	@	*/
		acuu,			/* 4/1	A	*/
		acud,			/* 4/2	B	*/
		acuf,			/* 4/3	C	*/
		acub,			/* 4/4	D	*/
		aerror,			/* 4/5	E	*/
		aerror,			/* 4/6	F	*/
		aerror,			/* 4/7	G	*/
		acup,			/* 4/8	H	*/
		aerror,			/* 4/9	I	*/
		aed,			/* 4/10	J	*/
		ael,			/* 4/11	K	*/
		ail,			/* 4/12	L	*/
		adl,			/* 4/13	M	*/
		aerror,			/* 4/14	N	*/
		aerror,			/* 4/15	O	*/
		adch,			/* 5/0	P	*/
		aerror,			/* 5/1	Q	*/
		aerror,			/* 5/2	R	*/
		asu,			/* 5/3	S	*/
		asd,			/* 5/4	T	*/
		aerror,			/* 5/5	U	*/
		aerror,			/* 5/6	V	*/
		aerror,			/* 5/7	W	*/
		aerror,			/* 5/8	X	*/
		aerror,			/* 5/9	Y	*/
		aerror,			/* 5/10	Z	*/
		aerror,			/* 5/11	[	*/
		aerror,			/* 5/12	backslash	*/
		aerror,			/* 5/13	]	*/
		aerror,			/* 5/14	^	*/
		aerror,			/* 5/15	_	*/
		aerror,			/* 6/0	`	*/
		acuf,			/* 6/1	a	*/
		aerror,			/* 6/2	b	*/
		aerror,			/* 6/3	c	*/
		aerror,			/* 6/4	d	*/
		acud,			/* 6/5	e	*/
		acup,			/* 6/6	f	*/
		aerror,			/* 6/7	g	*/
		aerror,			/* 6/8	h	*/
		aerror,			/* 6/9	i	*/
		aerror,			/* 6/10	j	*/
		aerror,			/* 6/11	k	*/
		aerror,			/* 6/12	l	*/
		asgr,			/* 6/13	m	*/
		aerror,			/* 6/14	n	*/
		aerror,			/* 6/15	o	*/
		aerror,			/* 7/0	p	*/
		aerror,			/* 7/1	q	*/
		aerror,			/* 7/2	r	*/
		aerror,			/* 7/3	s	*/
		aerror,			/* 7/4	t	*/
		aerror,			/* 7/5	u	*/
		aerror,			/* 7/6	v	*/
		aerror,			/* 7/7	w	*/
		aerror,			/* 7/8	x	*/
		aerror,			/* 7/9	y	*/
		aerror,			/* 7/10	z	*/
		aerror,			/* 7/11	{	*/
		aerror,			/* 7/12	|	*/
		aerror,			/* 7/13	}	*/
		aerror			/* 7/14	~	*/
	};

/*
	ctdisp - used for seqeuences CSI = ...
*/

int	awrap(), aabsvector(), arelvector(), adrawbox();
int	aloadvpat(), asetvpat(), asetvmode(), aloadrpat(), asetrpat();
int	ainitgraph(), avertsynch(), aerasegraph(), arasterop();
int	asetrtext(), afillarea(), adrawraster();

fint	ctdisp[] =
	{
		aerror,			/* 4/0	@	*/
		adrawraster,		/* 4/1	A	*/
		avertsynch,		/* 4/2	B	*/
		aerror,			/* 4/3	C	*/
		aerror,			/* 4/4	D	*/
		aerror,			/* 4/5	E	*/
		aerror,			/* 4/6	F	*/
		aerror,			/* 4/7	G	*/
		aerasegraph,		/* 4/8	H	*/
		aerror,			/* 4/9	I	*/
		aerror,			/* 4/10	J	*/
		aerror,			/* 4/11	K	*/
		aerror,			/* 4/12	L	*/
		adrawbox,		/* 4/13	M	*/
		aerror,			/* 4/14	N	*/
		aerror,			/* 4/15	O	*/
		aerror,			/* 5/0	P	*/
		aerror,			/* 5/1	Q	*/
		aerror,			/* 5/2	R	*/
		aerror,			/* 5/3	S	*/
		aerror,			/* 5/4	T	*/
		aerror,			/* 5/5	U	*/
		aerror,			/* 5/6	V	*/
		awrap,			/* 5/7	W	*/
		aerror,			/* 5/8	X	*/
		aerror,			/* 5/9	Y	*/
		aerror,			/* 5/10	Z	*/
		aerror,			/* 5/11	[	*/
		aerror,			/* 5/12	backslash	*/
		aerror,			/* 5/13	]	*/
		aerror,			/* 5/14	^	*/
		aerror,			/* 5/15	_	*/
		aerror,			/* 6/0	`	*/
		aerror,			/* 6/1	a	*/
		aerror,			/* 6/2	b	*/
		aerror,			/* 6/3	c	*/
		aerror,			/* 6/4	d	*/
		aerror,			/* 6/5	e	*/
		aerror,			/* 6/6	f	*/
		aerror,			/* 6/7	g	*/
		aerror,			/* 6/8	h	*/
		aerror,			/* 6/9	i	*/
		aerror,			/* 6/10	j	*/
		aerror,			/* 6/11	k	*/
		aerror,			/* 6/12	l	*/
		aerror,			/* 6/13	m	*/
		aerror,			/* 6/14	n	*/
		aerror,			/* 6/15	o	*/
		aerror,			/* 7/0	p	*/
		aloadvpat,		/* 7/1	q	*/
		asetvmode,		/* 7/2	r	*/
		asetvpat,		/* 7/3	s	*/
		asetrpat,		/* 7/4	t	*/
		aerror,			/* 7/5	u	*/
		arasterop,		/* 7/6	v	*/
		ainitgraph,		/* 7/7	w	*/
		aloadrpat,		/* 7/8	x	*/
		asetrtext,		/* 7/9	y	*/
		aabsvector,		/* 7/10	z	*/
		arelvector,		/* 7/11	{	*/
		afillarea,		/* 7/12	|	*/
		aerror,			/* 7/13	}	*/
		aerror			/* 7/14	~	*/
	};

/******************************************************************************

	ANSI(wp,tbuf)	 - parse ansi x3.64 chars in tbuf

		Parse the character stream arriving in subsequent bytes
		("c") as ANSI X3.64 sequences.

*****************************************************************************/


ansi(tbuf)
register struct ccblock *tbuf;
{
register unsigned char c;
char	 *cp;
int	 cc;
int	 x;
int	 why;

	if ( (cc = tbuf->c_count) == 0 )
		return;
	if ( (cp = tbuf->c_ptr) == NULL )
		return;

/*  As long as there are characters left, stay here trying to get the
    job done.  If we're in normal state, call raster text (rastex) on
    the characters.  */

top:

	while ( cc > 0 )
	{
		if ( wp->w_astate != NORM )
			break;

/*  Since rastex expects an int ptr for DstX, we use a temporary int
    to access w_cx.  */

		x = wp->w_cx;
		why = rastex( &cp, &cc, winscreen, VIDBYTES,
			&x, wp->w_cy, wp->w_inrec.rec_lrx,
			wp->w_cff, wp->w_baseline, wp->w_vs,
			(wp->w_aflags & ALASTLF)?1:0 );
		if ( wp->w_attr && x != wp->w_cx )
			attr(wp,&x);
		wp->w_cx = x;

/*  See why rastex gave up.  If 0=all done, 1=funny char, 2=clipx.
    If all done, just continue (cc better be zero!).  If why=1, dispatch
    on the funny char.  If why == 2, check about scrolling the window, 
    else go into TRUNC state to eat up characters.  */

		if ( why == 1 )
			break;

#ifdef LATER
/*  This code should also cause a newline to occur if the last character ends
    at or beyon the right edge (lrx).  The obvious fix ( ==2 || x >= lrx )
    doesn't work -- WHY?  */
#endif

		if ( why == 2 )
		{
			if ( wp->w_aflags & ANOWRAP )
			{
				wp->w_astate = TRUNC;
				break;
			}
			else
				anel(wp);
		}
	}

/*  Here only when  astate is not NORM, or why=1 and state=NORM.  The
    first case arises when we're in the middle of a control sequence, the
    second case arsises when we've aborted the middle of a rastex becuase
    we found a funny character.  Thus, if astate = NORM, we dispatch on the
    character.  */

	if ( cc-- <= 0 )
		goto aleave;

	c = (*cp++) & 0x7F;

	switch (wp->w_astate)
	{
		case NORM:			/* c must be special */
norm:			if ( c == 033 )
			{
				wp->w_astate = ESC;
				wp->w_adisp = escdisp;
			}
			else
				aspec(wp,c);
			goto top;

		case ESC:			/* just got esc    */
			if ( c == '[' )
			{
				wp->w_astate = CSI;
				wp->w_adisp = csidisp;
				wp->w_iparam = 0;
			}
			else
				final(wp,c);
			goto top;

		case CSI:
			switch (c)
			{
				case ';':
					wp->w_aparam[0] = 0;
					wp->w_iparam = 1;
					wp->w_astate = PARAM;
					goto top;

				case '=':
					wp->w_adisp = ctdisp;
					goto top;

				default:
					if ( c>='0' && c<='9' )
					{
						wp->w_aparam[0] = c - '0';
						wp->w_astate = PARAM;
					}
					else
						final(wp,c);
					goto top;
			}

		case PARAM:
			if ( c>='0' && c<='9' )
			{
				wp->w_aparam[wp->w_iparam] *= 10;
				wp->w_aparam[wp->w_iparam] += c - '0';
				goto top;
			}
			if ( c == ';' )
			{
				if (++wp->w_iparam == NAPARAM)
					aerror(wp);
				wp->w_aparam[wp->w_iparam] = 0;
				goto top;
			}
			wp->w_iparam++;
			final(wp,c);
			goto top;

		case TRUNC:
			while ( c >= ' ' && c <= '~' )
			{
				if ( cc-- )
					goto aleave;
				c = (*cp++) & 0x7F;
			}
			wp->w_astate = NORM;
			goto norm;
	}

aleave:
	tbuf->c_count = 0;
}

/*****************************************************************************

	FINAL(wp,c)	- process sequence terminator c

		Attempt to dispatch on final character "c".  The char
		must be in the legal ansi final range (@-~).  We dispatch
		via the table pointed to by w_adisp.  Also records the
		number of parameters in nparam.

*****************************************************************************/

final(wp,c)
register struct windef *wp;
register unsigned char c;
{
	wp->w_astate = NORM;
	wp->w_nparam = wp->w_iparam;
	wp->w_iparam = 0;
	wp->w_aflags &= ~ALASTLF;	/* assume its not an lf	*/

	if ( c<'@' || c>'~' )
	{
		aerror(wp);
		return;
	}

	(wp->w_adisp)[c-'@'](wp);
}

/**************************************************************************

	ASPEC(wp,c)	- handle special characters

		The character "c" is not in the printing range, but is
		not ESC, either.  It must be a control character or DEL.

*************************************************************************/

aspec(wp,c)
register struct windef *wp;
register unsigned char c;
{
	if ( c == 0177 )
		return;			/* ignore DEL */

	wp->w_aflags &= ~ALASTLF;
	(*ctrldisp[c])(wp);
}


/******************************************************************************

	AERROR(wp)	- error catch-all routine

		Displays the error character on the display and resets the
		parser state to NORM.

******************************************************************************/

aerror(wp)
register struct windef *wp;
{
	wp->w_astate = NORM;
#ifdef ERRCHR
	aout(wp,ERRCHR);
#endif
}


/******************************************************************************

	APN(wp,p)	- Get numeric parameter number "p".
	APN0(wp,p)	- returns APN(wp,p)-1

******************************************************************************/

int
apn(wp,p)
register struct windef *wp;
int p;
{
	register int pn = wp->w_aparam[p];
	if ( pn == 0 || p >= wp->w_nparam )
		return (1);
	else
		return (pn);
}

int
apntrue(wp,p)
register struct windef *wp;
int p;
{
	register int pn = wp->w_aparam[p];
	if (p >= wp->w_nparam )
		return (-1);
	else
		return (pn);
}

int
apn0true(wp,p)
register struct windef *wp;
register int p;
{
int	t;

	if ((t = apntrue(wp,p)) < 0)
		return (aerror(wp));
	return (t - 1);
}

int
apn0(wp,p)
register struct windef *wp;
register int p;
{
	return (apn(wp,p)-1);
}

/******************************************************************************

	APS(wp)		- Get next selective parameter

******************************************************************************/

int
aps(wp)
register struct windef *wp;
{
	register short i = wp->w_iparam++;

	if ( i < wp->w_nparam )
		return (wp->w_aparam[i]);
	else
	{
		if ( i == 0 )
			return (0);
		else
			return (-1);
	}
}



/******************************************************************************

	ATTR(wp,&newx)	- spread attributes over new region

******************************************************************************/

attr(wp,newx)
register struct windef *wp;
int *newx;
{
	register short h = wp->w_vs;
	register short cy = wp->w_cy;
	register short cx = wp->w_cx;
	register short w = *newx - cx;
	short thick;
	short w2;

/*  First see if bold is turned on.  If it is, spread the text to the
    right one pixel.  If we do this, increase the width of the area
    by one pixel so the other attrs will extend, too.  */

	if ( wp->w_attr & ATTRBOLD )
	{
		if ( *newx < wp->w_inrec.rec_lrx )
			w++, *newx++, w2 = w;
		rastop(winscreen, VIDBYTES, winscreen, VIDBYTES,
			cx, cy, cx+1, cy,
			w2, h,
			SRCSRC, DSTOR, 0);
	}

	thick = wp->w_baseline/UNDLRATIO;
	if (!thick)
		thick = 1;

	if ( wp->w_attr & ATTRUNDER )
		rastop( 0,0, winscreen, VIDBYTES,
			0,0,
			cx, cy + wp->w_baseline,
			w,thick,
			SRCPAT, DSTOR, syspat[PATWHITE] );

	if ( wp->w_attr & ATTRSTRIKE )
		rastop( 0,0, winscreen, VIDBYTES,
			0,0,
			cx, cy + wp->w_baseline/STRKRATIO,
			w,thick,
			SRCPAT, DSTOR, syspat[PATWHITE] );

	if ( wp->w_attr & ATTRREV )
		rastop( 0,0, winscreen, VIDBYTES,
			0,0,
			cx, cy,
			w,h,
			SRCPAT, DSTXOR, syspat[PATWHITE] );

	if ( wp->w_attr & ATTRDIM )
		rastop( 0,0, winscreen, VIDBYTES,
			0,0,
			cx, cy,
			w,h,
			SRCPAT, DSTCAM, syspat[PATGRAY] );
}



/******************************************************************************

	Cursor Positioning:

	ACR		- cursor to col 0
	ALF		- cursor to next line, scroll if necessary
	ANEL		- combination CR + LF
	ABACKS (BS)	- cursor to previous column
	AHT		- horizontal tab to next multiple of 8 cols
	ARI		- reverse index (reverse LF)
	ACUU		- cursor up
	ACUD		- cursor down
	ACUF		- cursor forward
	ACUB		- cursor back
	ACUP		- abs cursor position

******************************************************************************/

acr(wp)
register struct windef *wp;
{
	wp->w_cx = wp->w_inrec.rec_ulx;
}


alf(wp)
register struct windef *wp;
{
	register short vs = wp->w_vs;
	register short lry = wp->w_inrec.rec_lry;

	if ( wp->w_cy + 2*vs <= lry )
		wp->w_cy += vs;
	else
	{

/*  We're making new room at the bottom of the display, light the ALASTLF
    flag so rastex will know to skip clearing out characters.  */

		ascroll(wp, wp->w_inrec.rec_uly, lry, -vs);
		wp->w_aflags |= ALASTLF;
	}
}

anel(wp)
register struct windef *wp;
{
	acr(wp);
	alf(wp);
}

abacks(wp)
register struct windef *wp;
{
	/*  The casts to short force signed comparison.  */
	if ( (short)(wp->w_cx -= wp->w_hs) < (short)wp->w_inrec.rec_ulx )
		wp->w_cx = wp->w_inrec.rec_ulx;
}

aht(wp)
register struct windef *wp;
{
	register short fwd;

	fwd = 8 - ((wp->w_cx-wp->w_inrec.rec_ulx)/wp->w_hs & 7);
	arelcurse(wp, fwd, 0);
}

ari(wp)
register struct windef *wp;
{
	register short vs = wp->w_vs;

	if ( wp->w_cy - vs >= wp->w_inrec.rec_uly )
		wp->w_cy -= vs;
	else
		ascroll(wp,wp->w_inrec.rec_uly,wp->w_inrec.rec_lry,vs);
}

acuu(wp)
register struct windef *wp;
{
	arelcurse(wp,0,-apn(wp,0));
}

acud(wp)
register struct windef *wp;
{
	arelcurse(wp,0,apn(wp,0));
}

acuf(wp)
register struct windef *wp;
{
	arelcurse(wp,apn(wp,0),0);
}

acub(wp)
register struct windef *wp;
{
	arelcurse(wp,-apn(wp,0),0);
}

acup(wp)
register struct windef *wp;
{
	aabscurse(wp,apn0(wp,1),apn0(wp,0));
}



/******************************************************************************

	Font/Attribute Control:

	ASGR		- select graphic rendition
	ASI		- shift in
	ASO		- shift out

******************************************************************************/

asgr(wp)				/* set graphic rendition */
register struct windef *wp;
{
	register short ps;
	register char attr = wp->w_attr;

	while ( (ps=aps(wp)) != -1 )
	{
		if ( ps>=10 && ps<=17 )
		{
			if ( wp->w_font[ps-10] )
				wp->w_cff = wp->w_font[ ps-10 ]->wf_ff;
			else
				aerror(wp);
			continue;
		}

		switch (ps)
		{
			case 0:		/* normal */
				attr = ATTRNORM;
				break;

			case 1:		/* bold */
				attr |= ATTRBOLD;
				break;

			case 2:		/* dim */
				attr |= ATTRDIM;
				break;

			case 4:		/* underline */
				attr |= ATTRUNDER;
				break;

			case 7:		/* reverse */
				attr |= ATTRREV;
				break;

			case 9:		/* struck-out */
				attr |= ATTRSTRIKE;
				break;

			default:
				aerror(wp);
				return;
		}
	}

	wp->w_attr = attr;
}

asi(wp)					/* shift in (select G0)	*/
register struct windef *wp;
{
	if ( wp->w_font[0] )
		wp->w_cff = wp->w_font[0]->wf_ff;
	else
		aerror(wp);
}

aso(wp)					/* shift out (select G1) */
register struct windef *wp;
{
	if ( wp->w_font[1] )
		wp->w_cff = wp->w_font[1]->wf_ff;
	else
		aerror(wp);
}



/*****************************************************************************

	AABSCURSE(wp,x,y)	- cursor to x,y

******************************************************************************/

aabscurse(wp,x,y)
register struct windef *wp;
register short x,y;
{
	register short ulx = wp->w_inrec.rec_ulx;
	register short uly = wp->w_inrec.rec_uly;
	register short hs = wp->w_hs;
	register short vs = wp->w_vs;
	register short i;

	x = x * hs + ulx;
	y = y * vs + uly;

/*  The x direction doesn't need to fit fully becuase aout will truncate.  */

	if ( x < ulx ) x = ulx;
	else if ( x >= wp->w_inrec.rec_lrx ) x = wp->w_inrec.rec_lrx-1;

	if ( y < uly ) y = uly;
	else
	{
		i = wp->w_inrec.rec_lry - vs;
		if ( y > i ) y = i;
	}

	wp->w_cx = x;
	wp->w_cy = y;
}



/******************************************************************************

	ARELCURSE(wp,dx,dy)	- move cursor relative

******************************************************************************/

arelcurse(wp,dx,dy)
register struct windef *wp;
register int dx,dy;
{
	register short i = wp->w_hs;
	register short vs = wp->w_vs;
	register int x = wp->w_cx + dx * i;
	register int y = wp->w_cy + dy * vs;

	if ( x < wp->w_inrec.rec_ulx ) x = wp->w_inrec.rec_ulx;
	else if ( x >= wp->w_inrec.rec_lrx ) x = wp->w_inrec.rec_lrx-1;

	if ( y < wp->w_inrec.rec_uly ) y = wp->w_inrec.rec_uly;
	else
	{
		i = wp->w_inrec.rec_lry - vs;
		if ( y > i ) y = i;
	}

	wp->w_cx = x;
	wp->w_cy = y;
}


#ifdef AOUT		/* dont want this anymore */
/******************************************************************************

	AOUT(wp,c)	- display char c in window wp

******************************************************************************/

aout(wp,c)
register struct windef *wp;
unsigned char c;
{
	register struct fntdef *ff = wp->w_cff;
	register short cx = wp->w_cx;
	register short lrx = wp->w_inrec.rec_lrx;
	register struct fcdef *fc;
	register short hi;
	register short hs,hs2,vs;
	char attr = wp->w_attr;

	int dstop = DSTSRC;

	if ( !ff || cx >= lrx )
		return;

	fc  = &(ff->ff_fc[c-FNTBASE]);
	hs = fc->fc_hs;
	vs = fc->fc_vs;

	if ( wp->w_uflags & VCWIDTH )
	{
		hi = fc->fc_hi;
		dstop = DSTOR;
	}
	else
		hi = wp->w_hs;

	if ( cx + hs + fc->fc_ha >= wp->w_inrec.rec_lrx )
	{
		wp->w_cx = lrx;
		hs2 = wp->w_inrec.rec_lrx - fc->fc_ha - cx;	/* clip down */
		if ( hs2 <= 0 )
			return;			/* doesn't fit at all */
	}	
	else
	{
		wp->w_cx += hi;
		hs2 = hs;
	}

	if ( fc->fc_mr )
	{
		rastop( 0,0, winscreen, VIDBYTES,
			0,0,
			cx, wp->w_cy,
			hi,wp->w_vs,
			SRCPAT, DSTSRC, 0 );

		rastop( (char *)(&(fc->fc_mr)) + fc->fc_mr, 2*((hs+15)>>4),
			winscreen, VIDBYTES,
			0,0,
			cx + fc->fc_ha, wp->w_cy + wp->w_baseline + fc->fc_va,
			hs2,vs,
			SRCSRC, dstop, NULL );

		if ( attr == 0 )
			return;

		if ( attr & ATTRREV )
			rastop( 0,0, winscreen, VIDBYTES,
				0,0,
				cx, wp->w_cy,
				hi,wp->w_vs,
				SRCPAT, DSTXOR, syspat[PATWHITE] );

		vs = wp->w_baseline/UNDLRATIO;
		if (!vs)
			vs = 1;

		if ( attr & ATTRUNDER )
			rastop( 0,0, winscreen, VIDBYTES,
				0,0,
				cx, wp->w_cy + wp->w_baseline,
				hi,vs,
				SRCPAT, DSTXOR, syspat[PATWHITE] );

		if ( attr & ATTRSTRIKE )
			rastop( 0,0, winscreen, VIDBYTES,
				0,0,
				cx, wp->w_cy + wp->w_baseline/STRKRATIO,
				hi,vs,
				SRCPAT, DSTXOR, syspat[PATWHITE] );
	}
}

#endif AOUT


/******************************************************************************

	ASCROLL(wp,top,bot,cnt)	- scroll raster lines top-bot by cnt

******************************************************************************/

ascroll(wp,top,bot,cnt)
register struct windef *wp;
register short top,bot;
register short cnt;
{
	register short ulx = wp->w_inrec.rec_ulx;
	register short width = wp->w_inrec.rec_lrx - ulx;

	if ( cnt < 0 )		/* scroll up */
	{
/*  Note that since cnt < 0, expressions read foo-cnt instead of foo+cnt
    when addition is desired.  */
		if ((winscreen == VIDMEM) && (width == VIDWIDTH) &&
			(top == 0) && (bot >= (VIDHEIGHT - wp->w_diff)))
		{
			vmo = (vmo - cnt) % VIDHEIGHT;		/* full screen scroll */
			*(VIDSHIFT) = (vmo & 0x0003ff);
		}
		else
		{
			rastop( winscreen, VIDBYTES, winscreen, VIDBYTES,
				ulx, top-cnt, ulx, top,
				width, (bot - top) + cnt,
				SRCSRC, DSTSRC, NULL );
		}
		rastop( winscreen, VIDBYTES, winscreen, VIDBYTES,
			0,0, ulx, bot + cnt - wp->w_diff,
			width, -cnt + wp->w_diff,
			SRCPAT, DSTSRC, 0 );

	}
	else if ( cnt > 0 )	/* scroll dn */
	{
		if ((winscreen == VIDMEM) && (width == VIDWIDTH) &&
			(top == 0) && (bot >= (VIDHEIGHT - wp->w_diff)))
		{
			vmo = (vmo + (VIDHEIGHT - cnt)) % VIDHEIGHT;
			*(VIDSHIFT) = (vmo & 0x0003ff);
			rastop( winscreen, VIDBYTES, winscreen, VIDBYTES,
				0,0, ulx, bot - wp->w_diff,
				width, wp->w_diff,
				SRCPAT, DSTSRC, 0 );
		}
		else
		{
			rastop( winscreen, VIDBYTES, winscreen, VIDBYTES,
				ulx, top, ulx, top+cnt,
				width, (bot - wp->w_diff - top) - cnt,
				SRCSRC, DSTSRC, NULL );
		}
		rastop( winscreen, VIDBYTES, winscreen, VIDBYTES,
			0,0, ulx, top,
			width, cnt,
			SRCPAT, DSTSRC, 0 );
	}
}



/******************************************************************************

	AINIT(wp)	- initialize ansi parser for window wp

******************************************************************************/

ainit(wp)
register struct windef *wp;
{
	wp->w_astate = NORM;
	wp->w_attr   = ATTRNORM;
	wp->w_aflags = 0;
	wp->w_cx     = wp->w_inrec.rec_ulx;
	wp->w_cy     = wp->w_inrec.rec_uly;
	wp->w_cff    = wp->w_font[0]->wf_ff;	/* font better be there! */
	ainitgraph(wp);
}

aich(wp)
register struct windef *wp;
{
	register short sx = wp->w_cx;
	register short dx = sx + apn(wp,0)*wp->w_hs;
	register short w = wp->w_inrec.rec_lrx - dx;
	register short y = wp->w_cy;
	register short h = wp->w_vs;

	if ( w <= 0 )
		return;

	rastop(winscreen,VIDBYTES,winscreen,VIDBYTES,
		sx,y, dx,y, w,h,
		SRCSRC, DSTSRC, NULL);

	rastop(winscreen,VIDBYTES,winscreen,VIDBYTES,
		0,0, sx,y, dx-sx,h,
		SRCPAT, DSTSRC, 0);
}

adch(wp)
register struct windef *wp;
{
	register short dx = wp->w_cx;
	register short w2 = apn(wp,0)*wp->w_hs;
	register short sx = dx + w2;
	register short w = wp->w_inrec.rec_lrx - sx;
	register short y = wp->w_cy;
	register short h = wp->w_vs;

	if ( w <= 0 )
		return;

	rastop(winscreen,VIDBYTES,winscreen,VIDBYTES,
		sx,y, dx,y, w,h,
		SRCSRC, DSTSRC, NULL);

	rastop(winscreen,VIDBYTES,winscreen,VIDBYTES,
		0,0, wp->w_inrec.rec_lrx-w2,y, w2,h,
		SRCPAT, DSTSRC, 0 );
}
		
ail(wp)
register struct windef *wp;
{
	ascroll(wp,wp->w_cy, wp->w_inrec.rec_lry, apn(wp,0)*wp->w_vs);
}

adl(wp)
register struct windef *wp;
{
	ascroll(wp,wp->w_cy, wp->w_inrec.rec_lry, -apn(wp,0)*wp->w_vs);
}

asu(wp)
register struct windef *wp;
{
	ascroll(wp,wp->w_inrec.rec_uly, wp->w_inrec.rec_lry, -apn(wp,0)*wp->w_vs);
}

asd(wp)
register struct windef *wp;
{
	ascroll(wp,wp->w_inrec.rec_uly, wp->w_inrec.rec_lry, apn(wp,0)*wp->w_vs);
}


aed(wp)
register struct windef *wp;
{
	register int ps;
	register short x,y,h;

	while ( (ps=aps(wp)) != -1 )
	{
		switch(ps)
		{
			case 0:			/* to EOD */
				y = wp->w_cy + wp->w_vs;
				h = wp->w_inrec.rec_lry - y;
				doel(wp,0);
				break;

			case 1:			/* from BOD */
				y = wp->w_inrec.rec_uly;
				h = wp->w_cy - y;
				doel(wp,1);
				break;

			case 2:			/* all */
				y = wp->w_inrec.rec_uly;
				h = wp->w_inrec.rec_lry - y;
				vmo = 0;
				*(VIDSHIFT) = 0;
				break;

			default:
				aerror(wp);
				return;		
		}

		x = wp->w_inrec.rec_ulx;

		rastop(winscreen,VIDBYTES,winscreen,VIDBYTES,
			0,0, x,y, (wp->w_inrec.rec_lrx - x),h,
			SRCPAT, DSTSRC, 0);

	}
}

ael(wp)
register struct windef *wp;
{
	register int ps;

	while ( (ps=aps(wp)) != -1 )
		doel(wp,ps);
}

doel(wp,ps)
register struct windef *wp;
int ps;
{
	register short x,w;

	switch (ps)
	{
		case 0:				/* to EOL */
			x = wp->w_cx;
			w = wp->w_inrec.rec_lrx - x;
			break;

		case 1:				/* from BOL */
			x = wp->w_inrec.rec_ulx;
			w = wp->w_cx - x;
			break;

		case 2:				/* both */
			x = wp->w_inrec.rec_ulx;
			w = wp->w_inrec.rec_lrx - x;
			break;

		default:
			aerror(wp);
			return;
	}

	rastop(winscreen,VIDBYTES,winscreen,VIDBYTES,
		0,0, x, wp->w_cy, w, wp->w_vs,
		SRCPAT, DSTSRC, 0);

}


awrap(wp)
register struct windef *wp;
{
	if ( aps(wp) == 0 )
		wp->w_aflags |= ANOWRAP;
	else
		wp->w_aflags &= ~ANOWRAP;
};

anul(wp)
register struct windef *wp;
{
}

abell(wp)
register struct windef *wp;
{
}

adrawbox(wp)
register struct windef *wp;
{
int	x1, y1, x2, y2;

	if (wp->w_nparam != 4)
	{
		aerror(wp);
		return;
	}
	x1 = apntrue(wp,0) + wp->w_inrec.rec_ulx;
	y1 = apntrue(wp,1) + wp->w_inrec.rec_uly;
	x2 = apntrue(wp,2) + wp->w_inrec.rec_ulx;
	y2 = apntrue(wp,3) + wp->w_inrec.rec_uly;
	acomvector(wp, x1, y1, x2, y1);		/* > - right	*/
	acomvector(wp, x2, y1, x2, y2);		/* V - down	*/
	acomvector(wp, x2, y2, x1, y2);		/* < - left	*/
	acomvector(wp, x1, y2, x1, y1);		/* ^ - up	*/
}

aabsvector(wp)
register struct windef *wp;
{
int	x1, y1, x2, y2;

	if (wp->w_nparam != 4)
	{
		aerror(wp);
		return;
	}
	x1 = apntrue(wp,0) + wp->w_inrec.rec_ulx;
	y1 = apntrue(wp,1) + wp->w_inrec.rec_uly;
	x2 = apntrue(wp,2) + wp->w_inrec.rec_ulx;
	y2 = apntrue(wp,3) + wp->w_inrec.rec_uly;
	acomvector(wp, x1, y1, x2, y2);
}

arelvector(wp)
register struct windef *wp;
{
int	x, y;

	if (wp->w_nparam != 3)
	{
		aerror(wp);
		return;
	}
	x = apntrue(wp,0) + wp->w_inrec.rec_ulx;
	y = apntrue(wp,1) + wp->w_inrec.rec_uly;
	if (apntrue(wp,2))
		acomvector(wp, wp->w_relx, wp->w_rely, x, y);
	wp->w_relx = x;
	wp->w_rely = y;
}

acomvector(wp, x1, y1, x2, y2)
register struct windef *wp;
int	x1, y1, x2, y2;
{
int	e, dx, dy, dx2, dy2, xclip, yclip, i, yintercept;

	xclip = wp->w_inrec.rec_lrx;
	yclip = wp->w_inrec.rec_lry;
	if ((dx = (x2 - x1)) < 0)	/* make sure dx is >= 0	*/
	{
		dx = -dx;
		i = x1;
		x1 = x2;
		x2 = i;
		i = y1;
		y1 = y2;
		y2 = i;
	}
	if (x1 >= xclip)		/* if start is off window	*/
	{
		aerror(wp);
		return;
	}
	if ((y1 >= yclip) && (y2 >= yclip))
	{
		aerror(wp);
		return;
	}
	dy = y2 - y1;
	if (dy == 0)
	{
		if (x2 >= xclip)
			x2 = (xclip - 1);
	}
	else
	{
		if (dx == 0)
		{
			if (y1 >= yclip)
				y1 = (yclip - 1);
			else
				if (y2 >= yclip)
					y2 = (yclip - 1);
		}
		else
		{
			if (x2 >= xclip)
			{
				yintercept = (y1 - ((x1 * dy) / dx));
				x2 = (xclip - 1);
				y2 = (((x2 * dy) / dx) + yintercept);
				if (y2 >= yclip)
					y2 = (yclip - 1);
			}
			if (y1 >= yclip)
			{
				yintercept = (y1 - ((x1 * dy) / dx));
				y1 = (yclip - 1);
				x1 = (((y1 - yintercept) * dx) / dy);
				if (x1 >= xclip)
					x1 = (xclip - 1);
			}
			else
			{
				if (y2 >= yclip)
				{
					yintercept = (y1 - ((x1 * dy) / dx));
					y2 = (yclip - 1);
					x2 = (((y2 - yintercept) * dx) / dy);
					if (x2 >= xclip)
						x2 = (xclip - 1);
				}
			}
		}
	}
	if ((dx = (x2 - x1)) < 0)	/* make sure dx is >= 0	*/
	{
		aerror(wp);
		return;
	}
	/*	At this point the vector has been clipped	*/
	dx2 = dx * 2;
	if ((dy = (y1 - y2)) < 0)	/* start in lower quadrant and	*/
	{
		dy = -dy;
		dy2 = dy * 2;
		if (dx > dy)		/* octant number 2	*/
		{
			e = dy2 - dx;
			for (i = 0; i <= dx; i++)
			{
				asetpixel(wp,x1++,y1);
				if (e < 0)
					e += dy2;
				else
				{
					y1++;
					e -= (dx2 - dy2);
				}
			}
		}
		else			/* octant number 3	*/
		{
			e = dx2 - dy;
			for (i = 0; i <= dy; i++)
			{
				asetpixel(wp,x1,y1++);
				if (e < 0)
					e += dx2;
				else
				{
					x1++;
					e -= (dy2 - dx2);
				}
			}
		}
	}
	else				/* start in upper quadrant and	*/
	{
		dy2 = dy * 2;
		if (dx > dy)		/* octant number 1	*/
		{
			e = dy2 - dx;
			for (i = 0; i <= dx; i++)
			{
				asetpixel(wp,x1++,y1);
				if (e < 0)
					e += dy2;
				else
				{
					y1--;
					e -= (dx2 - dy2);
				}
			}
		}
		else			/* octant number 0	*/
		{
			e = dx2 - dy;
			for (i = 0; i <= dy; i++)
			{
				asetpixel(wp,x1,y1--);
				if (e < 0)
					e += dx2;
				else
				{
					x1++;
					e -= (dy2 - dx2);
				}
			}
		}
	}
}

asetpixel(wp,x,y)
register struct windef *wp;
int	x, y;
{
	rastop( 0,
		0,
		VIDMEM,
		VIDBYTES,
		0,
		0,
		x,
		y,
		1,
		1,
		SRCPAT,
		wp->w_drawmode,
		syspat[PATWHITE] );
}

asetvmode(wp)
register struct windef *wp;
{
	if (wp->w_nparam != 1)
	{
		aerror(wp);
		return;
	}
	switch (apntrue(wp,0))
	{
		case 0	:
			wp->w_drawmode = DSTOR;
			break;
		case 1	:
			wp->w_drawmode = DSTCAM;
			break;
		case 2	:
			wp->w_drawmode = DSTXOR;
			break;
		case 3	:
			wp->w_drawmode = DSTSRC;
			break;
		default	:
			wp->w_drawmode = DSTSRC;
			break;
	}
}

aloadvpat(wp)
register struct windef *wp;
{
	aerror(wp);
}

asetvpat(wp)
register struct windef *wp;
{
	aerror(wp);
}

aloadrpat(wp)
register struct windef *wp;
{
	aerror(wp);
}

asetrpat(wp)
register struct windef *wp;
{
	aerror(wp);
}

ainitgraph(wp)
register struct windef *wp;
{
	wp->w_relx     = wp->w_inrec.rec_ulx;
	wp->w_rely     = wp->w_inrec.rec_uly;
	wp->w_drawmode = DSTSRC;
}

adrawraster(wp)
register struct windef *wp;
{
	aerror(wp);
}

afillarea(wp)
register struct windef *wp;
{
	aerror(wp);
}

asetrtext(wp)
register struct windef *wp;
{
	aerror(wp);
}

arasterop(wp)
register struct windef *wp;
{
struct recdef srcrec, dstrec;
int	srcx, srcy, dstx, dsty, width, height;

	if (wp->w_nparam != 8)
	{
		aerror(wp);
		return;
	}
	srcx = apntrue(wp,0);
	srcy = apntrue(wp,1);
	dstx = apntrue(wp,2);
	dsty = apntrue(wp,3);
	width = apntrue(wp,4);
	height = apntrue(wp,5);

	/* The following code will get intersection of src and dst rectangles */

	/* define source rectangle */
	srcrec.rec_ulx = srcx + wp->w_inrec.rec_ulx;
	srcrec.rec_uly = srcy + wp->w_inrec.rec_uly;
	srcrec.rec_lrx = srcx + width;
	srcrec.rec_lry = srcy + height;

	/* define destination rectangle */
	dstrec.rec_ulx = dstx + wp->w_inrec.rec_ulx;
	dstrec.rec_uly = dsty + wp->w_inrec.rec_uly;
	dstrec.rec_lrx = dstx + width;
	dstrec.rec_lry = dsty + height;

	/* clip source rectangle within window */
	if ( !recclip( &srcrec, &wp->w_inrec ) )
		return (aerror(wp));

	/* clip destination rectangle within window */
	if ( !recclip( &dstrec, &wp->w_inrec ) )
		return (aerror(wp));

	/* find the intersection of src and dst rectangles */
	width =		min((srcrec.rec_lrx - srcrec.rec_ulx),
					(dstrec.rec_lrx - dstrec.rec_ulx));
	height =	min((srcrec.rec_lrx - srcrec.rec_ulx),
					(dstrec.rec_lrx - dstrec.rec_ulx));

	rastop( VIDMEM,
			VIDBYTES,
			VIDMEM,
			VIDBYTES,
			srcx,
			srcy,
			dstx,
			dsty,
			width,
			height,
			apntrue(wp,6),	/* srcop */
			apntrue(wp,7),	/* dstop */
			0 );
}

aerasegraph(wp)
register struct windef *wp;
{
	aerror(wp);
}

avertsynch(wp)
register struct windef *wp;
{
	aerror(wp);
}
