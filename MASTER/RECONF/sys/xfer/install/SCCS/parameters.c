#include "install.h"

/*
 * PART I
 */
doparmpart()
{
int	fd;

	dssize = ssize;
	dswapsiz = swapsiz;
	for (drv = ((xt_ndef < 0) ? 0 : xt_ndef); drv < xt_ndrv; drv++)
	{
		fd = opendrive(drv);
		getblk0(fd);
		if (!drv || !allsame)
		{
			initheader();
			parmheading();
			dumpheader();
			while(!prompt("\nAre these parameters acceptable?",0))
			{
				newheader();
				sleep(5);
				parmheading();
				dumpheader();
			}
			initmins();
			partheading();
			dumppart();
			while(!prompt("\nAre these partitions acceptable?",0))
			{
				dosyspart();
				dotmppart();
				dousrpart();
				dodospart();
				sleep(5);
				partheading();
				dumppart();
			}
			if (!drv && !hssize)
				mustsys();
		}
		closedrive(fd);
		reread(drv);
		sleep(5);
	}
	if (xt_ndrv)
	{
		ssize   = hssize;
		swapsiz = hswapsiz;
		tsize   = htsize;
		usize   = husize;
		dsize   = hdsize;
	}
	return (0);
}

opendrive(drive)
int	drive;
{
int	fd;

	devname[6] = drive + '0';
	if ((fd = open(devname,2)) < 0)
	{
		perror("open block device");
		pf("ERROR: %s:\t",devname);
		fatal("Error in accessing winchester.\n");
	}
	return (fd);
}

closedrive(fd)
int	fd;
{
	write(fd, &xp, 512);
	sync();
	close(fd);
	sync();
	return (0);
}

reread(drive)
int	drive;
{
int	fd;
extern	errno;

	rdevname[7] = drive + '0';
	if ((fd = open(rdevname,2)) < 0)
	{
		perror("open raw device");
		pf("ERROR: %s:\t",rdevname);
		fatal("Error in accessing raw winchester.\n");
	}
	errno = 0;
	ioctl(fd, I_REREAD, (char *)0);
	if (errno)
		perror("ioctl reread");
	close(fd);
	sync();
	return (0);
}

getblk0(fd)
int	fd;
{
int	fd2;

	/*
		if first drive or (all drives not same and not similar)
	*/
	if (!drv || (!allsame && !similar))
	{
		if (read(fd, &xp, 512) != 512)
			fatal("Cannot read disk block zero.\n");
		if (xp.xp_sig != XP_SIG)
		{
			if ((fd2 = open("/uboot/xtblk0",0)) < 0)
			{
				pf("Cannot open partition file: ");
				fatal("/uboot/xtblk0\n");
			}
			if (read(fd2, &xp, 512) != 512)
			{
				pf("Cannot read partition file: ");
				fatal("/uboot/xtblk0\n");
			}
			close(fd2);
		}
		lseek(fd, 0L, 0);	/* rewind to block 0 */
	}
	return (0);
}

parmheading()
{
	pf("%s%s                VENIX/86 Installing Drive #%d%s\n\n",
		clear,hlon,drv,hloff);
	pf("%sPart I - A%s            Drive Parameters\n\n",hlon,hloff);
	return (0);
}

partheading()
{
	pf("%s%s                VENIX/86 Installing Drive #%d%s\n\n",
		clear,hlon,drv,hloff);
	pf("%sPart I - B%s            Drive Partitions\n\n",hlon,hloff);
	return (0);
}

initheader()
{
	/*
		The defaults are for 10MB full height
	*/
	xh = (struct xh *)&xp.xp_code[XH_START];
	xp.xp_sig = XP_SIG;
	if ((xh->xh_sig1 != XH_SIG1) || (xh->xh_sig2 != XH_SIG2))
	{
		xh->xh_sig1 = XH_SIG1;
		xh->xh_ndrv = drv;
		strcpy(xh->xh_name,"Winche");
		xh->xh_name[6] = drv + '0';
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
	xh->xh_ndrv = drv;
	xh->xh_name[6] = drv + '0';
	xh->xh_name[7] = 0;
	return (0);
}

dumpheader()
{
	pf("Drive Number         : %d %s*%s\n",xh->xh_ndrv,hlon,hloff);
	pf("Drive Name           : %s\n",xh->xh_name);
	pf("Number of Partitions : %d\n",xh->xh_npart);
	pf("Number of Units      : %d\n",xh->xh_nunit);
	pf("Number of Heads      : %d\n",xh->xh_nhead);
	pf("Number of Cylinders  : %d\n",xh->xh_ncyls);
	pf("Sectors per Head     : %d\n",xh->xh_sechd);
	pf("Tracks per Drive     : %d %s*%s\n",xh->xh_ntrack,hlon,hloff);
	pf("Blocks per Cylinder  : %d %s*%s\n",xh->xh_blkscyl,hlon,hloff);
	pf("Block Size           : %d %s*%s\n",xh->xh_blksize,hlon,hloff);
	pf("Step Speed Code      : %d\n",xh->xh_cntf);
	pf("Max Single transfer  : %d\n",xh->xh_maxfer);
	pf("\n%s* these values are generated automatically%s\n",hlon,hloff);
	return (0);
}

newheader()
{
	pf("\nDrive Number                    [%d] : %s%d%s\n",
		xh->xh_ndrv,rvon,xh->xh_ndrv,rvoff);
	pf("Drive Name                      [%s] : ",
		xh->xh_name);
	strcpy(xh->xh_name,getstr(1,7,xh->xh_name));
	pf("Number of Partitions (%d to %d) [%d] : ",
		0,4,xh->xh_npart);
	xh->xh_npart = getnum(0,4,xh->xh_npart);
	pf("Number of Units      (%d to %d) [%d] : ",
		0,8,xh->xh_nunit);
	xh->xh_nunit = getnum(0,8,xh->xh_nunit);
	pf("Number of Heads      (%d to %d) [%d] : ",
		0,8,xh->xh_nhead);
	xh->xh_nhead = getnum(0,8,xh->xh_nhead);
	pf("Number of Cylinders  (%d to %d) [%d] : ",
		0,999,xh->xh_ncyls);
	xh->xh_ncyls = getnum(0,999,xh->xh_ncyls);
	pf("Sectors per Head     (%d to %d) [%d] : ",
		0,99,xh->xh_sechd);
	xh->xh_sechd = getnum(0,99,xh->xh_sechd);
	pf("Tracks per Drive                [%d] : %s%d%s\n",
		xh->xh_ntrack,rvon,
		xh->xh_nhead * xh->xh_ncyls,rvoff);
	xh->xh_ntrack = xh->xh_nhead * xh->xh_ncyls;
	pf("Blocks per Cylinder             [%d] : %s%d%s\n",
		xh->xh_blkscyl,rvon,
		xh->xh_nhead * xh->xh_sechd,rvoff);
	xh->xh_blkscyl = xh->xh_nhead * xh->xh_sechd;
	pf("Block Size                      [%d] : %s%d%s\n",
		xh->xh_blksize,rvon,xh->xh_blksize,rvoff);
	pf("Step Speed Code      (%d to %d) [%d] : ",
		0,9,xh->xh_cntf);
	xh->xh_cntf = getnum(0,9,xh->xh_cntf);
	pf("Max Single transfer  (%d to %d) [%d] : ",
		0,maxfer,xh->xh_maxfer);
	xh->xh_maxfer = getnum(0,maxfer,xh->xh_maxfer);
	return (0);
}
