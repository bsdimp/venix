#include "install.h"

/*
 * PART III
 */
dosysarea()
{
int	i, fd, fd2;

	if (!ssize)
		return (1);
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart III%s               Build System Area\n\n",hlon,hloff);
	if (!prompt("Do you wish to create the system area?",0))
		return (1);
	pf("\nCheck the swapping area for bad blocks.\n\n");
	if ((fd = open("/dev/w0.sys",2)) < 0)
		fatal("Error in accessing winchester\n");
	lseek(fd, ssize * 512L, 0);
	for (i = 0; i < swapsiz; i++)
	{
		if (read(fd, buf, 512) != 512)
		{
			pf("%sWARNING%s: bad block(s)\n",hlon,hloff);
			break;
		}
	}
	pf("%s%d blocks for swapping%s\n",hlon,i,hloff);
	if (tsize)
	{
		sprintf(buf,"%d",tsize);
		pf("\nCheck the temporary area for bad blocks.\n");
		pf("\nThis will take about 1 minute per 4000 blocks.\n\n");
		if (run("/bin/mkfs","mkfs","-b","/dev/w0.tmp",buf,0))
			fatal("Error in accessing winchester (mkfs tmp)\n");
	}
	if ((fd2 = open("/uboot/xtboot",0)) < 0)
		fatal("Cannot open boot file: /uboot/xtboot\n");
	if (read(fd2, buf, 512) != 512)
		fatal("Cannot read boot file: /uboot/xtboot\n");
	close(fd2);
	lseek(fd, 0L, 0);
	write(fd, buf, 512);
	close(fd);
	if (checkfs("/dev/w0.sys",ssize))
	{
	pf("\n%sWARNING:%s There is a file system already on the system area\n",
			hlon,hloff);
		if (!prompt("Do you wish to continue?",0))
			return (1);
	}
	sprintf(buf,"%d",ssize);
	pf("\nCheck the system area for bad blocks.\n");
	pf("\nThis will take about 1 minute per 4000 blocks.\n\n");
	if (run("/bin/mkfs","mkfs","-b","/dev/w0.sys",buf,0))
		fatal("Error in accessing winchester (mkfs sys)\n");
	if (!prompt("\nDo you wish to load the system area from floppies?",0))
		return (1);
	pf("\nNumber of system floppies to be loaded (0 to 24) [4] : ");
	if ((sysflpy = getnum(0,24,4)) < 1)
		return (1);
	if (mount("/dev/w0.sys", "/usr", 0) < 0)
		fatal("Can't mount system area\n");
	chdir("/usr");
	sprintf(cntflpy,"xf%d-",sysflpy);
	pf("\nTar the system area to the winchester from the floppy.\n");
	run("/bin/tar","tar",cntflpy,"/dev/rf0",".", 0);
	chdir("/");
	umount("/dev/w0.sys");
	pf("\nSystem area completed.");
	return (0);
}
