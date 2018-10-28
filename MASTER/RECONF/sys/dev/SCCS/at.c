/*
* NOTE: This driver assumes that during booting, the firmware will
*       have setup the drive characteristics in the controller.
*/
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <sys/devparm.h>
#include <sys/atblk0.h>

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
	defines for ad_tab_in
*/
#define	AD_READ	0x0001
#define	AD_LOAD	0x0002

#define	tabinmem(drv)	(ad_parm[drv].ad_tab_in & AD_READ)
#define	drvloaded(drv)	(ad_parm[drv].ad_tab_in & AD_LOAD)
#define	tabinon(drv)	(ad_parm[drv].ad_tab_in |=  AD_READ)
#define	tabinoff(drv)	(ad_parm[drv].ad_tab_in &= ~AD_READ)
#define	drvlodon(drv)	(ad_parm[drv].ad_tab_in |=  AD_LOAD)
#define	drvlodoff(drv)	(ad_parm[drv].ad_tab_in &= ~AD_LOAD)

/*
	The following define elements have to correspond
	to the ad_parm elemts.
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
			AD_LOAD\
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
			AD_LOAD\
		}

#define IOADD   ((struct at *)0x320)    /* i/o register origin */
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
struct  ad
	{
		unsigned int    ad_npart;       /* phys partitions per drive */
		unsigned int    ad_nunit;       /* logical units per drive */
		unsigned int    ad_ntrack;      /* tracks per drive */
		unsigned int    ad_nhead;       /* heads per drive */
		unsigned int    ad_sechd;       /* sectors per head */
		unsigned int    ad_ncyls;       /* cylinders per drive */
		unsigned int    ad_blkscyl;     /* blocks per cylinder */
		unsigned int    ad_blksize;     /* bytes per block */
		unsigned int    ad_cntf;        /* step rate for drive */
		unsigned int    ad_maxfer;      /* maximum transfer size */
		/* bit 0: 0 = partition table not read, 1 = yes */
		/* bit 1: 0 = drive not loaded, 1 = drive loaded */
		unsigned int    ad_tab_in;
	}
	ad_parm[NDRV] =                         /* initialize with defaults */
	{
		FSTPARM,
		STDPARM,
#ifndef	ATASI
		STDPARM,
		STDPARM,
#endif	ATASI
	};

unsigned int    at_ndrv = NDRV; /* a variable for flexibility sake */

struct  at                      /* i/o register layout */
	{
		unsigned char x_data;
		unsigned char x_stat;
		unsigned char x_selt;
		unsigned char x_mask;
	};

struct  at_cmd                  /* command packet */
	{
		unsigned char c_cmmd;
		unsigned char c_head;
		unsigned char c_sect;
		unsigned char c_cyln;
		unsigned char c_bcnt;
		unsigned char c_ctrl;
	} at_cmd;

struct  atmsg                   /* convert error numbers to messages */
	{
		char    num;
		char    *msg;
	}
	atmsg[] =
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
	at_sizes[NDRV][NUNIT] =
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

struct  devtab  attab;

struct  buf    *at_bp;  /* temp buf pointer for raw i/o stradling of 64kb */

char    atcntrl;        /* control:     0 = normal, 1 = more, -1 = stradle */
char    atvec[9];       /* interrupt transfer vector */
char    atnoerr;        /* set during tests to supress error messages */

atstrategy(abp)
struct	buf *abp;
{
register struct buf *bp;
register struct ap *ap;
register struct ah *ah;
register int unit, drive, part, tunit;
static	 char atfirst = 0;		/* driver already called */
int	 s, atintr();

