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
