#include <signal.h>
#undef NSIG                     /* to prevent redefine errors in <param.h> */
#include <sgtty.h>
#include <a.out.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/filsys.h>
#include <sys/devparm.h>
#include <sys/xtblk0.h>

#define	maximum(a,b)	(((a) > (b)) ? (a) : (b))
#define	minimum(a,b)	(((a) < (b)) ? (a) : (b))

#define DEL             ('H' & 037)     /* delete key */
#define NUSR_BOOT       508L    /* loc. of #users variable in floppy boot */
#define SN_BOOT         510L    /* loc. of serial no. in floppy boot */
#define NUSR_INIT       4L      /* offset to #users variable in /etc/init */
#define SN_INIT         (508L + 512L)
#define	TMPMAX		35

#define	DOS12FAT	2	/* dos version to use 12 bit fat */
#define	DOS16FAT	3	/* dos version to use 16 bit fat */

#define pf printf

#ifdef	MAIN_LINE

/*
	The defualt sizes are for 10MB drive.
	Do not change these numbers, since they constitute
	the smallest VENIX system.  By answering the installation
	prompts these sizes can by dynamically increased.
*/
int     ssize   = 2980;         /* default sys area size */
int     swapsiz = 750;          /* default swap size */
int     tblsize = 10;           /* default internal table area size */
int     tsize   = 340;          /* default tmp area size */
int     usize   = 16660;        /* default user area size */
int     dsize   = 0;            /* default dos area size */
int     swapint = 50;           /* default swap interval size */
int     maxfer  = 16*1024;      /* max single transfer size, don't change */

/*
	The following variables are defined here only for documentation.
	They are recalculated at run time.
	However, don't change sysmin and tmpmin since they represent
	the smallest possible file system sizes.
*/
int     sysmin  = 55;           /* min system size in cylinders, don't change */
int     sysmax  = 321;          /* max system size in cylinders */
int     tmpmin  = 5;            /* min temp   size in cylinders, don't change */
int     tmpmax  = TMPMAX;       /* max temp   size in cylinders */
int     usrmin  = 0;            /* min usr    size in cylinders */
int     usrmax  = 245;          /* max usr    size in cylinders */

/*
	Miscellaneous variables.
*/
int     dsysmin  = 0;		/* hold minimum # of cylinders for sys area */
int     dtmpmin  = 0;		/* hold minimum # of cylinders for tmp area */
int     dusrmin  = 0;		/* hold minimum # of cylinders for usr area */
int	dssize   = 0;		/* hold original values for calculations    */
int	dswapsiz = 0;		/* hold original values for calculations    */
int	hssize   = 0;		/* hold ssize for build of drive 0  */
int	hswapsiz = 0;		/* hold swapsiz for build of drive 0  */
int	htsize   = 0;		/* hold tsize for build of drive 0  */
int	husize   = 0;		/* hold usize for build of drive 0  */
int	hdsize   = 0;		/* hold dsize for build of drive 0  */
int     allsame  = 0;           /* are all drives exactly the same  */
int     similar  = 0;           /* are all drives almost  the same  */
int     sysyes   = 0;           /* is the SYS area defined          */
int     xt_ndrv  = 0;           /* used to patch the kernel         */
int     xt_ndef  = -1;          /* which only drive to initialize   */
int	sysflpy  = 0;		/* count of system floppies to load */
int	usrflpy  = 0;		/* count of user   floppies to load */
int	nusr     = 1;		/* number of allowed users */
int	serno    = 7;		/* serial number of this distribution */
int     totcyl   = 0, drv = 0, tmpoff = 0;
int	dosversion = DOS16FAT;	/* dos version to use 16 bin fat    */
int	doscutoff = 20000;	/* minimum blocks to use 16 bin fat */

char	*cntflpy    = "1234567890";
char    devname[16] = "/dev/wX.phy";	/* build area for drive names       */
char    rdevname[16] = "/dev/rwX.phy";	/* build area for drive names       */
char    *getstr();			/* get a string from stdin          */
char    clear[] = "\033H\033J\033\016"; /* clear and reset screen */
char    hlon[]  = "\033\006";           /* highlight */
char    hloff[] = "\033\005";
char    rvon[]  = "\033\010";           /* reverse video */
char    rvoff[] = "\033\007";
char	buf[512];

