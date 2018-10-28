/*
 *              VENIX/XT Installation Program   Nov 17, 1983
 *                 Version 2.0    6/14/84
 *                 Version 3.0    8/28/85
 *
 *      Compile as:     cc -s -O -o init install.c
 *      and put on the "Bootable Xfer" floppy in "/f0/etc/init".
 */

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

#define pf printf

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

main()
{
	start();
	getserial();
	askshell();
	askdrives();
	doparmpart();
	dousrarea();
	dosysarea();
	putserial();
	doetcrc();
	putnusers();
	dokernel();
	finish();
}

start()
{
int	i;

	if (open("/dev/console",2) < 0)         /* open standard error */
		for (;;);                       /* loop forever */
	dup(0);                                 /* std in */
	dup(0);                                 /* std out */
	for (i = 0; i < 16; ++i)
		signal(i,SIG_IGN);              /* ignore all signals */
	ioctl(1, TIOCGETP, &sgbuf);             /* turn off echo */
	sgbuf.sg_flags = CRMOD|CRT;
	ioctl(1, TIOCSETP, &sgbuf);
	return (0);
}

/*
 * find out number of users and system serial number,
 * stored in floppy boot block
 */
getserial()
{
int	fd;

	if ((fd = open("/dev/f0",0)) < 0)
		fatal("Can't open /dev/f0\n");
	lseek(fd, NUSR_BOOT, 0);
	read(fd, &nusr, sizeof(nusr));
	lseek(fd, SN_BOOT, 0);
	read(fd, &serno, sizeof(serno));
	close(fd);
	return (0);
}

askshell()
{
	pf("%sXFER/XT%s                Version 2.1\n\n",hlon,hloff);
	if (!prompt(
	"Do you wish to prepare the hard disk for VENIX/86 operation?",0))
	{
		pf("\nThe shell will be executed.\n\n");
		signal(SIGHUP,SIG_DFL);
		signal(SIGINT,SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
		sgbuf.sg_flags = ECHO|CRT|CRMOD;
		ioctl(1,TIOCSETP,&sgbuf);
		execl("/bin/sh","sh","-i",0);
		fatal("Cannot execute the shell `/bin/sh'.\n");
	}
	return (0);
}

askdrives()
{
	allsame = similar = 0;
	xt_ndef = -1;
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart I%s          Drive Parameters and Partitions\n\n",
		hlon,hloff);
	if (!prompt("Will more than one drive be initialized?",0))
	{
		pf("\nEnter drive number to be initialized (0 to 7) [0] : ");
		xt_ndef = getnum(0,7,0);
		xt_ndrv = xt_ndef + 1;
		return (0);
	}
	pf("\nNumber of disk drives to be initialized (0 to 8) [2] : ");
	if ((xt_ndrv = getnum(0,8,2)) > 1)
	{
		pf("\nWill all %d drives have same parameters?",xt_ndrv);
		if (allsame = prompt("",0))
			return (0);
		pf("\nWill the %d drives have similar parameters?",xt_ndrv);
		similar = prompt("",0);
	}
	return (0);
}

/*
 * PART I
 */
doparmpart()
{
int	fd;

	dssize = ssize;
	dswapsiz = swapsiz;
	for (drv = ((xt_ndef < 0) ? 0 : xt_ndef); drv < xt_ndrv; drv++)
	{
		fd = opendrive(drv);
		getblk0(fd);
		if (!drv || !allsame)
		{
			initheader();
			parmheading();
			dumpheader();
			while(!prompt("\nAre these parameters acceptable?",0))
			{
				newheader();
				sleep(5);
				parmheading();
				dumpheader();
			}
			initmins();
			partheading();
			dumppart();
			while(!prompt("\nAre these partitions acceptable?",0))
			{
				dosyspart();
				dotmppart();
				dousrpart();
				dodospart();
				sleep(5);
				partheading();
				dumppart();
			}
		}
		closedrive(fd);
	/*	reread(drv);	*/
		sleep(5);
	}
	if (xt_ndrv && (xt_ndef < 0))
	{
		ssize   = hssize;
		swapsiz = hswapsiz;
		tsize   = htsize;
		usize   = husize;
		dsize   = hdsize;
	}
	return (0);
}

opendrive(drive)
int	drive;
{
int	fd;

	devname[6] = drive + '0';
	if ((fd = open(devname,2)) < 0)
	{
		perror("open block device");
		pf("ERROR: %s:\t",devname);
		fatal("Error in accessing winchester.\n");
	}
	return (fd);
}

closedrive(fd)
int	fd;
{
	write(fd, &xp, 512);
	sync();
	close(fd);
	sync();
	return (0);
}

reread(drive)
int	drive;
{
int	fd;
extern	errno;

	rdevname[7] = drive + '0';
	if ((fd = open(rdevname,2)) < 0)
	{
		perror("open raw device");
		pf("ERROR: %s:\t",rdevname);
		fatal("Error in accessing raw winchester.\n");
	}
	errno = 0;
	ioctl(fd, I_REREAD, (char *)0);
	if (errno)
		perror("ioctl reread");
	close(fd);
	sync();
	return (0);
}

