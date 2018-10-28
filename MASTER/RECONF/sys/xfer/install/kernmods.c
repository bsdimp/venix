#include "install.h"

/*
 * Insert serial number
 */
putserial()
{
int	fd;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	if ((fd = open("/dev/w0.sys",2)) < 0)
		fatal("Cannot open /dev/w0.sys\n");
	lseek(fd, SN_INIT, 0);
	write(fd, &serno, sizeof(serno));
	close(fd);
	return (0);
}

/*
 * Patch /etc/rc with tmp file system size.
 */
doetcrc()
{
int	i, k, fd;
char	*cp;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	if (mount("/dev/w0.sys","/usr",0) < 0)
		fatal("Cannot mount /dev/w0.sys\n");
	if ((fd = open("/usr/etc/rc",2)) < 0)
		fatal("Cannot open file /etc/rc\n");
	if ((i = read(fd, buf, 512)) > 0)
	{
		lseek(fd, 0L, 0);
		for (cp = buf; cp < &buf[i]; cp++)
		{
			if (strncmp(cp,"/etc/mkfs /dev/w0.tmp ",22) == 0)
			{
				if ((k = cp - buf) > i)
					break;
				write(fd, buf, k);
				write(fd, "/bin/rm -f /tmp/*", 17);
				while (*cp++ != '\n' && cp < &buf[i]);
				cp--;
				write(fd, cp, &buf[i]-cp);
				break;
			}
		}
	}
	close(fd);
	return (0);
}

/*
 * patch /etc/init with # of users
 */
putnusers()
{
int	i, fd;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	pf("\nNumber of users this system will have (1 to 16) [%d] : ",nusr);
	nusr = getnum(1,16,nusr);
	if ((fd = open("/usr/etc/init",2)) < 0)
		fatal("Cannot open /usr/etc/init\n"); 
	/* NUSR_INIT tells us positition of
	 * symbol determining # of users,
	 * offset from end of code area 
	 */
	read(fd, &header, sizeof(struct exec));
	lseek(fd, header.a_text+NUSR_INIT+sizeof(struct exec), 0);
	read(fd, &i, sizeof(i));
	if ((i < 1) || (i > 16)) /* check we're in right spot */
	{
		nusr = 0;
	}
	else
	{
		lseek(fd, header.a_text+NUSR_INIT+sizeof(header), 0);
		write(fd, &nusr, sizeof(nusr));
	}
	close(fd);
	return (0);
}

/*
 * Copy the venix kernel from floppy to winchester.
 */
dokernel()
{
int	a, i, n, fd, fd2;

	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
	if ((fd2 = open("/venix",0)) < 0)
		fatal("Cannot open /venix\n");
	close(creat("/usr/venix",0444));
	if ((fd = open("/usr/venix",2)) < 0)
		fatal("Cannot create `venix' on the winchester\n");
	while ((i = read(fd2, buf, 512)) > 0)
		write(fd, buf, i);
	close(fd2);
	lseek(fd, 0L, 0);
	if (read(fd, &header, sizeof(struct exec)) != sizeof(struct exec) ||
		N_BADMAG(header))
		fatal("venix header is bad\n");
	/*
	 * Read symbol table from /venix so we can
	 * patch device numbers and timezone info
	 * and number of drives
	 */
	nlist("/usr/venix",symbols);
	for (pnl = symbols; *(pnl->n_name); ++pnl)
	{
		if (pnl->n_type == 0)
			fatal("Bad venix namelist\n");
	}
	for (i = 0; i < 7;)
	{
		symbols[i++].n_value += (long)sizeof(struct exec)
			+ (header.a_magic!=0407 ? header.a_text : 0);
	}
	/*
	 * change device numbers
	 */
	i = 0x0100;
	lseek(fd, (long)symbols[3].n_value, 0);
	write(fd, &i, sizeof(i));
	lseek(fd, (long)symbols[4].n_value, 0);
	write(fd, &i, sizeof(i));
	lseek(fd, (long)symbols[5].n_value, 0);
	i = 0x0101;
	write(fd, &i, sizeof(i));
	n = dotimezone();
	a = prompt("\nDoes daylight saving time ever apply here?",0);
	lseek(fd, (long)symbols[0].n_value, 0);
	write(fd, &n, sizeof(n));
	lseek(fd, (long)symbols[1].n_value, 0);
	write(fd, &a, sizeof(a));
	if (xt_ndrv)
	{
		lseek(fd, (long)symbols[6].n_value, 0);
		write(fd, &xt_ndrv, sizeof(xt_ndrv));
	}
	close(fd);
	sync();
	sync();
	sync();
	return (0);
}

/*
 * PART IV
 */
dotimezone()
{
int	n;


	if (xt_ndef > 0)
		return (1);
	if (!ssize)
		return (1);
zoneloop:
	pf("%s%s                VENIX/86 Installation Procedure%s\n\n",
		clear,hlon,hloff);
	pf("%sPart IV%s               Time Zone Selection\n\n",hlon,hloff);
	pf("    1 - Eastern   time zone (EST)\n");
	pf("    2 - Pacific   time zone (PST)\n");
	pf("    3 - Mountain  time zone (MST)\n");
	pf("    4 - Central   time zone (CST)\n");
	pf("    5 - Atlantic  time zone (AST)\n");
	pf("    6 - Japan     time zone (JST)\n");
	pf("    7 - Greenwich mean time zone (GMT)\n");
	pf("    8 - Other\n\n");
	pf("Enter a number corresponding to your time zone (1 to 8) [2] : ");
	n = getnum(1,8,2);
	if (n-- < 8)
	{
		pf("You have selected the %s zone. Are you sure?",tz[n].msg);
		if (!prompt("",0))
			goto zoneloop;
		n = tz[n].min;
	}
	else
	{
pf("Enter the minutes west of GMT (minus for east) (-720 to 720) [0] : ");
		n = getnum(-720,720,0);
	}
	return (n);
}
