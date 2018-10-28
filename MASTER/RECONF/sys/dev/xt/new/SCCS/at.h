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