getblk0(fd)
int	fd;
{
int	fd2;

	/*
		if first drive or (all drives not same and not similar)
	*/
	if (!drv || (!allsame && !similar))
	{
		if (read(fd, &xp, 512) != 512)
			fatal("Cannot read disk block zero.\n");
		if (xp.xp_sig != XP_SIG)
		{
			if ((fd2 = open("/uboot/xtblk0",0)) < 0)
			{
				pf("Cannot open partition file: ");
				fatal("/uboot/xtblk0\n");
			}
			if (read(fd2, &xp, 512) != 512)
			{
				pf("Cannot read partition file: ");
				fatal("/uboot/xtblk0\n");
			}
			close(fd2);
		}
		lseek(fd, 0L, 0);	/* rewind to block 0 */
	}
	return (0);
}

parmheading()
{
	pf("%s%s                VENIX/86 Installing Drive #%d%s\n\n",
		clear,hlon,drv,hloff);
	pf("%sPart I - A%s            Drive Parameters\n\n",hlon,hloff);
	return (0);
}

partheading()
{
	pf("%s%s                VENIX/86 Installing Drive #%d%s\n\n",
		clear,hlon,drv,hloff);
	pf("%sPart I - B%s            Drive Partitions\n\n",hlon,hloff);
	return (0);
}

initheader()
{
	/*
		The defaults are for 10MB full height
	*/
	xh = (struct xh *)&xp.xp_code[XH_START];
	xp.xp_sig = XP_SIG;
	if ((xh->xh_sig1 != XH_SIG1) || (xh->xh_sig2 != XH_SIG2))
	{
		xh->xh_sig1 = XH_SIG1;
		xh->xh_ndrv = drv;
		strcpy(xh->xh_name,"Winche");
		xh->xh_name[6] = drv + '0';
		xh->xh_name[7] = 0;
		xh->xh_npart = 4;
		xh->xh_nunit = 8;
		xh->xh_nhead = 4;
		xh->xh_ncyls = 305;
		xh->xh_sechd = 17;
		xh->xh_ntrack = xh->xh_nhead * xh->xh_ncyls;
		xh->xh_blkscyl = xh->xh_nhead * xh->xh_sechd;
		xh->xh_blksize = 512;
		xh->xh_cntf = 5;
		xh->xh_maxfer = maxfer;
		xh->xh_sig2 = XH_SIG2;
		return (0);
	}
	xh->xh_ndrv = drv;
	xh->xh_name[6] = drv + '0';
	xh->xh_name[7] = 0;
	return (0);
}

dumpheader()
{
	pf("Drive Number         : %d %s*%s\n",xh->xh_ndrv,hlon,hloff);
	pf("Drive Name           : %s\n",xh->xh_name);
	pf("Number of Partitions : %d\n",xh->xh_npart);
	pf("Number of Units      : %d\n",xh->xh_nunit);
	pf("Number of Heads      : %d\n",xh->xh_nhead);
	pf("Number of Cylinders  : %d\n",xh->xh_ncyls);
	pf("Sectors per Head     : %d\n",xh->xh_sechd);
	pf("Tracks per Drive     : %d %s*%s\n",xh->xh_ntrack,hlon,hloff);
	pf("Blocks per Cylinder  : %d %s*%s\n",xh->xh_blkscyl,hlon,hloff);
	pf("Block Size           : %d %s*%s\n",xh->xh_blksize,hlon,hloff);
	pf("Step Speed Code      : %d\n",xh->xh_cntf);
	pf("Max Single transfer  : %d\n",xh->xh_maxfer);
	pf("\n%s* these values are generated automatically%s\n",hlon,hloff);
	return (0);
}

newheader()
{
	pf("\nDrive Number                    [%d] : %s%d%s\n",
		xh->xh_ndrv,rvon,xh->xh_ndrv,rvoff);
	pf("Drive Name                      [%s] : ",
		xh->xh_name);
	strcpy(xh->xh_name,getstr(1,7,xh->xh_name));
	pf("Number of Partitions (%d to %d) [%d] : ",
		0,4,xh->xh_npart);
	xh->xh_npart = getnum(0,4,xh->xh_npart);
	pf("Number of Units      (%d to %d) [%d] : ",
		0,8,xh->xh_nunit);
	xh->xh_nunit = getnum(0,8,xh->xh_nunit);
	pf("Number of Heads      (%d to %d) [%d] : ",
		0,8,xh->xh_nhead);
	xh->xh_nhead = getnum(0,8,xh->xh_nhead);
	pf("Number of Cylinders  (%d to %d) [%d] : ",
		0,999,xh->xh_ncyls);
	xh->xh_ncyls = getnum(0,999,xh->xh_ncyls);
	pf("Sectors per Head     (%d to %d) [%d] : ",
		0,99,xh->xh_sechd);
	xh->xh_sechd = getnum(0,99,xh->xh_sechd);
	pf("Tracks per Drive                [%d] : %s%d%s\n",
		xh->xh_ntrack,rvon,
		xh->xh_nhead * xh->xh_ncyls,rvoff);
	xh->xh_ntrack = xh->xh_nhead * xh->xh_ncyls;
	pf("Blocks per Cylinder             [%d] : %s%d%s\n",
		xh->xh_blkscyl,rvon,
		xh->xh_nhead * xh->xh_sechd,rvoff);
	xh->xh_blkscyl = xh->xh_nhead * xh->xh_sechd;
	pf("Block Size                      [%d] : %s%d%s\n",
		xh->xh_blksize,rvon,xh->xh_blksize,rvoff);
	pf("Step Speed Code      (%d to %d) [%d] : ",
		0,9,xh->xh_cntf);
	xh->xh_cntf = getnum(0,9,xh->xh_cntf);
	pf("Max Single transfer  (%d to %d) [%d] : ",
		0,maxfer,xh->xh_maxfer);
	xh->xh_maxfer = getnum(0,maxfer,xh->xh_maxfer);
	return (0);
}