	drive = minor(abp->b_dev) / NUNIT;
	unit  = minor(abp->b_dev) % NUNIT;
	if (atfirst == 0)
	{
		atfirst++;
		/*
		 * Set up the interrupt vector.
		 */
		setiva(0x34, atintr, atvec);

		/*
		 * Calculate number of heads.  First try max,
		 * if error then decrease by 1, try again.
		 */
		atnoerr++;
		while (ad_parm[0].ad_nhead > 0)
		{
			bp = (struct buf *)bread(abp->b_dev | 7,
				ad_parm[0].ad_sechd *
				(ad_parm[0].ad_nhead - 1));
			brelse(bp);
			if (bp->b_flags & B_ERROR)
			{
				extern lbolt;

				ad_parm[0].ad_nhead--;
				u.u_error = 0;
				sleep(&lbolt,PWAIT);	/* delay */
			}
			else
				break;
		}
		atnoerr = 0;
		ad_parm[0].ad_blkscyl = ad_parm[0].ad_nhead *
			ad_parm[0].ad_sechd;
		printf("HD: strat: drive 0 has %u heads\n",
			ad_parm[0].ad_nhead);
		/*
		 * Get a buffer for raw i/o which stradles a 64kb boundry.
		 */
		at_bp = (struct buf *)getblk(NODEV);
	}
	if ((tabinmem(drive) == 0) && (unit < ad_parm[drive].ad_npart))
	{
		printf("HD: strat: first access: drive=%u unit=%u\n",
			drive,unit);
		tabinon(drive);
		bp = (struct buf *)bread(abp->b_dev | 7, 0);
		ap = (struct ap  *)bp->b_addr;
		ah = (struct ah  *)&ap->ap_code[AH_START];
		if ((ah->ah_sig1 == AH_SIG1) && (ah->ah_sig2 == AH_SIG2))
		{
			ad_parm[drive].ad_npart   = ah->ah_npart;
			ad_parm[drive].ad_nunit   = ah->ah_nunit;
			ad_parm[drive].ad_ntrack  = ah->ah_ntrack;
			ad_parm[drive].ad_nhead   = ah->ah_nhead;
			ad_parm[drive].ad_sechd   = ah->ah_sechd;
			ad_parm[drive].ad_ncyls   = ah->ah_ncyls;
			ad_parm[drive].ad_blkscyl = ah->ah_blkscyl;
			ad_parm[drive].ad_blksize = ah->ah_blksize;
			ad_parm[drive].ad_cntf    = ah->ah_cntf;
			ad_parm[drive].ad_maxfer  = ah->ah_maxfer;
		}
		else
		{
#ifdef	ATASI
printf("WARNING: no parm header on block 0, using 7 head 19.5 Meg default\n");
#else	ATASI
printf("WARNING: no parm header on block 0, using 4 head 10 Meg default\n");
#endif	ATASI
		}
		if (ap->ap_sig == AP_SIG)
		{
			for (part = 0; part < ad_parm[drive].ad_npart; part++)
			{
				switch (ap->ap_tab[part].ap_sys)
				{
					case AP_SYS:
					case AP_SYS_1:
					case AP_SYS_2:
					case AP_SYS_3:
						tunit = 0;
						break;

					case AP_TMP:
					case AP_TMP_1:
					case AP_TMP_2:
					case AP_TMP_3:
						tunit = 1;
						break;

					case AP_USR:
					case AP_USR_1:
					case AP_USR_2:
					case AP_USR_3:
						tunit = 2;
						break;

					case AP_DOS:
					case AP_DOS_4:
						tunit = 3;
						break;

					default:
	printf("HD: unused partition %u on drive %u unit %u\n",part,drive,unit);
					/*
						usage of part in follwing
						two assignments needs to
						be rethought!!!
					*/
					at_sizes[drive][part].nblock = 0;
					at_sizes[drive][part].oblock = 0;
					continue;
				}
				at_sizes[drive][tunit].nblock =
					ap->ap_tab[part].ap_size;
				at_sizes[drive][tunit].oblock =
					ap->ap_tab[part].ap_start;
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
	if ((unit >= NUNIT) || (drive >= at_ndrv) ||
		((bp->b_blkno + ((255 - bp->b_wcount) >> 8)) >
		at_sizes[drive][unit].nblock))
	{
		printf("HD: strat: something is wrong\n");
		printf("    unit  = %u  >=  NUNIT   = %u\n",unit,NUNIT);
		printf("    drive = %u  >=  at_ndrv = %u\n",drive,at_ndrv);
		printf("    blkno = %u  >=  nblock  = %u\n",
			(bp->b_blkno + ((255 - bp->b_wcount) >> 8)),
			at_sizes[drive][unit].nblock);
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

	/*
	 * Calculate approximate cylinder for sorting,
	 * queue the request, and
	 * start the transfer if none in progress.
	 */
	bp->b_cylin = (bp->b_blkno + at_sizes[drive][unit].oblock) /
		ad_parm[drive].ad_blkscyl;
	spl5();
	disksort(&attab, bp);
	if (attab.d_active == 0)
		atstart();
	spl0();
}

atstart()
{
register struct buf *bp;
register unsigned int count;
unsigned int addr, unit, drive;
struct	 ib
	 {
		char	lobyte;
		char	hibyte;
	 };

	if ((bp = attab.d_actf) == NULL)
		return;
	attab.d_active++;
	atcntrl = 0;
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
			atcntrl--;
			if ((bp->b_flags & B_READ) == 0)
			{
				addr = spl6();
				count = u.u_ds;
				u.u_ds = (bp->b_xmem << 12) + 32;
				copyin(bp->b_addr - 512, at_bp->b_addr, 512);
				u.u_ds = count;
				splx(addr);
			}
			addr = (unsigned int) at_bp->b_addr;
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
		if ((count > ad_parm[drive].ad_maxfer) || 
		    ((((unsigned int) bp->b_addr) + count) < count))
		{
			atcntrl++;
			if (count > ad_parm[drive].ad_maxfer &&
			    ((((unsigned int) bp->b_addr) + count) >= count))
			{
				count = ad_parm[drive].ad_maxfer;
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
	at_cmd.c_bcnt = count >> 9;
	count--;
	io_outb(DMACOUNT, count);
	io_outb(DMACOUNT, count >> 8);

	/*
	 * Set up the rest of XT atasi command packet.
	 */
	count = bp->b_blkno + at_sizes[drive][unit].oblock;
	at_cmd.c_sect = count % ad_parm[drive].ad_sechd;
	count /= ad_parm[drive].ad_sechd;
	at_cmd.c_head = (count % ad_parm[drive].ad_nhead) | (drive << 5);
	count /= ad_parm[drive].ad_nhead;
	at_cmd.c_cyln = count;
	at_cmd.c_sect |= (count >> 2) & 0xC0;
	at_cmd.c_ctrl = ad_parm[drive].ad_cntf;

	/*
	 * Start the action.
	 */
	if (atcmd(bp->b_flags & B_READ ? READ : WRITE, 0x3))
	{
		atcntrl = 2;
		atintr();               /* print the error */
		return;
	}
	io_outb(0x0a, DMACHAN);
	io_outb(0x21, io_inb(0x21) & ~(01 << IOINT));
}

atintr()
{
register struct buf *bp;
register int i;
char	 j = 0xFF;

	bp = attab.d_actf;
	if (atcntrl == 2)
		goto error;
	io_outb(0x21, io_inb(0x21) | (01 << IOINT));
	io_outb(0x20, 0x20);
	io_outb(0x0a, 07);
	io_outb(&IOADD->x_mask,0);
	if (attab.d_active != 0)
	{
		if (io_inb(&IOADD->x_data) & 02)
		{
			if (atcmd(RSTAT,0))
				goto error;
			for (i = 100; (io_inb(&IOADD->x_stat) & 01) == 0;)
				if (i-- == 0)
					goto error;
			j = io_inb(&IOADD->x_data) & 0x3f;
error:			if (atnoerr == 0)
			{
				for (i = 0; atmsg[i].num != 0; i++)
					if (atmsg[i].num == j)
						break;
				deverror(bp, atmsg[i].msg, j);
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
		if (atcntrl)
		{
			if (atcntrl < 0)
			{
				if (bp->b_flags & B_READ)
				{
					i = u.u_ds;
					u.u_ds = (bp->b_xmem << 12) + 32;
					copyout(at_bp->b_addr,
						bp->b_addr - 512, 512);
					u.u_ds = i;
				}
				bp->b_addr += 512;
				bp->b_xmem++;
				bp->b_wcount += 256;
			}
			if (bp->b_wcount >= 0)
				goto done;
			bp->b_blkno += at_cmd.c_bcnt;
		}
		else
		{
done:			attab.d_active = 0;
			attab.d_actf = bp->av_forw;
			bp->b_resid = 0;
			iodone(bp);
		}
	}
	atstart();
}

atcmd(command,mask)
{
register char *x;
register int i;

	at_cmd.c_cmmd = command;
	io_outb(&IOADD->x_selt,0);
	io_outb(&IOADD->x_mask,mask);
	for (i = 1000; (io_inb(&IOADD->x_stat) & 0xD) != 0xD;)
		if (i-- == 0)
			return(-1);
	x = (char *)&at_cmd;
	i = sizeof(at_cmd);
	while (i--)
		io_outb(&IOADD->x_data, *x++);
	return(0);
}

atread(dev)
{
register int drive, unit;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= at_ndrv) || (unit >= NUNIT))
	{
		u.u_error = ENXIO;
		return;
	}
	if (drvloaded(drive) == 0)
	{
		u.u_error = ENXIO;
		return;
	}
	aphysio(atstrategy, dev, B_READ);
}

atwrite(dev)
{
register int drive, unit;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= at_ndrv) || (unit >= NUNIT))
	{
		u.u_error = ENXIO;
		return;
	}
	if (drvloaded(drive) == 0)
	{
		u.u_error = ENXIO;
		return;
	}
	aphysio(atstrategy, dev, B_WRITE);
}

atioctl(dev,cmd,addr)
char    *addr;
{
register int unit, drive;
struct   diskparm buf;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= at_ndrv) || (unit >= NUNIT))
	{
		u.u_error = ENXIO;
		return;
	}
	switch (cmd)
	{
		case I_GETDPP:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if (unit >= ad_parm[drive].ad_npart)
			{
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
				brelse(bread(dev,0));
			buf.d_nblock = at_sizes[drive][unit].nblock;
			buf.d_offset = at_sizes[drive][unit].oblock;
			buf.d_nsect = ad_parm[drive].ad_sechd;
			buf.d_nhead = ad_parm[drive].ad_nhead;
			buf.d_ntrack = ad_parm[drive].ad_ntrack;
			if (copyout(&buf, addr, sizeof(buf)))
				u.u_error = EFAULT;
			return;

		case I_REREAD:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			tabinoff(drive);
			brelse(bread(dev,0));
			return;

		case I_SETNDRV:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
				brelse(bread(dev,0));
			if (copyin(addr, &buf, sizeof(buf)))
			{
				u.u_error = EFAULT;
				return;
			}
			if ((buf.d_nblock == 0) && (buf.d_offset == 0))
			{
				if ((0 < buf.d_ntrack) && (buf.d_ntrack <=NDRV))
					at_ndrv = buf.d_ntrack;
				else
					u.u_error = EINVAL;
			}
			else
				u.u_error = EINVAL;
			return;

		case I_LOAD:
			if (drvloaded(drive))
				return;
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			drvlodon(drive);
			tabinoff(drive);
			brelse(bread(dev,0));
			return;

		case I_UNLOAD:
			if (drvloaded(drive) == 0)
				return;
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			drvlodoff(drive);
			tabinoff(drive);
			return;

		case I_DUMP:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
				brelse(bread(dev,0));
			printf("HD: drive   = %u\n",drive);
			printf("HD: unit    = %u\n",unit);
			printf("HD: npart   = %u\n",ad_parm[drive].ad_npart);
			printf("HD: nunit   = %u\n",ad_parm[drive].ad_nunit);
			printf("HD: ntrack  = %u\n",ad_parm[drive].ad_ntrack);
			printf("HD: nhead   = %u\n",ad_parm[drive].ad_nhead);
			printf("HD: sechd   = %u\n",ad_parm[drive].ad_sechd);
			printf("HD: ncyls   = %u\n",ad_parm[drive].ad_ncyls);
			printf("HD: blkscyl = %u\n",ad_parm[drive].ad_blkscyl);
			printf("HD: blksize = %u\n",ad_parm[drive].ad_blksize);
			printf("HD: cntf    = %u\n",ad_parm[drive].ad_cntf);
			printf("HD: maxfer  = %u\n",ad_parm[drive].ad_maxfer);
			return;

		default:
			u.u_error = EINVAL;
			return;
	}
}

