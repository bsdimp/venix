/*	IBM floppy driver using BIOS (single & double sided).
*		Version 86/2.0		Dec 20, 1982
*			Edited: 4/26/84
*
*	Major performance benifits can be achieved by avoiding
*	BIOS and doing the i/o directly to the hardware.  But
*	some compatability would be lost.
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
****************************************************************/
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/user.h>

#define	NTRACK	40		/* tracks per diskette */
#define	NSECT	8		/* default sectors per track */
#define	RETRY	4		/* number of retys on error */
#define	RESET	0		/* BIOS commands */
#define	READ	2
#define	WRITE	3
#define	FORMAT	5

struct flpmsg {			/* convert error numbers to messages */
	unsigned char	num;
	char		*msg;
} flpmsg[] = {
	0x80,	"Time out",
	0x40,	"Bad seek",
	0x20,	"Bad NEC controller",
	0x10,	"Bad CRC",
	0x9,	"Bad DMA boundry",
	0x8,	"DMA overrun",
	0x4,	"Requested block not found",
	0x3,	"Write protected",
	0x2,	"Bad address mark",
	0x1,	"Bad command",
	0,	"Unkown error status",
};

struct devtab flptab;
struct buf *fl_bp;	/* temp buf pointer for raw i/o stradling of 64 kb */

flpstrategy(bp) register struct buf *bp; {
	register unsigned int bn;
	unsigned int head, sect, track, i, rdflg, p, nsect;
	unsigned int xmem, count, addr;
	unsigned char err;

	bn = bp->b_blkno;
	count = (255 - bp->b_wcount) >> 8;
	addr = (unsigned int)bp->b_addr;
	if( bp->b_flags&B_PHYS )
		xmem = bp->b_xmem<<12;
	else
		xmem = getds();
	rdflg = (bp->b_flags&B_READ) ? READ : WRITE;
	if( (bp->b_dev&04) != 0 )
		rdflg = FORMAT;
	err = 0;
	while( err==0 && count--!=0 ){
		/*
		 * Convert logical to physical:
		 *  000 = 8 sect, all top surface tracks then bottom
		 *  010 = 9 sect, "				   "
		 *  020 = 8 sect, top/bottom on track then next track
		 *  030 = 9 sect, "				    "
		 */
		nsect = NSECT;
		head = 0;
		if( bp->b_dev&010 )
			nsect++;
		track = bn/nsect;
		if( bp->b_dev&020 ){
			if( track & 01 )
				head++;
			track >>= 1;
		} else {
			if( track >= NTRACK ){
				head++;
				track = 2*NTRACK-1 - track;
			}		
		}
		sect = (bn%nsect) + 1;
		if( track>=NTRACK ){
			bp->b_flags |= B_ERROR;
			goto done;
		}
		/*
		 * Check for stradling of 64kb DMA boundry.
		 */
		if( addr > 0xFE00 ){
			/*
			 * Get a temporary buffer and xfer data through it.
			 */
			if( fl_bp == NULL )
				fl_bp = (struct buf *)getblk(NODEV);
			if( rdflg != READ ){
				p = spl6();
				i = u.u_ds;
				u.u_ds = xmem + 32;
				copyin(addr - 512, fl_bp->b_addr, 512);
				u.u_ds = i;
				splx(p);
			}
			for( i=0; i<RETRY; i++ ){
				if( (err = flpio( rdflg, 1, track, sect, head,
				    bp->b_dev&03, fl_bp->b_addr, getds() ))==0 )
					break;
				if( i >= RETRY/2 )
					flpio(RESET);
			}
			if( rdflg == READ ){
				p = spl6();
				i = u.u_ds;
				u.u_ds = xmem + 32;
				copyout(fl_bp->b_addr, addr - 512, 512);
				u.u_ds = i;
				splx(p);
			}
		} else for( i=0; i<RETRY; i++ ){
			if( (err = flpio( rdflg, 1, track, sect, head,
			    bp->b_dev&03, addr, xmem) )==0 )
				break;
			if( i >= RETRY/2 )
				flpio(RESET);
		}
		bn++;
		if( (addr += 512) < 512 )
			xmem += 01<<12;
	}
	if( err ){
		bp->b_flags |= B_ERROR;
		for( bn = 0; flpmsg[bn].num; bn++)
			if( flpmsg[bn].num == err )
				break;
		deverror( bp, flpmsg[bn].msg, err);
	}
	bp->b_resid = 0;
done:
	iodone(bp);
}

flpread(dev){ aphysio(flpstrategy, dev, B_READ); }

flpwrite(dev){ aphysio(flpstrategy, dev, B_WRITE); }