initmins()
{
	if (drv)
	{
		dsysmin = dtmpmin = sysmin = tmpmin = 0;
	}
	else
	{
		dsysmin = sysmin =
		maximum(((ssize + swapsiz + tblsize + 1) / xh->xh_blkscyl),
			sysmin);
		dtmpmin = tmpmin = maximum((tsize / xh->xh_blkscyl),tmpmin);
	}
	dusrmin = usrmin = 0;
	sysmax = xh->xh_ncyls - tmpmin;
	return (0);
}

dosyspart()
{
int	cyl, blks, endcyl;

	sysmin = dsysmin;
	pf("\nSizes are in cylinders (%d blocks per cylinder)\n",
		xh->xh_blkscyl);
	checkrange(&sysmin,&sysmax,&sysmin);
	pf("\nDesired size of system area (%d to %d) [%d] : ",
		sysmin,sysmax,sysmin);
	if ((cyl = getnum(sysmin,sysmax,sysmin)) <= 0)
	{
		xp.xp_tab[0].xp_boot = XP_NULL;
		xp.xp_tab[0].xp_s_h = 0;
		xp.xp_tab[0].xp_s_s = 0;
		xp.xp_tab[0].xp_s_c = 0;
		xp.xp_tab[0].xp_sys = XP_UNUSED;
		xp.xp_tab[0].xp_e_h = 0;
		xp.xp_tab[0].xp_e_s = 0;
		xp.xp_tab[0].xp_e_c = 0;
		xp.xp_tab[0].xp_start = 0;
		xp.xp_tab[0].xp_size = 0;
		sysyes = swapsiz = totcyl = ssize = 0;
		tmpmax = TMPMAX;
		tmpoff = 1;
		pf("\nThere will be no SYS area on this drive #%d\n",drv);
		return (1);
	}
	swapsiz = dswapsiz;
	blks = ((xh->xh_blkscyl * cyl) - dssize - tblsize - 1) / swapint;
	blks *= swapint;
	checkrange(&swapsiz,&blks,&swapsiz);
	pf("\nNumber of blocks for swap (intervals of %d) (%d to %d) [%d] : ",
		swapint,swapsiz,blks,swapsiz);
	swapsiz = getnum(swapsiz,blks,swapsiz);
	swapsiz = (swapsiz / swapint) * swapint;
	pf("%s\nUsing %d blocks at end of system area for swapping\n%s",
		hlon,swapsiz,hloff);
	ssize = (xh->xh_blkscyl * cyl) - tblsize - swapsiz - 1;
	endcyl = cyl - 1;
	xp.xp_tab[0].xp_boot = XP_BOOT;
	xp.xp_tab[0].xp_s_h = 0;
	xp.xp_tab[0].xp_s_s = 1;
	xp.xp_tab[0].xp_s_c = 0;
	xp.xp_tab[0].xp_sys = XP_SYS;
	xp.xp_tab[0].xp_e_h = xh->xh_nhead - 1;
	xp.xp_tab[0].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp.xp_tab[0].xp_e_c = endcyl;
	xp.xp_tab[0].xp_start = 1;
	xp.xp_tab[0].xp_size = cyl * xh->xh_blkscyl - 1;
	totcyl = cyl;
	tmpmax = minimum((xh->xh_ncyls - cyl),TMPMAX);
	if (!drv)
	{
		hssize = ssize;
		hswapsiz = swapsiz;
	}
	tmpoff = 0;
	sysyes = 1;
	return (0);
}

