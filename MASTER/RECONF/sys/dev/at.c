/*
* NOTE: This driver assumes that during booting, the firmware will
*       have setup the drive characteristics in the controller.
*/
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <sys/devparm.h>
#include <sys/xtblk0.h>

#ifdef	ATASI

#define NDRV	4			/* default number of drives */
#define NPART	4			/* number of partitions per drive */
#define NUNIT	8			/* number of units (minor dev number) */
#define MAXHEAD	9			/* maximum number of heads */
#define NHEAD	7			/* default number of heads */
#define SECHEAD	17			/* default sectors per head */
#define NCYLS	321			/* default number of cylinders */
#define CNTF	0x03			/* stepping speed 13 us */

#else	ATASI

#define NDRV	2			/* default number of drives */
#define NPART	4			/* number of partitions per drive */
#define NUNIT	8			/* number of units (minor dev number) */
#define MAXHEAD	5			/* maximum number of heads */
#define NHEAD	4			/* default number of heads */
#define SECHEAD	17			/* default sectors per head */
#define NCYLS	305			/* default number of cylinders */
#define CNTF	0x05			/* stepping speed 70 us */

#endif	ATASI

#define BLKSCYL	NHEAD*SECHEAD           /* default blocks  per cylinder */
#define MAXBLKS	MAXHEAD*SECHEAD         /* maximum blocks  per cylinder */
#define BLKSIZE	BSIZE                   /* default bytes   per block */
#define NTRACK	NHEAD*NCYLS             /* default tracks  per drive */

#define MAXFER	(512*32)                /* maximum DMA xfer size in bytes */

/*
	defines for xd_tab_in
*/
#define	XD_READ	0x0001
#define	XD_LOAD	0x0002

#define	tabinmem(drv)	(xd_parm[drv].xd_tab_in & XD_READ)
#define	drvloaded(drv)	(xd_parm[drv].xd_tab_in & XD_LOAD)
#define	tabinon(drv)	(xd_parm[drv].xd_tab_in |=  XD_READ)
#define	tabinoff(drv)	(xd_parm[drv].xd_tab_in &= ~XD_READ)
#define	drvlodon(drv)	(xd_parm[drv].xd_tab_in |=  XD_LOAD)
#define	drvlodoff(drv)	(xd_parm[drv].xd_tab_in &= ~XD_LOAD)

/*
	The following define elements have to correspond
	to the xd_parm elemts.
*/
#define FSTPARM {\
			NPART,\
			NUNIT,\
			NTRACK,\
			MAXHEAD,\
			SECHEAD,\
			NCYLS,\
			MAXBLKS,\
			BLKSIZE,\
			CNTF,\
			MAXFER,\
			XD_LOAD,\
			0\
		}

#define STDPARM {\
			NPART,\
			NUNIT,\
			NTRACK,\
			NHEAD,\
			SECHEAD,\
			NCYLS,\
			BLKSCYL,\
			BLKSIZE,\
			CNTF,\
			MAXFER,\
			XD_LOAD,\
			0\
		}

#define IOADD   ((struct xt *)0x320)    /* i/o register origin */
#define IOINT   5                       /* interrupt request number */

#define DMACHAN         3               /* DMA channel number */
#define DMAPAGE         0x82            /* channel dep: 1 = 0x83, 3 = 0x82 */
#define DMAREAD         (0x44+DMACHAN)
#define DMAWRITE        (0x48+DMACHAN)
#define DMAADDR         (2*DMACHAN)
#define DMACOUNT        (DMAADDR+1)

/*
 * XT winchester controller commands and formats.
 */
#define RSTAT   0x03            /* request status information */
#define READ    0x08            /* read data */
#define WRITE   0x0a            /* write data */

/*
 * XT per atasi drive local parameters
 */
