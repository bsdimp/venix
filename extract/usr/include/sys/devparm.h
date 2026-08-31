/*
 * ioctl calls to disk drivers.
 *	This must be applied to the character ("raw") interface.
 */

#define	I_FORMAT	(('X'<<8)|1)	/* format a disk */
#define	I_GETDPP	(('X'<<8)|2)	/* get disk partition parameters */

struct diskparm {		/*** I_GETDPP structure ***/
	long	d_nblock;	/* no. of blocks in this partition */
	long	d_offset;	/* no. of blocks offset from start of disk */
	short	d_nsect;	/* no. of sectors on disk */
	short	d_nhead;	/* no. of heads on disk */
	short	d_ntrack;	/* no. of tracks on disk (0 == unknown) */
};