dotmppart()
{
int	cyl, endcyl;

	tmpmin = dtmpmin;
	if (tmpmax < 1)
		goto notmparea;
	checkrange(&tmpmin,&tmpmax,&tmpmin);
	pf("\nDesired size of temp area (%d to %d) [%d] : ",
		tmpmin,tmpmax,tmpmin);
	if ((cyl = getnum(tmpmin,tmpmax,tmpmin)) <= 0)
	{
notmparea:
		xp.xp_tab[1].xp_boot = XP_NULL;
		xp.xp_tab[1].xp_s_h = 0;
		xp.xp_tab[1].xp_s_s = 0;
		xp.xp_tab[1].xp_s_c = 0;
		xp.xp_tab[1].xp_sys = XP_UNUSED;
		xp.xp_tab[1].xp_e_h = 0;
		xp.xp_tab[1].xp_e_s = 0;
		xp.xp_tab[1].xp_e_c = 0;
		xp.xp_tab[1].xp_start = 0;
		xp.xp_tab[1].xp_size = 0;
		tsize = 0;
		pf("\nThere will be no TMP area on this drive #%d\n",drv);
		return (1);
	}
	tsize = cyl * xh->xh_blkscyl;
	endcyl = totcyl + cyl - 1;
	xp.xp_tab[1].xp_boot = XP_NULL;
	xp.xp_tab[1].xp_s_h = 0;
	xp.xp_tab[1].xp_s_s = 1 | ((totcyl >> 2) & 0300);
	xp.xp_tab[1].xp_s_c = totcyl;
	xp.xp_tab[1].xp_sys = XP_TMP;
	xp.xp_tab[1].xp_e_h = xh->xh_nhead - 1;
	xp.xp_tab[1].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp.xp_tab[1].xp_e_c = endcyl;
	xp.xp_tab[1].xp_start = (totcyl * xh->xh_blkscyl) + tmpoff;
	xp.xp_tab[1].xp_size = tsize - tmpoff;
	tsize -= tmpoff;
	totcyl += cyl;
	if (!drv)
		htsize = tsize;
	tmpoff = 0;
	return (0);
}

dousrpart()
{
int	cyl, endcyl;

	usrmin = dusrmin;
	usrmax = xh->xh_ncyls - totcyl;
	if (usrmax < 1)
		goto nousrpart;
	checkrange(&usrmin,&usrmax,&usrmin);
	pf("\nDesired size of user area (%d to %d) [%d] : ",
		usrmin,usrmax,usrmin);
	if ((cyl = getnum(usrmin,usrmax,usrmin)) <= 0)
	{
nousrpart:
		xp.xp_tab[2].xp_boot = XP_NULL;
		xp.xp_tab[2].xp_s_h = 0;
		xp.xp_tab[2].xp_s_s = 0;
		xp.xp_tab[2].xp_s_c = 0;
		xp.xp_tab[2].xp_sys = XP_UNUSED;
		xp.xp_tab[2].xp_e_h = 0;
		xp.xp_tab[2].xp_e_s = 0;
		xp.xp_tab[2].xp_e_c = 0;
		xp.xp_tab[2].xp_start = 0;
		xp.xp_tab[2].xp_size = 0;
		usize = 0;
		pf("\nThere will be no USR area on this drive #%d\n",drv);
		return (1);
	}
	usize = cyl * xh->xh_blkscyl;
	endcyl = totcyl + cyl - 1;
	xp.xp_tab[2].xp_boot = XP_NULL;
	xp.xp_tab[2].xp_s_h = 0;
	xp.xp_tab[2].xp_s_s = 1 | ((totcyl >> 2) & 0300);
	xp.xp_tab[2].xp_s_c = totcyl;
	xp.xp_tab[2].xp_sys = XP_USR;
	xp.xp_tab[2].xp_e_h = xh->xh_nhead - 1;
	xp.xp_tab[2].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp.xp_tab[2].xp_e_c = endcyl;
	xp.xp_tab[2].xp_start = (totcyl * xh->xh_blkscyl) + tmpoff;
	xp.xp_tab[2].xp_size = usize - tmpoff;
	usize -= tmpoff;
	totcyl += cyl;
	if (!drv)
		husize = usize;
	tmpoff = 0;
	return (0);
}

dodospart()
{
int	cyl, endcyl;

	cyl = xh->xh_ncyls - totcyl;
	if (cyl <= 0)
	{
		xp.xp_tab[3].xp_boot = XP_NULL;
		xp.xp_tab[3].xp_s_h = 0;
		xp.xp_tab[3].xp_s_s = 0;
		xp.xp_tab[3].xp_s_c = 0;
		xp.xp_tab[3].xp_sys = XP_UNUSED;
		xp.xp_tab[3].xp_e_h = 0;
		xp.xp_tab[3].xp_e_s = 0;
		xp.xp_tab[3].xp_e_c = 0;
		xp.xp_tab[3].xp_start = 0;
		xp.xp_tab[3].xp_size = 0;
		dsize = 0;
		pf("\nThere will be no DOS area on this drive #%d\n",drv);
		return (1);
	}
	dsize = cyl * xh->xh_blkscyl;
	endcyl = totcyl + cyl - 1;
	if (sysyes)
		xp.xp_tab[3].xp_boot = XP_NULL;
	else
		xp.xp_tab[3].xp_boot = XP_BOOT;
	xp.xp_tab[3].xp_s_h = 0;
	xp.xp_tab[3].xp_s_s = 1 | ((totcyl >> 2) & 0300);
	xp.xp_tab[3].xp_s_c = totcyl;
	if ((dsize - tmpoff) > doscutoff)
		xp.xp_tab[3].xp_sys = XP_DOS_4;
	else
		xp.xp_tab[3].xp_sys = XP_DOS;
	xp.xp_tab[3].xp_e_h = xh->xh_nhead - 1;
	xp.xp_tab[3].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp.xp_tab[3].xp_e_c = endcyl;
	xp.xp_tab[3].xp_start = (totcyl * xh->xh_blkscyl) + tmpoff;
	xp.xp_tab[3].xp_size = dsize - tmpoff;
	dsize -= tmpoff;
	totcyl += cyl;
	if (!drv)
		hdsize = dsize;
	tmpoff = 0;
	pf("\nDOS area will have %d cylinders\n",cyl);
	pf("\nUse DOS's FDISK and FORMAT commands to create the DOS area.\n");
	return (0);
}

