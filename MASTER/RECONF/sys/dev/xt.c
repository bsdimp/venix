#include <sys/param.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <sys/devparm.h>
#include <sys/xtblk0.h>

#ifdef	ATASI

#define	NDRV	4			/* maximum number of drives */
#define	NHEAD	7			/* default number of heads */
#define	CNTF	0x03			/* stepping speed 13us */

#else	ATASI

#define	NDRV	2			/* maximum number of drives */
#define	NHEAD	4			/* default number of heads */
#define	CNTF	0x05			/* stepping speed 70us */

#endif	ATASI

#define	IOADD	((struct xt *)0x320)	/* i/o register origin */
#define	SECHEAD	17			/* default sectors per head */
#define	IOINT	5			/* interrupt request number */
#define	MAXFER	(512*32)		/* maximum DMA xfer size in bytes */

#define	DMACHAN		3		/* DMA channel number */
#define	DMAPAGE		0x82		/* channel dep: 1 = 0x83, 3 = 0x82 */
#define	DMAREAD		(0x44+DMACHAN)
#define	DMAWRITE	(0x48+DMACHAN)
#define	DMAADDR		(2*DMACHAN)
#define	DMACOUNT	(DMAADDR+1)

/*
 * XT winchester controller commands and formats.
 */
#define	RSTAT	0x03		/* request status information */
#define	READ	0x08		/* read data */
#define	WRITE	0x0a		/* write data */

struct	xt	/* i/o register layout */
	{
		unsigned char x_data;
		unsigned char x_stat;
		unsigned char x_selt;
		unsigned char x_mask;
	};

unsigned int	xt_nhead = NHEAD;	/* for benefit of external changes */
unsigned int	xt_sechd = SECHEAD;
unsigned int	xt_mxfer = MAXFER;

struct	xt_cmd	/* command packet */
	{
		unsigned char c_cmmd;
		unsigned char c_head;
		unsigned char c_sect;
		unsigned char c_cyln;
		unsigned char c_bcnt;
		unsigned char c_ctrl;
	} xt_cmd;

struct	xtmsg	/* convert error numbers to messages */
	{
		char	num;
		char	*msg;
	}
	xtmsg[] =
	{
		0xFF,	"Controller timeout",
		0x21,	"Illegal disk address",
		0x20,	"Invalid command",
		0x19,	"Bad track",
		0x18,	"Correctable data error",
		0x15,	"Seek error",
		0x14,	"Sector not found",
		0x12,	"No address mark",
		0x11,	"Data error",
		0x10,	"ID error",
		0x06,	"No track 0",
		0x04,	"Drive not ready",
		0x03,	"Write fault",
		0x02,	"No seek complete",
		0x01,	"No index signal",
		0x00,	"Unknown error",
	};

struct	{				/* disk partitions */
		unsigned nblock;	/* number of 512 byte blocks */
		unsigned oblock;	/* offset in blocks for first block */
	}
	xt_sizes[8*NDRV] =
	{
#ifdef	ATASI
					/***	Drive 0		      ***/
		7258,	1,		/*  3.7  5248 system, 2000 swap	*/
		1785,	7259,		/*   .9  tmp & pipes		*/
		17255,	9044,		/*  8.8  user			*/
		11900,	26299,		/*  6.1  dos			*/
		0,	0,
		0,	0,
		0,	0,
		-1,	0,		/* 19.5  physical		*/

					/***	Drive 1		      ***/
		38199,	0,		/* 19.5  user			*/
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		-1,	0,		/* 19.5  physical		*/ 

					/***	Drive 2		      ***/
		38199,	0,		/* 19.5  user			*/
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		-1,	0,		/* 19.5  physical		*/ 

					/***	Drive 3		      ***/
		38199,	0,		/* 19.5  user			*/
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		-1,	0,		/* 19.5  physical		*/ 
#else	ATASI
					/***	Drive 0		      ***/
		3739,	1,		/*  1.8  2980 system, 750 swap	*/
		340,	3740,		/*   .2  tmp & pipes		*/
		16660,	4080,		/*  8.0  user			*/
		0,	0,		/*   .0  dos			*/
		0,	0,
		0,	0,
		0,	0,
		-1,	0,		/* 10.0  physical		*/

					/***	Drive 1		      ***/
		20740,	0,		/* 10.0  user			*/
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		0,	0,
		-1,	0,		/* 10.0  physical		*/ 
#endif	ATASI
	};

#define	b_cylin	b_resid		/* redefine for readability */

struct	devtab	xttab;
struct  buf    *xt_bp;	/* temp buf pointer for raw i/o stradling of 64kb */
char	xt_tab_in;	/* flag:	0 = partition table not read, 1 = ok */
char	xtcntrl;	/* control:	0 = normal, 1 = more, -1 = stradle */
char	xtvec[9];	/* interrupt transfer vector */
char	xtnoerr;	/* set during tests to supress error messages */
int	xt_debug = 0;

xtstrategy(abp)
struct	buf *abp;
{
register struct buf *bp;
register int unit;
static	 char xtfirst = 0;	/* driver already called */
int	 xtintr();