long	lseek();

struct	xp xp;                  /* partition definitions */
struct	xh *xh;                 /* block 0 header structure */
struct  exec header;
struct	sgttyb sgbuf;
struct  nlist *pnl;

struct  nlist symbols[] =       /* symbols looked for to patch in /venix */
	{
		"_timezon",     0,0,
		"_dstflag",     0,0,
		"_proc",        0,0,
		"_rootdev",     0,0,
		"_swapdev",     0,0,
		"_pipedev",     0,0,
		"_xt_ndrv",     0,0,
		"",             0,0
	};

struct  tz                      /* timezone info */
	{
		char    *msg;
		int     min;
	}
	tz[] =
	{
		"EST",  5*60,
		"PST",  8*60,
		"MST",  7*60,
		"CST",  6*60,
		"AST",  4*60,
		"JST",  -9*60,
		"GMT",  0,
		0,      0
	};

#else	MAIN_LINE

extern	int	ssize;		/* default sys area size		*/
extern	int	swapsiz;	/* default swap size			*/
extern	int	tblsize;	/* default internal table area size	*/
extern	int	tsize;		/* default tmp area size		*/
extern	int	usize;		/* default user area size		*/
extern	int	dsize;		/* default dos area size		*/
extern	int	swapint;	/* default swap interval size		*/
extern	int	sysmax;		/* max system size in cylinders		*/
extern	int	maxfer;		/* max single transfer size, don't change     */
extern	int	sysmin;		/* min system size in cylinders, don't change */
extern	int	tmpmin;		/* min temp   size in cylinders, don't change */
extern	int	tmpmax;		/* max temp   size in cylinders		*/
extern	int	usrmin;		/* min usr    size in cylinders		*/
extern	int	usrmax;		/* max usr    size in cylinders		*/
extern	int	dsysmin;	/* hold minimum # of cylinders for sys area */
extern	int	dtmpmin;	/* hold minimum # of cylinders for tmp area */
extern	int	dusrmin;	/* hold minimum # of cylinders for usr area */
extern	int	dssize;		/* hold original values for calculations    */
extern	int	dswapsiz;	/* hold original values for calculations    */
extern	int	hssize;		/* hold ssize for build of drive 0	*/
extern	int	hswapsiz;	/* hold swapsiz for build of drive 0	*/
extern	int	htsize;		/* hold tsize for build of drive 0	*/
extern	int	husize;		/* hold usize for build of drive 0	*/
extern	int	hdsize;		/* hold dsize for build of drive 0	*/
extern	int	allsame;	/* are all drives exactly the same	*/
extern	int	similar;	/* are all drives almost  the same	*/
extern	int	sysyes;		/* is the SYS area defined		*/
extern	int	xt_ndrv;	/* used to patch the kernel		*/
extern	int	xt_ndef;	/* which only drive to initialize	*/
extern	int	sysflpy;	/* count of system floppies to load	*/
extern	int	usrflpy;	/* count of user   floppies to load	*/
extern	int	nusr;		/* number of allowed users		*/
extern	int	serno;		/* serial number of this distribution	*/
extern	int	dosversion;	/* dos version to use 16 bin fat	*/
extern	int	doscutoff;	/* minimum blocks to use 16 bin fat	*/
extern	int	totcyl, drv, tmpoff;
extern	char	*cntflpy;
extern	char	devname[];	/* build area for drive names		*/
extern	char	rdevname[];	/* build area for drive names		*/
extern	char	*getstr();	/* get a string from stdin		*/
extern	char	clear[];	/* clear and reset screen		*/
extern	char	hlon[];		/* highlight				*/
extern	char	hloff[];
extern	char	rvon[];		/* reverse video			*/
extern	char	rvoff[];
extern	char	buf[];
extern	long	lseek();
extern	struct	xp xp;                  /* partition definitions	*/
extern	struct	xh *xh;                 /* block 0 header structure	*/
extern	struct	exec header;
extern	struct	sgttyb sgbuf;
extern	struct	nlist *pnl;
extern	struct	nlist symbols[]; /* symbols looked for to patch in /venix */
extern	struct	tz			/* timezone info */
	{
		char    *msg;
		int     min;
	} tz[];

#endif	MAIN_LINE