/*
 * PART II
 */
dousrarea()
{
	if (!usize)
		return (1);
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart II%s                 Build User Area\n\n",hlon,hloff);
	if (!prompt("Do you wish to create the user area?",0))
		return (1);
	if (checkfs("/dev/w0.usr",usize))
	{
	pf("\n%sWARNING:%s There is a file system already on the user area\n",
			hlon,hloff);
		if (!prompt("Do you wish to continue?",0))
			return (1);
	}
	sprintf(buf,"%d",usize);
	if (prompt("\nDo you wish to check for bad blocks on the user area?",0))
	{
		pf("\nThis will take about 1 minute per 100 blocks.\n\n");
		if (run("/bin/mkfs","mkfs","-b","/dev/w0.usr",buf,0))
			fatal("Error in accessing winchester (mkfs)\n");
	}
	else
	{
		pf("\nCreating a file system on the user area.\n");
		if (run("/bin/mkfs","mkfs","/dev/w0.usr",buf,0))
			fatal("Error in accessing winchester (mkfs)\n");
	}
	if (!prompt("\nDo you wish to load the user area from floppies?",0))
		return (1);
	pf("\nNumber of user floppies to be loaded (0 to 24) [7] : ");
	if ((usrflpy = getnum(0,24,7)) < 1)
		return (1);
	if (mount("/dev/w0.usr","/usr",0) < 0)
		fatal("Can't mount user area\n");
	chdir("/usr");
	sprintf(cntflpy,"xf%d",usrflpy);
	pf("\nTar the user area to the winchester from the floppy.\n");
	run("/bin/tar","tar",cntflpy,"/dev/rf0",0);
	chdir("/");
	umount("/dev/w0.usr");
	pf("\nUser area completed.");
	return (0);
}

/*
 * PART III
 */
dosysarea()
{
int	i, fd, fd2;

	if (!ssize)
		return (1);
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart III%s               Build System Area\n\n",hlon,hloff);
	if (!prompt("Do you wish to create the system area?",0))
		return (1);
	pf("\nCheck the swapping area for bad blocks.\n\n");
	if ((fd = open("/dev/w0.sys",2)) < 0)
		fatal("Error in accessing winchester\n");
	lseek(fd, ssize * 512L, 0);
	for (i = 0; i < swapsiz; i++)
	{
		if (read(fd, buf, 512) != 512)
		{
			pf("%sWARNING%s: bad block(s)\n",hlon,hloff);
			break;
		}
	}
	pf("%s%d blocks for swapping%s\n",hlon,i,hloff);
	if (tsize)
	{
		sprintf(buf,"%d",tsize);
		pf("\nCheck the temporary area for bad blocks.\n\n");
		pf("\nThis will take about 1 minute per 100 blocks.\n\n");
		run("/bin/mkfs","mkfs","-b","/dev/w0.tmp",buf,0);
	}
	if ((fd2 = open("/uboot/xtboot",0)) < 0)
		fatal("Cannot open boot file: /uboot/xtboot\n");
	if (read(fd2, buf, 512) != 512)
		fatal("Cannot read boot file: /uboot/xtboot\n");
	close(fd2);
	lseek(fd, 0L, 0);
	write(fd, buf, 512);
	close(fd);
	if (checkfs("/dev/w0.sys",ssize))
	{
	pf("\n%sWARNING:%s There is a file system already on the system area\n",
			hlon,hloff);
		if (!prompt("Do you wish to continue?",0))
			return (1);
	}
	sprintf(buf,"%d",ssize);
	pf("\nCheck the system area for bad blocks.\n\n");
	pf("\nThis will take about 1 minute per 100 blocks.\n\n");
	if (run("/bin/mkfs","mkfs","-b","/dev/w0.sys", buf, 0))
		fatal("Error in accessing winchester (mkfs)\n");
	if (!prompt("\nDo you wish to load the system area from floppies?",0))
		return (1);
	pf("\nNumber of system floppies to be loaded (0 to 24) [4] : ");
	if ((sysflpy = getnum(0,24,4)) < 1)
		return (1);
	if (mount("/dev/w0.sys", "/usr", 0) < 0)
		fatal("Can't mount system area\n");
	chdir("/usr");
	sprintf(cntflpy,"xf%d-",sysflpy);
	pf("\nTar the system area to the winchester from the floppy.\n");
	run("/bin/tar","tar",cntflpy,"/dev/rf0",".", 0);
	chdir("/");
	umount("/dev/w0.sys");
	pf("\nSystem area completed.");
	return (0);
}

