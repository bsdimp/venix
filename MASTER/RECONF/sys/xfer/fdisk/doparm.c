#include	"fdisk.h"

parminit(xp,drive)
struct	xp *xp;
int	drive;
{
struct	xh *xh;

	/*
		The defaults are for 10MB 4 head full height
	*/
	xh = (struct xh *)&xp->xp_code[XH_START];
	if ((xh->xh_sig1 != XH_SIG1) || (xh->xh_sig2 != XH_SIG2))
	{
		xh->xh_sig1 = XH_SIG1;
		xh->xh_ndrv = drive;
		strcpy(xh->xh_name,"Winche");
		xh->xh_name[6] = drive + '0';
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
	xh->xh_ndrv = drive;
	xh->xh_name[6] = drive + '0';
	xh->xh_name[7] = 0;
	return (0);
}

parmnew(xp,drive)
struct	xp *xp;
int	drive;
{
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	printf("Drive Number                    [%d] : %s%d%s\n",
		xh->xh_ndrv,rvon,xh->xh_ndrv,rvoff);
	printf("Drive Name                      [%s] : ",
		xh->xh_name);
	strcpy(xh->xh_name,getstr(1,7,xh->xh_name));
	printf("Number of Partitions (%d to %d) [%d] : ",
		0,4,xh->xh_npart);
	xh->xh_npart = getnum(0,4,xh->xh_npart);
	printf("Number of Units      (%d to %d) [%d] : ",
		0,8,xh->xh_nunit);
	xh->xh_nunit = getnum(0,8,xh->xh_nunit);
	printf("Number of Heads      (%d to %d) [%d] : ",
		0,8,xh->xh_nhead);
	xh->xh_nhead = getnum(0,8,xh->xh_nhead);
	printf("Number of Cylinders  (%d to %d) [%d] : ",
		0,999,xh->xh_ncyls);
	xh->xh_ncyls = getnum(0,999,xh->xh_ncyls);
	printf("Sectors per Head     (%d to %d) [%d] : ",
		0,99,xh->xh_sechd);
	xh->xh_sechd = getnum(0,99,xh->xh_sechd);
	printf("Tracks per Drive                [%d] : %s%d%s\n",
		xh->xh_ntrack,rvon,
		xh->xh_nhead * xh->xh_ncyls,rvoff);
	xh->xh_ntrack = xh->xh_nhead * xh->xh_ncyls;
	printf("Blocks per Cylinder             [%d] : %s%d%s\n",
		xh->xh_blkscyl,rvon,
		xh->xh_nhead * xh->xh_sechd,rvoff);
	xh->xh_blkscyl = xh->xh_nhead * xh->xh_sechd;
	printf("Block Size                      [%d] : %s%d%s\n",
		xh->xh_blksize,rvon,xh->xh_blksize,rvoff);
	printf("Step Speed Code      (%d to %d) [%d] : ",
		0,9,xh->xh_cntf);
	xh->xh_cntf = getnum(0,9,xh->xh_cntf);
	printf("Max Single transfer  (%d to %d) [%d] : ",
		0,maxfer,xh->xh_maxfer);
	xh->xh_maxfer = getnum(0,maxfer,xh->xh_maxfer);
	return (0);
}

parmdump(xp,drive)
struct	xp *xp;
int	drive;
{
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	printf("Drive Number         : %8u %s*%s\n",xh->xh_ndrv,hlon,hloff);
	printf("Drive Name           : %8s\n",xh->xh_name);
	printf("Number of Partitions : %8u\n",xh->xh_npart);
	printf("Number of Units      : %8u\n",xh->xh_nunit);
	printf("Number of Heads      : %8u\n",xh->xh_nhead);
	printf("Number of Cylinders  : %8u\n",xh->xh_ncyls);
	printf("Sectors per Head     : %8u\n",xh->xh_sechd);
	printf("Tracks per Drive     : %8u %s*%s\n",xh->xh_ntrack,hlon,hloff);
	printf("Blocks per Cylinder  : %8u %s*%s\n",xh->xh_blkscyl,hlon,hloff);
	printf("Block Size           : %8u %s*%s\n",xh->xh_blksize,hlon,hloff);
	printf("Step Speed Code      : %8u\n",xh->xh_cntf);
	printf("Max Single transfer  : %8u\n",xh->xh_maxfer);
	printf("%s* generated automatically%s\n",hlon,hloff);
	return (0);
}
