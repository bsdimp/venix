/*
 *              VENIX/XT Installation Program   Nov 17, 1983
 *                 Version 2.0    6/14/84
 *                 Version 3.0    8/28/85
 *                 Version 4.0    10/1/85
 */

#define	MAIN_LINE	1

#include "install.h"

main()
{
	start();
	getserial();
	askshell();
	askdrives();
	doparmpart();
	if (xt_ndef <= 0)
	{
		dousrarea();
		dosysarea();
		putserial();
		doetcrc();
		putnusers();
		dokernel();
	}
	finish();
}

start()
{
int	i;

	if (open("/dev/console",2) < 0)         /* open standard error */
		for (;;);                       /* loop forever */
	dup(0);                                 /* std in */
	dup(0);                                 /* std out */
	for (i = 0; i < 16; ++i)
		signal(i,SIG_IGN);              /* ignore all signals */
	ioctl(1, TIOCGETP, &sgbuf);             /* turn off echo */
	sgbuf.sg_flags = CRMOD|CRT;
	ioctl(1, TIOCSETP, &sgbuf);
	return (0);
}

/*
 * find out number of users and system serial number,
 * stored in floppy boot block
 */
getserial()
{
int	fd;

	if ((fd = open("/dev/f0",0)) < 0)
		fatal("Can't open /dev/f0\n");
	lseek(fd, NUSR_BOOT, 0);
	read(fd, &nusr, sizeof(nusr));
	lseek(fd, SN_BOOT, 0);
	read(fd, &serno, sizeof(serno));
	close(fd);
	return (0);
}

askshell()
{
	pf("%sXFER/XT%s                Version 2.1\n\n",hlon,hloff);
	if (!prompt(
	"Do you wish to prepare the hard disk for VENIX/86 operation?",0))
	{
		pf("\nThe shell will be executed.\n\n");
		signal(SIGHUP,SIG_DFL);
		signal(SIGINT,SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
		sgbuf.sg_flags = ECHO|CRT|CRMOD;
		ioctl(1,TIOCSETP,&sgbuf);
		execl("/bin/sh","sh","-i",0);
		fatal("Cannot execute the shell `/bin/sh'.\n");
	}
	return (0);
}

askdrives()
{
	allsame = similar = 0;
	xt_ndef = -1;
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart I%s          Drive Parameters and Partitions\n\n",
		hlon,hloff);
	if (!prompt("Will more than one drive be initialized?",0))
	{
		pf("\nEnter drive number to be initialized (0 to 7) [0] : ");
		xt_ndef = getnum(0,7,0);
		xt_ndrv = xt_ndef + 1;
		return (0);
	}
	pf("\nNumber of disk drives to be initialized (0 to 8) [2] : ");
	if ((xt_ndrv = getnum(0,8,2)) > 1)
	{
		pf("\nWill all %d drives have same parameters?",xt_ndrv);
		if (allsame = prompt("",0))
			return (0);
		pf("\nWill the %d drives have similar parameters?",xt_ndrv);
		similar = prompt("",0);
	}
	return (0);
}