/*
 * Insert serial number
 */
putserial()
{
int	fd;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	if ((fd = open("/dev/w0.sys",2)) < 0)
		fatal("Cannot open /dev/w0.sys\n");
	lseek(fd, SN_INIT, 0);
	write(fd, &serno, sizeof(serno));
	close(fd);
	return (0);
}

/*
 * Patch /etc/rc with tmp file system size.
 */
doetcrc()
{
int	i, k, fd;
char	*cp;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	if (mount("/dev/w0.sys","/usr",0) < 0)
		fatal("Cannot mount /dev/w0.sys\n");
	if ((fd = open("/usr/etc/rc",2)) < 0)
		fatal("Cannot open file /etc/rc\n");
	if ((i = read(fd, buf, 512)) > 0)
	{
		lseek(fd, 0L, 0);
		for (cp = buf; cp < &buf[i]; cp++)
		{
			if (strncmp(cp,"/etc/mkfs /dev/w0.tmp ",22) == 0)
			{
				if ((k = cp - buf) > i)
					break;
				write(fd, buf, k);
				write(fd, "/bin/rm -f /tmp/*", 17);
				while (*cp++ != '\n' && cp < &buf[i]);
				cp--;
				write(fd, cp, &buf[i]-cp);
				break;
			}
		}
	}
	close(fd);
	return (0);
}

/*
 * patch /etc/init with # of users
 */
putnusers()
{
int	i, fd;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	pf("\nNumber of users this system will have (1 to 16) [%d] : ",nusr);
	nusr = getnum(1,16,nusr);
	if ((fd = open("/usr/etc/init",2)) < 0)
		fatal("Cannot open /usr/etc/init\n"); 
	/* NUSR_INIT tells us positition of
	 * symbol determining # of users,
	 * offset from end of code area 
	 */
	read(fd, &header, sizeof(struct exec));
	lseek(fd, header.a_text+NUSR_INIT+sizeof(struct exec), 0);
	read(fd, &i, sizeof(i));
	if ((i < 1) || (i > 16)) /* check we're in right spot */
	{
		nusr = 0;
	}
	else
	{
		lseek(fd, header.a_text+NUSR_INIT+sizeof(header), 0);
		write(fd, &nusr, sizeof(nusr));
	}
	close(fd);
	return (0);
}

/*
 * Copy the venix kernel from floppy to winchester.
 */
dokernel()
{
int	a, i, n, fd, fd2;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	if ((fd2 = open("/venix",0)) < 0)
		fatal("Cannot open /venix\n");
	close(creat("/usr/venix",0444));
	if ((fd = open("/usr/venix",2)) < 0)
		fatal("Cannot create `venix' on the winchester\n");
	while ((i = read(fd2, buf, 512)) > 0)
		write(fd, buf, i);
	close(fd2);
	lseek(fd, 0L, 0);
	if (read(fd, &header, sizeof(struct exec)) != sizeof(struct exec) ||
		N_BADMAG(header))
		fatal("venix header is bad\n");
	/*
	 * Read symbol table from /venix so we can
	 * patch device numbers and timezone info
	 * and number of drives
	 */
	nlist("/usr/venix",symbols);
	for (pnl = symbols; *(pnl->n_name); ++pnl)
	{
		if (pnl->n_type == 0)
			fatal("Bad venix namelist\n");
	}
	for (i = 0; i < 7;)
	{
		symbols[i++].n_value += (long)sizeof(struct exec)
			+ (header.a_magic!=0407 ? header.a_text : 0);
	}
	/*
	 * change device numbers
	 */
	i = 0x0100;
	lseek(fd, (long)symbols[3].n_value, 0);
	write(fd, &i, sizeof(i));
	lseek(fd, (long)symbols[4].n_value, 0);
	write(fd, &i, sizeof(i));
	lseek(fd, (long)symbols[5].n_value, 0);
	i = 0x0101;
	write(fd, &i, sizeof(i));
	n = dotimezone();
	a = prompt("\nDoes daylight saving time ever apply here?",0);
	lseek(fd, (long)symbols[0].n_value, 0);
	write(fd, &n, sizeof(n));
	lseek(fd, (long)symbols[1].n_value, 0);
	write(fd, &a, sizeof(a));
	if (xt_ndrv)
	{
		lseek(fd, (long)symbols[6].n_value, 0);
		write(fd, &xt_ndrv, sizeof(xt_ndrv));
	}
	close(fd);
	sync();
	sync();
	sync();
	return (0);
}

/*
 * PART IV
 */
dotimezone()
{
int	n;


	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
zoneloop:
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart IV%s               Time Zone Selection\n\n",hlon,hloff);
	pf("    1 - Eastern   time zone (EST)\n");
	pf("    2 - Pacific   time zone (PST)\n");
	pf("    3 - Mountain  time zone (MST)\n");
	pf("    4 - Central   time zone (CST)\n");
	pf("    5 - Atlantic  time zone (AST)\n");
	pf("    6 - Japan     time zone (JST)\n");
	pf("    7 - Greenwich mean time zone (GMT)\n");
	pf("    8 - Other\n\n");
	pf("Enter a number corresponding to your time zone (1 to 8) [2] : ");
	n = getnum(1,8,2);
	if (n-- < 8)
	{
		pf("You have selected the %s zone. Are you sure?",tz[n].msg);
		if (!prompt("",0))
			goto zoneloop;
		n = tz[n].min;
	}
	else
	{
pf("Enter the minutes west of GMT (minus for east) (-720 to 720) [0] : ");
		n = getnum(-720,720,0);
	}
	return (n);
}

