#include	"fdisk.h"

partinit(xp,drive)
struct	xp *xp;
int	drive;
{
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	return (0);
}

partact(xp,drive)
struct	xp *xp;
int	drive;
{
int	i, j;

	for (i = 0, j = 0; i < 4; i++)
	{
		if (xp->xp_tab[i].xp_boot == XP_BOOT)
			j = i + 1;
		xp->xp_tab[i].xp_boot = XP_NULL;
		
	}
	printf("Enter new active boot partition (%d to %d) [%d] : ",0,4,j);
	j = getnum(0,4,j);
	if (j)
		xp->xp_tab[j-1].xp_boot = XP_BOOT;
	return (0);
}

dosyspart(xp,drive)
struct	xp *xp;
int	drive;
{
int	cyl, blks, endcyl, bsize, maxbsize;
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	sysmin = dsysmin;
	printf("Sizes are in cylinders (%d blocks per cylinder)\n",
		xh->xh_blkscyl);
	checkrange(&sysmin,&sysmax,&sysmin);
redosys:
	printf("Desired size of system area (%d to %d) [%d] : ",
		sysmin,sysmax,sysmin);
	if ((cyl = getnum(sysmin,sysmax,sysmin)) <= 0)
	{
		xp->xp_tab[0].xp_boot = XP_NULL;
		xp->xp_tab[0].xp_s_h = 0;
		xp->xp_tab[0].xp_s_s = 0;
		xp->xp_tab[0].xp_s_c = 0;
		xp->xp_tab[0].xp_sys = XP_UNUSED;
		xp->xp_tab[0].xp_e_h = 0;
		xp->xp_tab[0].xp_e_s = 0;
		xp->xp_tab[0].xp_e_c = 0;
		xp->xp_tab[0].xp_start = 0;
		xp->xp_tab[0].xp_size = 0;
		sysyes = swapsiz = totcyl = ssize = 0;
		tmpmax = TMPMAX;
		tmpoff = 1;
		printf("There will be no SYS area on this drive #%d\n",drive);
		return (1);
	}
	swapsiz = dswapsiz;
	maxbsize = (xh->xh_blkscyl * sysmax) - 1;
	bsize = (xh->xh_blkscyl * cyl) - 1;
	if (bsize < (dssize + tblsize + (bsize / 10)))
	{
	printf("The system area is too small for both file system and swap.\n");
		if (bsize >= maxbsize)
			printf("But since this is the maximum possible ...\n");
		else
		{
			printf("Allocate more cylinders to system area.\n");
			goto redosys;
		}
	}
	blks = (bsize - dssize - tblsize) / swapint;
	blks *= swapint;
	checkrange(&swapsiz,&blks,&swapsiz);
	printf("Number of blocks for swap (intervals of %d) (%d to %d) [%d] : ",
		swapint,swapsiz,blks,swapsiz);
	swapsiz = getnum(swapsiz,blks,swapsiz);
	swapsiz = (swapsiz / swapint) * swapint;
	printf("%sUsing %d blocks at end of system area for swapping\n%s",
		hlon,swapsiz,hloff);
	ssize = bsize - tblsize - swapsiz;
	endcyl = cyl - 1;
	xp->xp_tab[0].xp_boot = XP_BOOT;
	xp->xp_tab[0].xp_s_h = 0;
	xp->xp_tab[0].xp_s_s = 1;
	xp->xp_tab[0].xp_s_c = 0;
	xp->xp_tab[0].xp_sys = XP_SYS;
	xp->xp_tab[0].xp_e_h = xh->xh_nhead - 1;
	xp->xp_tab[0].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp->xp_tab[0].xp_e_c = endcyl;
	xp->xp_tab[0].xp_start = 1;
	xp->xp_tab[0].xp_size = cyl * xh->xh_blkscyl - 1;
	totcyl = cyl;
	tmpmax = minimum((xh->xh_ncyls - cyl),TMPMAX);
	tmpoff = 0;
	sysyes = 1;
	return (0);
}

