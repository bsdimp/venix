#include "install.h"

finish()
{
	if (xt_ndef == 0)
	{
		if (nusr)
		{
			pf("\nSystem will be %d user.",nusr);
		}
		else
		{
		pf("\n%sWARNING:%s configuration error will cause this system",
				hlon,hloff);
			pf(" to be single-user only.");
		}
	}
	pf("\n\n%sInstallation complete.%s\n",hlon,hloff);
	if (prompt("\nDo you wish to execute the shell?",0))
	{
		sync();
		pf("\nThe shell will be executed.\n\n");
		signal(SIGHUP,SIG_DFL);
		signal(SIGINT,SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
		sgbuf.sg_flags = ECHO|CRT|CRMOD;
		ioctl(1,TIOCSETP,&sgbuf);
		execl("/bin/sh","sh","-i",0);
		fatal("Cannot execute the shell `/bin/sh'.\n");
	}
	sync();
	sync();
	sync();
	pf("\nRemove XFER floppy and reboot.\n");
	while (1);
}

prompt(string,high)
char	*string;
int	high;
{
	pf("%s%s%s (%sy%s or %sn%s) ",
		high ? hlon : "", string, high ? hloff : "",
		hlon,hloff,hlon,hloff);
	while (1)
	{
		switch (xgetchar())
		{
			case 'y':
			case 'Y':
				pf("%sYes%s\n",rvon,rvoff);
				return (1);

			case 'n':
			case 'N':
				pf("%sNo%s\n",rvon,rvoff);
				return (0);

			default:
				pf("\007");
		}
	}
}
	
char *
getstr(min,max,defstr)
int     min, max;
char    *defstr;
{
register char *cp;
char     c, cbuf[64];
int	 flags;

	ioctl(0,TIOCGETP,&sgbuf);
	flags = sgbuf.sg_flags;
	sgbuf.sg_flags = RAW;
	ioctl(0,TIOCSETP,&sgbuf);
	while (1)
	{
		pf("%s",rvon);
		for (cp = cbuf; cp < cbuf + max;)
		{
			read(0,&c,1);           
			if ((c == '\r' || c == '\n') && (cp > cbuf))
			{
				break;
			}
			else
			{
				if (c == DEL)
				{
					if (cp > cbuf)
					{
						pf("%s\b \b%s",rvoff,rvon);
						--cp;
					}
					else
					{
						pf("\007");
					}
				}
				else 
				{
					/* use the default */
					if ((c == '\r' || c == '\n') &&
						(cp == cbuf))
					{
						pf("%s\r\n",(cp = defstr));
						goto strout;
					}
					else    /* record character */
					{ 
						*cp++ = c;
						pf("%c",c);
					}
				}
			}
		}
		*cp = 0;
		cp = cbuf;
		pf("\r\n");
strout:
		pf("%s",rvoff);
		/*
		 * check that string length is bewteen bounds, 
		 */
		if (min <= strlen(cp) && strlen(cp) <= max)
		{
			sgbuf.sg_flags = flags;
			ioctl(0,TIOCSETP,&sgbuf);
			return (cp);
		}
		pf("\r\n%sInvalid string%s: please type again: ",
			hlon,hloff);
	}
}

checkrange(min,max,def)
int     *min, *max, *def;
{

	if (*min > *max)
		*min = *max;
	if (*def > *max)
		*def = *max;
	else
		if (*def < *min)
			*def = *min;
	return (0);
}

getnum(min,max,def)
int     min, max, def;
{
register char *cp;
char     c, cbuf[10];
int      r, flags;

	if (min > max)
		min = max;
	if (def > max)
		def = max;
	else
		if (def < min)
			def = min;
	ioctl(0,TIOCGETP,&sgbuf);
	flags = sgbuf.sg_flags;
	sgbuf.sg_flags = RAW;
	ioctl(0,TIOCSETP,&sgbuf);
	while (1)
	{
		pf("%s",rvon);
		for (cp = cbuf; cp < cbuf + 10;)
		{
			read(0,&c,1);           
			if      (isdigit(c) || 
				 (
				  (cp == cbuf) &&       /* first digit? */
				  (
				   (c == '+' && max >= 0) ||    /* '+' okay? */
				   (c == '-' && min < 0)        /* '-' okay? */
				  )
				 )
				)
			{ 
				*cp++ = c;
				pf("%c",c);
			}
			else
			{
				if ((c == '\r' || c == '\n') && (cp > cbuf))
				{
					pf("\r\n");
					break;
				}
				else
				{
					if (c == DEL)
					{
						if (cp > cbuf)
						{
							pf("%s\b \b%s",
								rvoff,rvon);
							--cp;
						}
						else
						{
							pf("\007");
						}
					}
					else 
					{
						/* use the default */
						if (    (c == '\r' ||
							c == '\n') &&
							(cp == cbuf)
						   )
						{
							pf("%d\r\n",(r = def));
							goto numout;
						}
						else
						{
							pf("\007"); /* beep */
						}
					}
				}
			}
		}
		*cp = 0;
		r = atoi(cbuf);
numout:
		pf("%s",rvoff);
		/*
		 * check number bewteen bounds, 
		 * allowing also for unsigned values
		 */
		if (min <= r && r <= max)
		{
			sgbuf.sg_flags = flags;
			ioctl(0,TIOCSETP,&sgbuf);
			return(r);
		}
		pf("\r\n%sInvalid number%s: please type again: ",
			hlon,hloff);
	}
}

run(name,v)
char    *name, *v;
{
register char **rv;
register int i;
int      stat;

	pf("%s\t",hlon);
	for (rv = &v; *rv != (char *)0; rv++)
		pf("%s ",*rv);
	pf("\n\t");
	if ((i = fork()) == 0)
	{
		execv(name, &v);
		fatal("BOOTABLE XFER floppy corrupted: cannot find %s\n",
			name);
	}
	while ((i != wait(&stat)));
	pf("%s",hloff);
	return (stat);
}

checkfs(name,fsize)
char     *name;
unsigned int fsize;
{
int     fd;
static  struct filsys fs;

	if ((fd = open(name, 0)) < 0)
	{
		pf("XFER: cannot open %s\n",name);
		return (1);
	}
	lseek( fd, 512L, 0);
	if (read(fd, &fs, sizeof(fs)) != sizeof(fs))
	{
		pf("XFER: Cannot read %s\n",name);
		close(fd);
		return (1);
	}
	close(fd);
	if ((fs.s_isize < (fsize / 3)) &&
		(fs.s_fsize  == fsize) &&
		(fs.s_nfree  <= 100) &&
		(fs.s_ninode <= 100))
			return (1);
	return (0);
}

fatal(f,s1,s2)
char    *f, s1, s2;
{
	pf("\n%sFATAL ERROR: %s",hlon,hloff);
	pf(f,s1,s2);
	pf("\nXFER halting.  Consult your manual for assistance\n");
	sync();
	pf("\nThe shell will be executed.\n\n");
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);
	sgbuf.sg_flags = ECHO|CRT|CRMOD;
	ioctl(1,TIOCSETP,&sgbuf);
	execl("/bin/sh","sh","-i",0);
	pf("\nCannot execute the shell `/bin/sh'.\n");
	sync();
	while (1);
}

xgetchar()
{
int     flags;
char    c;

	ioctl(0,TIOCGETP,&sgbuf);
	flags = sgbuf.sg_flags;
	sgbuf.sg_flags = RAW;
	ioctl(0,TIOCSETP,&sgbuf);
	read(0, &c, 1);
	sgbuf.sg_flags = flags;
	ioctl(0,TIOCSETP,&sgbuf);
	return (c == '\r' ? '\n' : c);
}

#ifdef  notdef
	/*
	 * This code belongs in dosysarea().
	 * Decide if 4 head or 2 head drive.
	 */
	{
		struct diskparm buf;

		if( (i = open("/dev/rw0.sys",0)) < 0 )
			fatal("Cannot open /dev/rw0.sys.\n");
		buf.d_nhead = 0;
		ioctl(i, I_GETDPP, &buf);
		close(i);
		if (buf.d_nhead == 2)
		{
		    if ((i = open("/uboot/xtboot.2hd",0)) < 0)
			fatal("Cannot find boot file (/uboot/xtboot.2hd).\n");
		}
		else
		{
		    if ((i = open("/uboot/xtboot",0)) < 0)
			fatal("Cannot find boot file (/uboot/xtboot).\n");
		}
	}
#endif  notdef