finish()
{
	if (xt_ndef == 0)
	{
		if (nusr)
		{
			pf("\nSystem will be %d user.",nusr);
		}
		else
		{
		pf("\n%sWARNING:%s configuration error will cause this system",
				hlon,hloff);
			pf(" to be single-user only.");
		}
	}
	pf("\n\n%sInstallation complete.%s\n",hlon,hloff);
	if (prompt("\nDo you wish to execute the shell?",0))
	{
		sync();
		pf("\nThe shell will be executed.\n\n");
		signal(SIGHUP,SIG_DFL);
		signal(SIGINT,SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
		sgbuf.sg_flags = ECHO|CRT|CRMOD;
		ioctl(1,TIOCSETP,&sgbuf);
		execl("/bin/sh","sh","-i",0);
		fatal("Cannot execute the shell `/bin/sh'.\n");
	}
	sync();
	sync();
	sync();
	pf("\nRemove XFER floppy and reboot.\n");
	while (1);
}

dumppart()
{
int	i;

pf("Partition   Status       Type       Start   End   Size(blk) Offset\n");
	for (i = 0; i < 4; i++ )
	{
		pf("    %d         %c     ",
			i + 1, xp.xp_tab[i].xp_boot == XP_BOOT ? 'A' : 'N' );
		switch (xp.xp_tab[i].xp_sys)
		{
			case XP_DOS:
				pf("     DOS      ");
				break;

			case XP_SYS:
				pf(" VENIX system ");
				break;

			case XP_TMP:
				pf("  VENIX temp  ");
				break;

			case XP_USR:
				pf("  VENIX user  ");
				break;

			case XP_UNUSED:
				pf("    Unused    ");
				break;

			default:
				pf("    Unknown   ");
		}
		pf(" %5d  ",xp.xp_tab[i].xp_s_c +
			((xp.xp_tab[i].xp_s_s & 0300) << 2));
		pf("%5d  ",xp.xp_tab[i].xp_e_c +
			((xp.xp_tab[i].xp_e_s & 0300) << 2));
		pf("%8D ",xp.xp_tab[i].xp_size);
		pf("%8D\n",xp.xp_tab[i].xp_start);
	}
	return (0);
}

prompt(string,high)
char	*string;
int	high;
{
	pf("%s%s%s (%sy%s or %sn%s) ",
		high ? hlon : "", string, high ? hloff : "",
		hlon,hloff,hlon,hloff);
	while (1)
	{
		switch (xgetchar())
		{
			case 'y':
			case 'Y':
				pf("%sYes%s\n",rvon,rvoff);
				return (1);

			case 'n':
			case 'N':
				pf("%sNo%s\n",rvon,rvoff);
				return (0);

			default:
				pf("\007");
		}
	}
}
	
char *
getstr(min,max,defstr)
int     min, max;
char    *defstr;
{
register char *cp;
char     c, cbuf[64];
int	 flags;

	ioctl(0,TIOCGETP,&sgbuf);
	flags = sgbuf.sg_flags;
	sgbuf.sg_flags = RAW;
	ioctl(0,TIOCSETP,&sgbuf);
	while (1)
	{
		pf("%s",rvon);
		for (cp = cbuf; cp < cbuf + max;)
		{
			read(0,&c,1);           
			if ((c == '\r' || c == '\n') && (cp > cbuf))
			{
				break;
			}
			else
			{
				if (c == DEL)
				{
					if (cp > cbuf)
					{
						pf("%s\b \b%s",rvoff,rvon);
						--cp;
					}
					else
					{
						pf("\007");
					}
				}
				else 
				{
					/* use the default */
					if ((c == '\r' || c == '\n') &&
						(cp == cbuf))
					{
						pf("%s\r\n",(cp = defstr));
						goto strout;
					}
					else    /* record character */
					{ 
						*cp++ = c;
						pf("%c",c);
					}
				}
			}
		}
		*cp = 0;
		cp = cbuf;
		pf("\r\n");
strout:
		pf("%s",rvoff);
		/*
		 * check that string length is bewteen bounds, 
		 */
		if (min <= strlen(cp) && strlen(cp) <= max)
		{
			sgbuf.sg_flags = flags;
			ioctl(0,TIOCSETP,&sgbuf);
			return (cp);
		}
		pf("\r\n%sInvalid string%s: please type again: ",
			hlon,hloff);
	}
}

checkrange(min,max,def)
int     *min, *max, *def;
{

	if (*min > *max)
		*min = *max;
	if (*def > *max)
		*def = *max;
	else
		if (*def < *min)
			*def = *min;
	return (0);
}

getnum(min,max,def)
int     min, max, def;
{
register char *cp;
char     c, cbuf[10];
int      r, flags;

	if (min > max)
		min = max;
	if (def > max)
		def = max;
	else
		if (def < min)
			def = min;
	ioctl(0,TIOCGETP,&sgbuf);
	flags = sgbuf.sg_flags;
	sgbuf.sg_flags = RAW;
	ioctl(0,TIOCSETP,&sgbuf);
	while (1)
	{
		pf("%s",rvon);
		for (cp = cbuf; cp < cbuf + 10;)
		{
			read(0,&c,1);           
			if      (isdigit(c) || 
				 (
				  (cp == cbuf) &&       /* first digit? */
				  (
				   (c == '+' && max >= 0) ||    /* '+' okay? */
				   (c == '-' && min < 0)        /* '-' okay? */
				  )
				 )
				)
			{ 
				*cp++ = c;
				pf("%c",c);
			}
			else
			{
				if ((c == '\r' || c == '\n') && (cp > cbuf))
				{
					pf("\r\n");
					break;
				}
				else
				{
					if (c == DEL)
					{
						if (cp > cbuf)
						{
							pf("%s\b \b%s",
								rvoff,rvon);
							--cp;
						}
						else
						{
							pf("\007");
						}
					}
					else 
					{
						/* use the default */
						if (    (c == '\r' ||
							c == '\n') &&
							(cp == cbuf)
						   )
						{
							pf("%d\r\n",(r = def));
							goto numout;
						}
						else
						{
							pf("\007"); /* beep */
						}
					}
				}
			}
		}
		*cp = 0;
		r = atoi(cbuf);
numout:
		pf("%s",rvoff);
		/*
		 * check number bewteen bounds, 
		 * allowing also for unsigned values
		 */
		if (min <= r && r <= max)
		{
			sgbuf.sg_flags = flags;
			ioctl(0,TIOCSETP,&sgbuf);
			return(r);
		}
		pf("\r\n%sInvalid number%s: please type again: ",
			hlon,hloff);
	}
}

run(name,v)
char    *name, *v;
{
register char **rv;
register int i;
int      stat;

	pf("\t");
	for (rv = &v; *rv != (char *)0; rv++)
		pf("%s%s%s ",hlon,*rv,hloff);
	pf("\n");
	if ((i = fork()) == 0)
	{
		execv(name, &v);
		fatal("BOOTABLE XFER floppy corrupted: cannot find %s\n",
			name);
	}
	while ((i != wait(&stat)));
	return (stat);
}

checkfs(name,fsize)
char     *name;
unsigned int fsize;
{
int     fd;
static  struct filsys fs;

	if ((fd = open(name, 0)) < 0)
	{
		pf("XFER: cannot open %s\n",name);
		return (1);
	}
	lseek( fd, 512L, 0);
	if (read(fd, &fs, sizeof(fs)) != sizeof(fs))
	{
		pf("XFER: Cannot read %s\n",name);
		close(fd);
		return (1);
	}
	close(fd);
	if ((fs.s_isize < (fsize / 3)) &&
		(fs.s_fsize  == fsize) &&
		(fs.s_nfree  <= 100) &&
		(fs.s_ninode <= 100))
			return (1);
	return (0);
}

fatal(f,s1,s2)
char    *f, s1, s2;
{
	pf("\n%sFATAL ERROR: %s",hlon,hloff);
	pf(f,s1,s2);
	pf("\nXFER halting.  Consult your manual for assistance\n");
	sync();
	pf("\nThe shell will be executed.\n\n");
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);
	sgbuf.sg_flags = ECHO|CRT|CRMOD;
	ioctl(1,TIOCSETP,&sgbuf);
	execl("/bin/sh","sh","-i",0);
	pf("\nCannot execute the shell `/bin/sh'.\n");
	sync();
	while (1);
}

xgetchar()
{
int     flags;
char    c;

	ioctl(0,TIOCGETP,&sgbuf);
	flags = sgbuf.sg_flags;
	sgbuf.sg_flags = RAW;
	ioctl(0,TIOCSETP,&sgbuf);
	read(0, &c, 1);
	sgbuf.sg_flags = flags;
	ioctl(0,TIOCSETP,&sgbuf);
	return (c == '\r' ? '\n' : c);
}

#ifdef  notdef
	/*
	 * This code belongs in dosysarea().
	 * Decide if 4 head or 2 head drive.
	 */
	{
		struct diskparm buf;

		if( (i = open("/dev/rw0.sys",0)) < 0 )
			fatal("Cannot open /dev/rw0.sys.\n");
		buf.d_nhead = 0;
		ioctl(i, I_GETDPP, &buf);
		close(i);
		if (buf.d_nhead == 2)
		{
		    if ((i = open("/uboot/xtboot.2hd",0)) < 0)
			fatal("Cannot find boot file (/uboot/xtboot.2hd).\n");
		}
		else
		{
		    if ((i = open("/uboot/xtboot",0)) < 0)
			fatal("Cannot find boot file (/uboot/xtboot).\n");
		}
	}
#endif  notdef