struct  xd
	{
		unsigned int    xd_npart;       /* phys partitions per drive */
		unsigned int    xd_nunit;       /* logical units per drive */
		unsigned int    xd_ntrack;      /* tracks per drive */
		unsigned int    xd_nhead;       /* heads per drive */
		unsigned int    xd_sechd;       /* sectors per head */
		unsigned int    xd_ncyls;       /* cylinders per drive */
		unsigned int    xd_blkscyl;     /* blocks per cylinder */
		unsigned int    xd_blksize;     /* bytes per block */
		unsigned int    xd_cntf;        /* step rate for drive */
		unsigned int    xd_maxfer;      /* maximum transfer size */
		/* bit 0: 0 = partition table not read, 1 = yes */
		/* bit 1: 0 = drive not loaded, 1 = drive loaded */
		unsigned int    xd_tab_in;
		unsigned long   xd_pcnt;	/* pass thru strategy cnt */
	}
	xd_parm[NDRV] =                         /* initialize with defaults */
	{
		FSTPARM,
		STDPARM,
#ifdef	ATASI
		STDPARM,
		STDPARM,
#endif	ATASI
	};

unsigned int    xt_ndrv  = NDRV;	/* a variable for flexibility sake */
unsigned int    xt_nunit = NUNIT;	/* a variable for flexibility sake */

struct  xt                      /* i/o register layout */
	{
		unsigned char x_data;
		unsigned char x_stat;
		unsigned char x_selt;
		unsigned char x_mask;
	};

struct  xt_cmd                  /* command packet */
	{
		unsigned char c_cmmd;
		unsigned char c_head;
		unsigned char c_sect;
		unsigned char c_cyln;
		unsigned char c_bcnt;
		unsigned char c_ctrl;
	} xt_cmd;

struct  xtmsg                   /* convert error numbers to messages */
	{
		char    num;
		char    *msg;
	}
	xtmsg[] =
	{
		0xFF,   "Controller timeout",
		0x21,   "Illegal disk address",
		0x20,   "Invalid command",
		0x19,   "Bad track",
		0x18,   "Correctable data error",
		0x15,   "Seek error",
		0x14,   "Sector not found",
		0x12,   "No address mark",
		0x11,   "Data error",
		0x10,   "ID error",
		0x06,   "No track 0",
		0x04,   "Drive not ready",
		0x03,   "Write fault",
		0x02,   "No seek complete",
		0x01,   "No index signal",
		0x00,   "Unknown error",
	};

struct  {                               /* disk partitions */
		unsigned nblock;        /* number of 512 byte blocks */
		unsigned oblock;        /* offset in blocks for first block */
	}
	xt_sizes[NDRV][NUNIT] =
	{
#ifdef	ATASI

/*maj   min*/   {                       /*  Drive 0                     */
/* 1     0 */         { 7258,   1 },    /*  3.7  5248 system, 2000 swap */
/* 1     1 */         { 1785,   7259 }, /*   .9  tmp & pipes            */
/* 1     2 */         { 17255,  9044 }, /*  8.8  user                   */
/* 1     3 */         { 11900,  26299 },/*  6.1  dos                    */
/* 1     4 */         { 0,      0 },
/* 1     5 */         { 0,      0 },
/* 1     6 */         { 0,      0 },
/* 1     7 */         { -1,     0 },    /* 19.5  physical               */
		},
/*maj   min*/   {                       /*  Drive 1                     */
/* 1     8 */         { 0,      0 },
/* 1     9 */         { 0,      0 },
/* 1    10 */         { 38198,  1 },    /* 19.5  user                   */
/* 1    11 */         { 0,      0 },
/* 1    12 */         { 0,      0 },
/* 1    13 */         { 0,      0 },
/* 1    14 */         { 0,      0 },
/* 1    15 */         { -1,     0 },    /* 19.5  physical               */ 
		},
/*maj   min*/   {                       /*  Drive 2                     */
/* 1    16 */         { 0,      0 },
/* 1    17 */         { 0,      0 },
/* 1    18 */         { 38198,  1 },    /* 19.5  user                   */
/* 1    19 */         { 0,      0 },
/* 1    20 */         { 0,      0 },
/* 1    21 */         { 0,      0 },
/* 1    22 */         { 0,      0 },
/* 1    23 */         { -1,     0 },    /* 19.5  physical               */ 
		},
/*maj   min*/   {                       /*  Drive 3                     */
/* 1    24 */         { 0,      0 },
/* 1    25 */         { 0,      0 },
/* 1    26 */         { 38198,  1 },    /* 19.5  user                   */
/* 1    27 */         { 0,      0 },
/* 1    28 */         { 0,      0 },
/* 1    29 */         { 0,      0 },
/* 1    30 */         { 0,      0 },
/* 1    31 */         { -1,     0 },    /* 19.5  physical               */ 
		},

#else	ATASI

/*maj   min*/   {                       /*  Drive 0                     */
/* 1     0 */         { 3739,   1 },    /*  1.8  2980 system, 750 swap  */
/* 1     1 */         { 340,    3740 }, /*   .2  tmp & pipes            */
/* 1     2 */         { 16660,  4080 }, /*  8.0  user                   */
/* 1     3 */         { 0,      0 },	/*   .0  dos                    */
/* 1     4 */         { 0,      0 },
/* 1     5 */         { 0,      0 },
/* 1     6 */         { 0,      0 },
/* 1     7 */         { -1,     0 },    /*  10.0 physical               */
		},
/*maj   min*/   {                       /*  Drive 1                     */
/* 1     8 */         { 0,      0 },
/* 1     9 */         { 0,      0 },
/* 1    10 */         { 20739,  1 },    /*  10.0 user                   */
/* 1    11 */         { 0,      0 },
/* 1    12 */         { 0,      0 },
/* 1    13 */         { 0,      0 },
/* 1    14 */         { 0,      0 },
/* 1    15 */         { -1,     0 },    /*  10.0 physical               */ 
		},

#endif	ATASI
	};