dotmppart(xp,drive)
struct	xp *xp;
int	drive;
{
int	cyl, endcyl;
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	tmpmin = dtmpmin;
	if (tmpmax < 1)
		goto notmparea;
	checkrange(&tmpmin,&tmpmax,&tmpmin);
	printf("Desired size of temp area (%d to %d) [%d] : ",
		tmpmin,tmpmax,tmpmin);
	if ((cyl = getnum(tmpmin,tmpmax,tmpmin)) <= 0)
	{
notmparea:
		xp->xp_tab[1].xp_boot = XP_NULL;
		xp->xp_tab[1].xp_s_h = 0;
		xp->xp_tab[1].xp_s_s = 0;
		xp->xp_tab[1].xp_s_c = 0;
		xp->xp_tab[1].xp_sys = XP_UNUSED;
		xp->xp_tab[1].xp_e_h = 0;
		xp->xp_tab[1].xp_e_s = 0;
		xp->xp_tab[1].xp_e_c = 0;
		xp->xp_tab[1].xp_start = 0;
		xp->xp_tab[1].xp_size = 0;
		tsize = 0;
		printf("There will be no TMP area on this drive #%d\n",drive);
		return (1);
	}
	tsize = cyl * xh->xh_blkscyl;
	endcyl = totcyl + cyl - 1;
	xp->xp_tab[1].xp_boot = XP_NULL;
	xp->xp_tab[1].xp_s_h = 0;
	xp->xp_tab[1].xp_s_s = 1 | ((totcyl >> 2) & 0300);
	xp->xp_tab[1].xp_s_c = totcyl;
	xp->xp_tab[1].xp_sys = XP_TMP;
	xp->xp_tab[1].xp_e_h = xh->xh_nhead - 1;
	xp->xp_tab[1].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp->xp_tab[1].xp_e_c = endcyl;
	xp->xp_tab[1].xp_start = (totcyl * xh->xh_blkscyl) + tmpoff;
	xp->xp_tab[1].xp_size = tsize - tmpoff;
	tsize -= tmpoff;
	totcyl += cyl;
	tmpoff = 0;
	return (0);
}

dousrpart(xp,drive)
struct	xp *xp;
int	drive;
{
int	cyl, endcyl;
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	usrmin = dusrmin;
	usrmax = xh->xh_ncyls - totcyl;
	if (usrmax < 1)
		goto nousrpart;
	checkrange(&usrmin,&usrmax,&usrmin);
	printf("Desired size of user area (%d to %d) [%d] : ",
		usrmin,usrmax,usrmin);
	if ((cyl = getnum(usrmin,usrmax,usrmin)) <= 0)
	{
nousrpart:
		xp->xp_tab[2].xp_boot = XP_NULL;
		xp->xp_tab[2].xp_s_h = 0;
		xp->xp_tab[2].xp_s_s = 0;
		xp->xp_tab[2].xp_s_c = 0;
		xp->xp_tab[2].xp_sys = XP_UNUSED;
		xp->xp_tab[2].xp_e_h = 0;
		xp->xp_tab[2].xp_e_s = 0;
		xp->xp_tab[2].xp_e_c = 0;
		xp->xp_tab[2].xp_start = 0;
		xp->xp_tab[2].xp_size = 0;
		usize = 0;
		printf("There will be no USR area on this drive #%d\n",drive);
		return (1);
	}
	usize = cyl * xh->xh_blkscyl;
	endcyl = totcyl + cyl - 1;
	xp->xp_tab[2].xp_boot = XP_NULL;
	xp->xp_tab[2].xp_s_h = 0;
	xp->xp_tab[2].xp_s_s = 1 | ((totcyl >> 2) & 0300);
	xp->xp_tab[2].xp_s_c = totcyl;
	xp->xp_tab[2].xp_sys = XP_USR;
	xp->xp_tab[2].xp_e_h = xh->xh_nhead - 1;
	xp->xp_tab[2].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp->xp_tab[2].xp_e_c = endcyl;
	xp->xp_tab[2].xp_start = (totcyl * xh->xh_blkscyl) + tmpoff;
	xp->xp_tab[2].xp_size = usize - tmpoff;
	usize -= tmpoff;
	totcyl += cyl;
	tmpoff = 0;
	return (0);
}

