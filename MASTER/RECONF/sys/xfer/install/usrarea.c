#include "install.h"

/*
 * PART II
 */
dousrarea()
{
	if (!usize)
		return (1);
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart II%s                 Build User Area\n\n",hlon,hloff);
	if (!prompt("Do you wish to create the user area?",0))
		return (1);
	if (checkfs("/dev/w0.usr",usize))
	{
	pf("\n%sWARNING:%s There is a file system already on the user area\n",
			hlon,hloff);
		if (!prompt("Do you wish to continue?",0))
			return (1);
	}
	sprintf(buf,"%d",usize);
	if (prompt("\nDo you wish to check for bad blocks on the user area?",0))
	{
		pf("\nThis will take about 1 minute per 4000 blocks.\n\n");
		if (run("/bin/mkfs","mkfs","-b","/dev/w0.usr",buf,0))
			fatal("Error in accessing winchester (mkfs usr)\n");
	}
	else
	{
		pf("\nCreating a file system on the user area.\n");
		if (run("/bin/mkfs","mkfs","/dev/w0.usr",buf,0))
			fatal("Error in accessing winchester (mkfs usr)\n");
	}
	if (!prompt("\nDo you wish to load the user area from floppies?",0))
		return (1);
	pf("\nNumber of user floppies to be loaded (0 to 24) [7] : ");
	if ((usrflpy = getnum(0,24,7)) < 1)
		return (1);
	if (mount("/dev/w0.usr","/usr",0) < 0)
		fatal("Can't mount user area\n");
	chdir("/usr");
	sprintf(cntflpy,"xf%d",usrflpy);
	pf("\nTar the user area to the winchester from the floppy.\n");
	run("/bin/tar","tar",cntflpy,"/dev/rf0",0);
	chdir("/");
	umount("/dev/w0.usr");
	pf("\nUser area completed.");
	return (0);
}