#define b_cylin b_resid         /* redefine for readability */

struct  devtab  xttab;

struct  buf    *xt_bp;  /* temp buf pointer for raw i/o stradling of 64kb */

char    xtcntrl;        /* control:     0 = normal, 1 = more, -1 = stradle */
char    xtvec[9];       /* interrupt transfer vector */
char    xtnoerr;        /* set during tests to supress error messages */
int	xt_debug = 0;

xtstrategy(abp)
struct	buf *abp;
{
register struct buf *bp;
register struct xp *xp;
register struct xh *xh;
register int unit, drive, part, tunit;
static	 char xtfirst = 0;		/* driver already called */
int	 s, xtintr();

	drive = minor(abp->b_dev) / NUNIT;
	unit  = minor(abp->b_dev) % NUNIT;
	if (++xd_parm[drive].xd_pcnt <= 0)
	{
		if (xt_debug)
			printf("xtstrategy: drive = %u: pass count overflow\n",
				drive);
		xd_parm[drive].xd_pcnt = 0;
	}
	if (xtfirst == 0)
	{
		xtfirst++;
		/*
		 * Set up the interrupt vector.
		 */
		setiva(0x34, xtintr, xtvec);

		/*
		 * Calculate number of heads.  First try max,
		 * if error then decrease by 1, try again.
		 */
		xtnoerr++;
		while (xd_parm[0].xd_nhead > 0)
		{
			/*
				Must insure major device number to be the one of
				the block device and not character.
				It happens to be that major device number for
				the winchester is 1 and character is 5.
				Also, preserve the original minor device number
				and then OR in the minor number for physical
				partition.  Then try to read the last
				physical block on the disk as dictated by
				the supplied number of heads and sectors
				per head variables.
			*/
			bp = (struct buf *)bread(((abp->b_dev & 0x01ff) | 7),
				xd_parm[0].xd_sechd *
				(xd_parm[0].xd_nhead - 1));
			brelse(bp);
			if (bp->b_flags & B_ERROR)
			{
				extern lbolt;

				xd_parm[0].xd_nhead--;
				u.u_error = 0;
				sleep(&lbolt,PWAIT);	/* delay */
			}
			else
				break;
		}
		xtnoerr = 0;
		xd_parm[0].xd_blkscyl = xd_parm[0].xd_nhead *
			xd_parm[0].xd_sechd;
		if (xt_debug)
			printf("HD: strat: drive 0 has %u heads: pass %u\n",
				xd_parm[0].xd_nhead,xd_parm[0].xd_pcnt);
		/*
		 * Get a buffer for raw i/o which stradles a 64kb boundry.
		 */
		xt_bp = (struct buf *)getblk(NODEV);
	}
	if ((tabinmem(drive) == 0) && (unit < xd_parm[drive].xd_npart))
	{
		if (xt_debug)
		printf("HD: strat: first access: drive=%u unit=%u pass=%u\n",
			drive,unit,xd_parm[drive].xd_pcnt);
		tabinon(drive);
		/*
			Must insure major device number to be the one of
			the block device and not character.
			It happens to be that major device number for
			the winchester is 1 and character is 5.
			Also, preserve the original minor device number
			and then OR in the minor number for physical
			partition.
		*/
		bp = (struct buf *)bread(((abp->b_dev & 0x01ff) | 7), 0);
		xp = (struct xp  *)bp->b_addr;
		xh = (struct xh  *)&xp->xp_code[XH_START];
		if ((xh->xh_sig1 == XH_SIG1) && (xh->xh_sig2 == XH_SIG2))
		{
			xd_parm[drive].xd_npart   = xh->xh_npart;
			xd_parm[drive].xd_nunit   = xh->xh_nunit;
			xd_parm[drive].xd_ntrack  = xh->xh_ntrack;
			xd_parm[drive].xd_nhead   = xh->xh_nhead;
			xd_parm[drive].xd_sechd   = xh->xh_sechd;
			xd_parm[drive].xd_ncyls   = xh->xh_ncyls;
			xd_parm[drive].xd_blkscyl = xh->xh_blkscyl;
			xd_parm[drive].xd_blksize = xh->xh_blksize;
			xd_parm[drive].xd_cntf    = xh->xh_cntf;
			xd_parm[drive].xd_maxfer  = xh->xh_maxfer;
		}
		else
		{
#ifdef	ATASI
printf("WARNING: no parm header on block 0, using 7 head 19.5 Meg default\n");
#else	ATASI
printf("WARNING: no parm header on block 0, using 4 head 10 Meg default\n");
#endif	ATASI
		}
		if (xp->xp_sig == XP_SIG)
		{
			for (part = 0; part < xd_parm[drive].xd_npart; part++)
			{
				switch (xp->xp_tab[part].xp_sys)
				{
					case XP_SYS:
					case XP_SYS_1:
					case XP_SYS_2:
					case XP_SYS_3:
						tunit = 0;
						break;

					case XP_TMP:
					case XP_TMP_1:
					case XP_TMP_2:
					case XP_TMP_3:
						tunit = 1;
						break;

					case XP_USR:
					case XP_USR_1:
					case XP_USR_2:
					case XP_USR_3:
						tunit = 2;
						break;

					case XP_DOS:
					case XP_DOS_4:
						tunit = 3;
						break;

					default:
						if (xt_debug)
	printf("HD: unused partition %u on drive %u unit %u\n",part,drive,unit);
					/*
						usage of part in follwing
						two assignments needs to
						be rethought!!!
					*/
					xt_sizes[drive][part].nblock = 0;
					xt_sizes[drive][part].oblock = 0;
					continue;
				}
				xt_sizes[drive][tunit].nblock =
					xp->xp_tab[part].xp_size;
				xt_sizes[drive][tunit].oblock =
					xp->xp_tab[part].xp_start;
			}
		}
		else
		{
	printf("WARNING: invalid signature on block 0 of drive %u unit %u\n",
			drive,unit);
		}
		brelse(bp);
	}