	if (xtfirst == 0)
	{
		xtfirst++;
		/*
		 * Set up the interrupt vector.
		 */
		setiva(0x34, xtintr, xtvec);

		/*
		 * Calculate number of heads.  First try max,
		 * if error then decrease by 2, try again.
		 */
		xtnoerr++;
		while (xt_nhead > 0)
		{
			bp = (struct buf *)bread(abp->b_dev | 7,
				xt_sechd * (xt_nhead - 1));
			brelse(bp);
			if (bp->b_flags & B_ERROR)
			{
				extern lbolt;

				xt_nhead -= 2;
				u.u_error = 0;
				sleep(&lbolt,PWAIT);	/* delay */
			}
			else
				break;
		}
		xtnoerr = 0;

		/*
		 * Get a buffer for raw i/o which stradles a 64kb boundry.
		 */
		xt_bp = (struct buf *)getblk(NODEV);
	}

	/*
	 * Read in partition table from physical block 0
	 * if this is the first 0-3 partition table access.
	 */
	if ((xt_tab_in == 0) && ((abp->b_dev & 037) < 4))
	{
		int	part;
				
		xt_tab_in++;
		bp = (struct buf *)bread( abp->b_dev | 7, 0);
		if (((struct xp *)bp->b_addr)->xp_sig == XP_SIG)
		{
		    for (unit = 0; unit < 4; unit++)
		    {
			switch (((struct xp *)bp->b_addr)->xp_tab[unit].xp_sys)
			{
				case XP_SYS:
					part = 0;
					break;

				case XP_TMP:
					part = 1;
					break;

				case XP_USR:
					part = 2;
					break;

				default:
					part = 3;
					break;
			}
			xt_sizes[part].nblock =
			    ((struct xp *)bp->b_addr)->xp_tab[unit].xp_size;
			xt_sizes[part].oblock =
			    ((struct xp *)bp->b_addr)->xp_tab[unit].xp_start;
		    }
		}
		brelse(bp);
	}

	/*
	 * Verify minor device and request block range.
	 */
	unit = (bp = abp)->b_dev&037;
	if ((unit >= (8 * NDRV)) ||
	   (bp->b_blkno + ((255-bp->b_wcount) >> 8)) > xt_sizes[unit].nblock)
	{
		if (xt_debug && (xtnoerr == 0))
		{
			printf("HD: strat: something is wrong\n");
			printf("    dev   = 0x%x\n",bp->b_dev);
			printf("    unit  = %u\t>=  (8 * NDRV) = %u\n",
				unit,(8 * NDRV));
			printf("    b_blkno = %u\t    b_wcount  = %u\n",
				bp->b_blkno,bp->b_wcount);
			printf("    blkno   = %u\t>=  nblock    = %u\n",
				(bp->b_blkno + ((255 - bp->b_wcount) >> 8)),
				xt_sizes[unit].nblock);
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
	bp->b_cylin = (bp->b_blkno + xt_sizes[unit].oblock) / (NHEAD * SECHEAD);
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
unsigned int addr;
struct	 ib
	 {
		char	lobyte;
		char	hibyte;
	 };

	if ((bp = xttab.d_actf) == NULL)
		return;
	xttab.d_active++;
	xtcntrl = 0;

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
		if ((count > xt_mxfer) || 
		    ((((unsigned int) bp->b_addr) + count) < count))
		{
			xtcntrl++;
			if ((count > xt_mxfer) &&
			    ((((unsigned int) bp->b_addr) + count) >= count))
			{
				count = xt_mxfer;
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
	 * Set up the rest of XT command packet.
	 */
	count = bp->b_blkno + xt_sizes[addr = bp->b_dev & 037].oblock;
	xt_cmd.c_sect = count % xt_sechd;
	count /= xt_sechd;
	xt_cmd.c_head = (count % xt_nhead) | (addr&030) << 2;
	count /= xt_nhead;
	xt_cmd.c_cyln = count;
	xt_cmd.c_sect |= (count >> 2) & 0xC0;
	xt_cmd.c_ctrl = CNTF;

	/*
	 * Start the action.
	 */
	if (xtcmd(bp->b_flags & B_READ ? READ : WRITE, 0x3))
	{
		xtcntrl = 2;
		xtintr();		/* print the error */
		return;
	}
	io_outb(0x0a, DMACHAN);
	io_outb(0x21, io_inb(0x21) & ~(01 << IOINT));
}

xtread(dev)
{
	aphysio(xtstrategy, dev, B_READ);
}

xtwrite(dev)
{
	aphysio(xtstrategy,dev,B_WRITE);
}

xtioctl(dev,cmd,addr)
char	*addr; 
{
register int i;
struct	 diskparm buf;

	if (cmd == I_GETDPP)
	{
		if (xt_tab_in == 0)	/* make sure data is accurate */
			brelse(bread(dev,0));
		i = dev & 037;
		buf.d_nblock = xt_sizes[i].nblock;
		buf.d_offset = xt_sizes[i].oblock;
		buf.d_nsect = xt_sechd;
		buf.d_nhead = xt_nhead;
		buf.d_ntrack = 0;		/* don't care */
		if (copyout( &buf, addr, sizeof(buf)))
			u.u_error = EFAULT;
	}
	else
		u.u_error = EINVAL;
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