dodospart(xp,drive)
struct	xp *xp;
int	drive;
{
int	cyl, endcyl;
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	cyl = xh->xh_ncyls - totcyl;
	if (cyl <= 0)
	{
		xp->xp_tab[3].xp_boot = XP_NULL;
		xp->xp_tab[3].xp_s_h = 0;
		xp->xp_tab[3].xp_s_s = 0;
		xp->xp_tab[3].xp_s_c = 0;
		xp->xp_tab[3].xp_sys = XP_UNUSED;
		xp->xp_tab[3].xp_e_h = 0;
		xp->xp_tab[3].xp_e_s = 0;
		xp->xp_tab[3].xp_e_c = 0;
		xp->xp_tab[3].xp_start = 0;
		xp->xp_tab[3].xp_size = 0;
		dsize = 0;
		printf("There will be no DOS area on this drive #%d\n",drive);
		return (1);
	}
	dsize = cyl * xh->xh_blkscyl;
	endcyl = totcyl + cyl - 1;
	if (sysyes)
		xp->xp_tab[3].xp_boot = XP_NULL;
	else
		xp->xp_tab[3].xp_boot = XP_BOOT;
	xp->xp_tab[3].xp_s_h = 0;
	xp->xp_tab[3].xp_s_s = 1 | ((totcyl >> 2) & 0300);
	xp->xp_tab[3].xp_s_c = totcyl;
	if ((dsize - tmpoff) > doscutoff)
		xp->xp_tab[3].xp_sys = XP_DOS_4;
	else
		xp->xp_tab[3].xp_sys = XP_DOS;
	xp->xp_tab[3].xp_e_h = xh->xh_nhead - 1;
	xp->xp_tab[3].xp_e_s = xh->xh_sechd | ((endcyl >> 2) & 0300);
	xp->xp_tab[3].xp_e_c = endcyl;
	xp->xp_tab[3].xp_start = (totcyl * xh->xh_blkscyl) + tmpoff;
	xp->xp_tab[3].xp_size = dsize - tmpoff;
	dsize -= tmpoff;
	totcyl += cyl;
	tmpoff = 0;
	printf("DOS area will have %d cylinders\n",cyl);
	printf("Use DOS's FDISK and FORMAT commands to create the DOS area.\n");
	return (0);
}

partdump(xp,drive)
struct	xp *xp;
int	drive;
{
int	i, scyl, ecyl;
struct	xh *xh;

	xh = (struct xh *)&xp->xp_code[XH_START];
	printf("               Boot     Partition       Cyl    ");
	printf("Cyl     Cyl     Block     Block\n");
	printf("  Partition   Status       Type       Start    ");
	printf("End    Size      Size    Offset\n");
	printf("  ---------------------------------------------");
	printf("-------------------------------\n");
	for (i = 0; i < 4; i++ )
	{
		printf("      %d         %c     ",
			i + 1,
			(xp->xp_tab[i].xp_boot == XP_BOOT ? 'A' : 'N'));
		switch (xp->xp_tab[i].xp_sys)
		{
			case XP_DOS:
				printf("   DOS 2.x    ");
				break;

			case XP_DOS_4:
				printf("   DOS 3.x    ");
				break;

			case XP_SYS:
				printf(" VENIX system ");
				break;

			case XP_TMP:
				printf(" VENIX temp   ");
				break;

			case XP_USR:
				printf(" VENIX user   ");
				break;

			case XP_UNUSED:
				printf(" Unused  part ");
				break;

			default:
				printf(" Unknown part ");
		}
		scyl = xp->xp_tab[i].xp_s_c +
			((xp->xp_tab[i].xp_s_s & 0300) << 2);
		ecyl = xp->xp_tab[i].xp_e_c +
			((xp->xp_tab[i].xp_e_s & 0300) << 2);
		printf("  %5u ",scyl);
		printf(" %5u ",ecyl);
		if (xp->xp_tab[i].xp_size)
			printf("  %5u ",(ecyl - scyl + 1));
		else
			printf("  %5u ",0);
		printf(" %8u ",xp->xp_tab[i].xp_size);
		printf(" %8u\n",xp->xp_tab[i].xp_start);
	}
	return (0);
}