	/*
	 * Verify minor device and request block range.
	 */
	bp = abp;
	if ((unit >= NUNIT) || (drive >= xt_ndrv) ||
		((bp->b_blkno + ((255 - bp->b_wcount) >> 8)) >
		xt_sizes[drive][unit].nblock))
	{
		if (xt_debug && (xtnoerr == 0))
		{
			printf("HD: strat: something is wrong: pass %u\n",
				xd_parm[drive].xd_pcnt);
			printf("    unit    = %u\t>=  NUNIT     = %u\n",
				unit,NUNIT);
			printf("    drive   = %u\t>=  xt_ndrv   = %u\n",
				drive,xt_ndrv);
			printf("    b_blkno = %u\t    b_wcount  = %u\n",
				bp->b_blkno,bp->b_wcount);
			printf("    blkno   = %u\t>=  nblock    = %u\n",
				(bp->b_blkno + ((255 - bp->b_wcount) >> 8)),
				xt_sizes[drive][unit].nblock);
		}
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

	/*
	 * Calculate approximate cylinder for sorting,
	 * queue the request, and
	 * start the transfer if none in progress.
	 */
	bp->b_cylin = (bp->b_blkno + xt_sizes[drive][unit].oblock) /
		xd_parm[drive].xd_blkscyl;
	spl5();
	disksort(&xttab, bp);
	if (xttab.d_active == 0)
		xtstart();
	spl0();
}

xtstart()
{
register struct buf *bp;
register unsigned int count;
unsigned int addr, unit, drive;
struct	 ib
	 {
		char	lobyte;
		char	hibyte;
	 };

	if ((bp = xttab.d_actf) == NULL)
		return;
	xttab.d_active++;
	xtcntrl = 0;
	drive = minor(bp->b_dev) / NUNIT;
	unit = minor(bp->b_dev) % NUNIT;

	/*
	 * Set up DMA controller.
	 */
	io_outb(0x0c, 1);		/* Clear first/last FF */
	if (bp->b_flags & B_READ)
	{
		io_outb(0x0c, DMAREAD);
		io_outb(0x0b, DMAREAD);
	}
	else
	{
		io_outb(0x0c, DMAWRITE);
		io_outb(0x0b, DMAWRITE);
	}
	addr = (unsigned int)bp->b_addr;
	if (bp->b_flags & B_PHYS)
	{
		/*
		 * Check for stradling of 64kb boundry.
		 */
		if (addr > 0xFE00)
		{
			xtcntrl--;
			if ((bp->b_flags & B_READ) == 0)
			{
				addr = spl6();
				count = u.u_ds;
				u.u_ds = (bp->b_xmem << 12) + 32;
				copyin(bp->b_addr - 512, xt_bp->b_addr, 512);
				u.u_ds = count;
				splx(addr);
			}
			addr = (unsigned int) xt_bp->b_addr;
			goto cont;
		}
		io_outb(DMAPAGE, bp->b_xmem);
		io_outb(DMAADDR, ((struct ib *)&bp->b_addr)->lobyte);
		io_outb(DMAADDR, ((struct ib *)&bp->b_addr)->hibyte);
		count = ((-bp->b_wcount << 1) + 0x1FF) & ~0x1FF;

		/*
		 * Long DMA transfers will delay the RAM refresh of memory
		 * on the I/O bus so that memory can get corrupted.
		 *
		 * Also check for 64kb DMA wraparound.
		 */
		if ((count > xd_parm[drive].xd_maxfer) || 
		    ((((unsigned int) bp->b_addr) + count) < count))
		{
			xtcntrl++;
			if (count > xd_parm[drive].xd_maxfer &&
			    ((((unsigned int) bp->b_addr) + count) >= count))
			{
				count = xd_parm[drive].xd_maxfer;
			}
			else
			{
				count = -((unsigned int) bp->b_addr); 
				if ((count & 0x1FF) != 0)    /* bad boundry */
					count &= ~0x1FF;
				else
					bp->b_xmem++;
			}
			bp->b_addr += count;
			bp->b_wcount += count >> 1;
		}
	}
	else
	{
cont:		count = addr >> 4;
		count += getds();
		io_outb(DMAPAGE, count >> 12);
		io_outb(DMAADDR, (count << 4) + (addr & 017));
		io_outb(DMAADDR, count >> 4);
		count = 512;
	}
	xt_cmd.c_bcnt = count >> 9;
	count--;
	io_outb(DMACOUNT, count);
	io_outb(DMACOUNT, count >> 8);

	/*
	 * Set up the rest of XT atasi command packet.
	 */
	count = bp->b_blkno + xt_sizes[drive][unit].oblock;
	xt_cmd.c_sect = count % xd_parm[drive].xd_sechd;
	count /= xd_parm[drive].xd_sechd;
	xt_cmd.c_head = (count % xd_parm[drive].xd_nhead) | (drive << 5);
	count /= xd_parm[drive].xd_nhead;
	xt_cmd.c_cyln = count;
	xt_cmd.c_sect |= (count >> 2) & 0xC0;
	xt_cmd.c_ctrl = xd_parm[drive].xd_cntf;

	/*
	 * Start the action.
	 */
	if (xtcmd(bp->b_flags & B_READ ? READ : WRITE, 0x3))
	{
		xtcntrl = 2;
		xtintr();               /* print the error */
		return;
	}
	io_outb(0x0a, DMACHAN);
	io_outb(0x21, io_inb(0x21) & ~(01 << IOINT));
}

xtread(dev)
{
register int drive, unit;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= xt_ndrv) || (unit >= NUNIT))
	{
		printf("cdev read drive = %u  >=  xt_ndrv = %u\n",
			drive,xt_ndrv);
		printf("cdev read unit  = %u  >=  NUNIT   = %u\n",
			unit,NUNIT);
		printf("cdev read dev   = 0x%x\n",dev);
		u.u_error = ENXIO;
		return;
	}
	if (drvloaded(drive) == 0)
	{
		printf("cdev read drive = %u is not loaded\n",drive);
		printf("cdev read unit  = %u  dev = 0x%x\n",unit,dev);
		u.u_error = ENXIO;
		return;
	}
	if (xt_debug)
		printf("cdev read drive = %u  unit = %u  dev = 0x%x\n",
			drive,unit,dev);
	aphysio(xtstrategy, dev, B_READ);
}

