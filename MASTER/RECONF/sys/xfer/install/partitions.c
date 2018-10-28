#include "install.h"

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

mustsys()
{
int	cyl, blks, bsize;

	pf("\nMust redefine system area anyway!\n");
	bsize = xp.xp_tab[0].xp_size;
	pf("\nSize of system area in blocks is : %d\n",bsize);
	if (bsize < (dssize + tblsize + (bsize / 10)))
		fatal(
		"The system area is too small for both file system and swap.\n"
		);
	swapsiz = dswapsiz;
	blks = (bsize - dssize - tblsize) / swapint;
	blks *= swapint;
	checkrange(&swapsiz,&blks,&swapsiz);
	pf("\nNumber of blocks for swap (intervals of %d) (%d to %d) [%d] : ",
		swapint,swapsiz,blks,swapsiz);
	swapsiz = getnum(swapsiz,blks,swapsiz);
	hswapsiz = swapsiz = (swapsiz / swapint) * swapint;
	pf("%s\nUsing %d blocks at end of system area for swapping\n%s",
		hlon,swapsiz,hloff);
	hssize = ssize = bsize - tblsize - swapsiz;
	htsize = tsize = xp.xp_tab[1].xp_size;
	husize = usize = xp.xp_tab[2].xp_size;
	hdsize = dsize = xp.xp_tab[3].xp_size;
	sleep(5);
	return (0);
}

dosyspart()
{
int	cyl, blks, endcyl, bsize, maxbsize;

	sysmin = dsysmin;
	pf("\nSizes are in cylinders (%d blocks per cylinder)\n",
		xh->xh_blkscyl);
	checkrange(&sysmin,&sysmax,&sysmin);
redosys:
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
	maxbsize = (xh->xh_blkscyl * sysmax) - 1;
	bsize = (xh->xh_blkscyl * cyl) - 1;
	if (bsize < (dssize + tblsize + (bsize / 10)))
	{
	pf("\nThe system area is too small for both file system and swap.\n");
		if (bsize >= maxbsize)
			pf("But since this is the maximum possible ...\n");
		else
		{
			pf("Allocate more cylinders to system area.\n");
			goto redosys;
		}
	}
	blks = (bsize - dssize - tblsize) / swapint;
	blks *= swapint;
	checkrange(&swapsiz,&blks,&swapsiz);
	pf("\nNumber of blocks for swap (intervals of %d) (%d to %d) [%d] : ",
		swapint,swapsiz,blks,swapsiz);
	swapsiz = getnum(swapsiz,blks,swapsiz);
	swapsiz = (swapsiz / swapint) * swapint;
	pf("%s\nUsing %d blocks at end of system area for swapping\n%s",
		hlon,swapsiz,hloff);
	ssize = bsize - tblsize - swapsiz;
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
#ifdef	olddef
	if ((dsize - tmpoff) > doscutoff)
		xp.xp_tab[3].xp_sys = XP_DOS_4;
	else
		xp.xp_tab[3].xp_sys = XP_DOS;
#else	olddef
	if (dosversion == DOS16FAT)
		xp.xp_tab[3].xp_sys = XP_DOS_4;
	else
		xp.xp_tab[3].xp_sys = XP_DOS;
#endif	olddef
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

dumppart()
{
int	i, scyl, ecyl;

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
			(xp.xp_tab[i].xp_boot == XP_BOOT ? 'A' : 'N'));
		switch (xp.xp_tab[i].xp_sys)
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
		scyl = xp.xp_tab[i].xp_s_c +
			((xp.xp_tab[i].xp_s_s & 0300) << 2);
		ecyl = xp.xp_tab[i].xp_e_c +
			((xp.xp_tab[i].xp_e_s & 0300) << 2);
		printf("  %5u ",scyl);
		printf(" %5u ",ecyl);
		if (xp.xp_tab[i].xp_size)
			printf("  %5u ",(ecyl - scyl + 1));
		else
			printf("  %5u ",0);
		printf(" %8u ",xp.xp_tab[i].xp_size);
		printf(" %8u\n",xp.xp_tab[i].xp_start);
	}
	return (0);
}
