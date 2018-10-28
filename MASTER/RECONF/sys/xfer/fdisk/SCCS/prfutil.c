#include "fdisk.h"

prompt(string,high,ret)
char	*string;
int	high;
int	ret;
{
	printf("%s%s%s (%sy%s or %sn%s) ",
		high ? hlon : "", string, high ? hloff : "",
		hlon,hloff,hlon,hloff);
	while (1)
	{
		switch (xgetchar())
		{
			case 'y':
			case 'Y':
				printf("%sYes%s",rvon,rvoff);
				if (ret)
					printf("\n");
				return (1);

			case 'n':
			case 'N':
				printf("%sNo%s",rvon,rvoff);
				if (ret)
					printf("\n");
				return (0);

			default:
				printf("\007");
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
		printf("%s",rvon);
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
						printf("%s\b \b%s",rvoff,rvon);
						--cp;
					}
					else
					{
						printf("\007");
					}
				}
				else 
				{
					/* use the default */
					if ((c == '\r' || c == '\n') &&
						(cp == cbuf))
					{
						printf("%s\r\n",(cp = defstr));
						goto strout;
					}
					else    /* record character */
					{ 
						*cp++ = c;
						printf("%c",c);
					}
				}
			}
		}
		*cp = 0;
		cp = cbuf;
		printf("\r\n");
strout:
		printf("%s",rvoff);
		/*
		 * check that string length is bewteen bounds, 
		 */
		if (min <= strlen(cp) && strlen(cp) <= max)
		{
			sgbuf.sg_flags = flags;
			ioctl(0,TIOCSETP,&sgbuf);
			return (cp);
		}
		printf("\r\n%sInvalid string%s: please type again: ",
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
		printf("%s",rvon);
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
				printf("%c",c);
			}
			else
			{
				if ((c == '\r' || c == '\n') && (cp > cbuf))
				{
					printf("\r\n");
					break;
				}
				else
				{
					if (c == DEL)
					{
						if (cp > cbuf)
						{
							printf("%s\b \b%s",
								rvoff,rvon);
							--cp;
						}
						else
						{
							printf("\007");
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
							printf("%d\r\n",
								(r = def));
							goto numout;
						}
						else
						{
							printf("\007");
						}
					}
				}
			}
		}
		*cp = 0;
		r = atoi(cbuf);
numout:
		printf("%s",rvoff);
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
		printf("\r\n%sInvalid number%s: please type again: ",
			hlon,hloff);
	}
}

fatal(f,s1,s2)
char    *f, s1, s2;
{
	printf("\n%sFATAL ERROR: %s",hlon,hloff);
	printf(f,s1,s2);
	sync();
	exit (1);
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