xtwrite(dev)
{
register int drive, unit;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= xt_ndrv) || (unit >= NUNIT))
	{
		printf("cdev write drive = %u  >=  xt_ndrv = %u\n",
			drive,xt_ndrv);
		printf("cdev write unit  = %u  >=  NUNIT   = %u\n",
			unit,NUNIT);
		printf("cdev write dev   = 0x%x\n",dev);
		u.u_error = ENXIO;
		return;
	}
	if (drvloaded(drive) == 0)
	{
		printf("cdev write drive = %u is not loaded\n",drive);
		printf("cdev write unit  = %u  dev = 0x%x\n",unit,dev);
		u.u_error = ENXIO;
		return;
	}
	if (xt_debug)
		printf("cdev write drive = %u  unit = %u  dev = 0x%x\n",
			drive,unit,dev);
	aphysio(xtstrategy, dev, B_WRITE);
}

xtioctl(dev,cmd,addr)
char    *addr;
{
register int unit, drive;
struct   diskparm buf;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= xt_ndrv) || (unit >= NUNIT))
	{
		printf("drive = %u  >=  xt_ndrv = %u\n",drive,xt_ndrv);
		printf("unit  = %u  >=  NUNIT   = %u\n",unit,NUNIT);
		printf("dev   = 0x%x\n",dev);
		u.u_error = ENXIO;
		return;
	}
	switch (cmd)
	{
		case I_GETDPP:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if (unit >= xd_parm[drive].xd_npart)
			{
				printf("unit  = %u  >=  xd_npart = %u\n",
					unit,xd_parm[drive].xd_npart);
				printf("drive = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
			{
		printf("partition table for drive %u is not in memory\n",
			drive);
				printf("unit = %u  dev = 0x%x\n",unit,dev);
				/* change to block device */
				dev = makedev(1,minor(dev));
				brelse(bread(dev,0));
			}
			buf.d_nblock = xt_sizes[drive][unit].nblock;
			buf.d_offset = xt_sizes[drive][unit].oblock;
			buf.d_nsect  = xd_parm[drive].xd_sechd;
			buf.d_nhead  = xd_parm[drive].xd_nhead;
			buf.d_ntrack = xd_parm[drive].xd_ntrack;
			if (copyout(&buf, addr, sizeof(buf)))
				u.u_error = EFAULT;
			return;

		case I_REREAD:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			tabinoff(drive);
			/* change to block device */
			dev = makedev(1,minor(dev));
			brelse(bread(dev,0));
			return;

		case I_SETNDRV:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
			{
		printf("partition table for drive %u is not in memory\n",
			drive);
				printf("unit = %u  dev = 0x%x\n",unit,dev);
				/* change to block device */
				dev = makedev(1,minor(dev));
				brelse(bread(dev,0));
			}
			if (copyin(addr, &buf, sizeof(buf)))
			{
				u.u_error = EFAULT;
				return;
			}
			if ((buf.d_nblock == 0) && (buf.d_offset == 0))
			{
				if ((0 < buf.d_ntrack) && (buf.d_ntrack <=NDRV))
					xt_ndrv = buf.d_ntrack;
				else
					u.u_error = EINVAL;
			}
			else
				u.u_error = EINVAL;
			return;

		case I_LOAD:
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (drvloaded(drive))
				return;
			drvlodon(drive);
			tabinoff(drive);
			/* change to block device */
			dev = makedev(1,minor(dev));
			brelse(bread(dev,0));
			return;

		case I_UNLOAD:
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (drvloaded(drive) == 0)
				return;
			drvlodoff(drive);
			tabinoff(drive);
			return;

		case I_DUMP:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
			{
		printf("partition table for drive %u is not in memory\n",
			drive);
				printf("unit = %u  dev = 0x%x\n",unit,dev);
				/* change to block device */
				dev = makedev(1,minor(dev));
				brelse(bread(dev,0));
			}
			printf("HD: drive   = %u\n",drive);
			printf("HD: unit    = %u\n",unit);
			printf("HD: npart   = %u\n",xd_parm[drive].xd_npart);
			printf("HD: nunit   = %u\n",xd_parm[drive].xd_nunit);
			printf("HD: ntrack  = %u\n",xd_parm[drive].xd_ntrack);
			printf("HD: nhead   = %u\n",xd_parm[drive].xd_nhead);
			printf("HD: sechd   = %u\n",xd_parm[drive].xd_sechd);
			printf("HD: ncyls   = %u\n",xd_parm[drive].xd_ncyls);
			printf("HD: blkscyl = %u\n",xd_parm[drive].xd_blkscyl);
			printf("HD: blksize = %u\n",xd_parm[drive].xd_blksize);
			printf("HD: cntf    = %u\n",xd_parm[drive].xd_cntf);
			printf("HD: maxfer  = %u\n",xd_parm[drive].xd_maxfer);
			return;

		default:
			u.u_error = EINVAL;
			return;
	}
}

xtcmd(command,mask)
{
register char *x;
register int i;

	xt_cmd.c_cmmd = command;
	io_outb(&IOADD->x_selt,0);
	io_outb(&IOADD->x_mask,mask);
	for (i = 1000; (io_inb(&IOADD->x_stat) & 0xD) != 0xD;)
		if (i-- == 0)
			return(-1);
	x = (char *)&xt_cmd;
	i = sizeof(xt_cmd);
	while (i--)
		io_outb(&IOADD->x_data, *x++);
	return(0);
}

xtintr()
{
register struct buf *bp;
register int i;
char	 j = 0xFF;

	bp = xttab.d_actf;
	if (xtcntrl == 2)
		goto error;
	io_outb(0x21, io_inb(0x21) | (01 << IOINT));
	io_outb(0x20, 0x20);
	io_outb(0x0a, 07);
	io_outb(&IOADD->x_mask,0);
	if (xttab.d_active != 0)
	{
		if (io_inb(&IOADD->x_data) & 02)
		{
			if (xtcmd(RSTAT,0))
				goto error;
			for (i = 100; (io_inb(&IOADD->x_stat) & 01) == 0;)
				if (i-- == 0)
					goto error;
			j = io_inb(&IOADD->x_data) & 0x3f;
error:			if (xtnoerr == 0)
			{
				for (i = 0; xtmsg[i].num != 0; i++)
					if (xtmsg[i].num == j)
						break;
				deverror(bp, xtmsg[i].msg, j);
			}
			io_outb(&IOADD->x_stat,0);
			if (j != 0x18)
			{
				bp->b_flags |= B_ERROR;
				goto done;
			}
		}
		/*
		 * Finished with this transfer ?
		 */
		if (xtcntrl)
		{
			if (xtcntrl < 0)
			{
				if (bp->b_flags & B_READ)
				{
					i = u.u_ds;
					u.u_ds = (bp->b_xmem << 12) + 32;
					copyout(xt_bp->b_addr,
						bp->b_addr - 512, 512);
					u.u_ds = i;
				}
				bp->b_addr += 512;
				bp->b_xmem++;
				bp->b_wcount += 256;
			}
			if (bp->b_wcount >= 0)
				goto done;
			bp->b_blkno += xt_cmd.c_bcnt;
		}
		else
		{
done:			xttab.d_active = 0;
			xttab.d_actf = bp->av_forw;
			bp->b_resid = 0;
			iodone(bp);
		}
	}
	xtstart();
}

