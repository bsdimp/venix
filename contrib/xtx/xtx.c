#ifndef lint
static char sccsid[] = "@(#)xtx.c	1.1 (UKC) 13/10/85";
#endif  lint
/***

* program name:
	xtx.c
* function:
	Looks in a file for a string of the form
		<-xtx[letter]->
	If the string is followed by a line ending with a \n
	Then takes the line and executes it using /bin/sh
	The letter is used to select one of a number of commands
	which may be embedded in the file. The character '*' is
	used to mean the default.
* switches:
	-c [letter]	select a particular line in the file
	-e		echo the matched command from the file
	-a		print all matched commands from the file and their
			key letter
	+[letter]	same as -c [letter]
	files
	-		Take data from standard input for scanning
* libraries used:
	standard
* compile time parameters:
<-xtx-*>	cc -o xtx -O xtx.c
<-xtx-l>	lint -h xtx.c
* history:
	Written October 1985 Peter Collinson UKC
***/
#include <stdio.h>
#include <ctype.h>

char	cmdbuf[BUFSIZ];	/* Where to store the command */

char	selchar='*';	/* selection character */
int	echosw;		/* set if echo only */
int	allsw;		/* print all matched commands from the file */

#define WILD	1	/* Wild character in the string below */
char	matchs[] =	/* Used to find a match with a command */
{	'<','-','x','t','x','-',WILD,'>'	};
#define STORESTATE	(sizeof matchs)

char Usage[] = "Usage:txt [-e][-c letter|+letter] files..\n";

main(argc, argv)
char **argv;
{	register char *p;
	FILE *fd;

	while (--argc)
	{	p = *++argv;
		switch (*p)
		{
		case '-':
			p++;
			switch (*p)
			{
			case 'a':
				allsw++;
				echosw++;
				break;
			case 'c':
				if (--argc)
				{	argv++;
					selchar = **argv;
				}
				else
				fatal("No argument to -c given\n");
				break;
			case 'e':
				echosw++;
				break;
			case '\0':
				scanfile(stdin);
				if (cmdbuf[0])
					execute(cmdbuf);
				break;
			default:
				fatal(Usage);
			}
			break;
		case '+':
			selchar = p[1];
			break;
		default:
			if ((fd = fopen(argv[0], "r")) == NULL)
			{	perror(argv[0]);
				break;
			}
			scanfile(fd);
			(void) fclose(fd);
			if (cmdbuf[0])
				execute(cmdbuf);
			break;
		}
				
	}
	exit(1);	/* Command has failed */
}

/*
 *	Scan a file looking for an appropriate line
 */
scanfile(fd)
FILE *fd;
{	register c;
	register state = 0;
	register char *cmdp;

	while ((c = getc(fd)) != EOF)
	{	if (state == STORESTATE)
		{	if (isprint(c) || isspace(c))
			{	if (c == '\n')
				{	*cmdp = '\0';
					if (allsw)
					{	execute(cmdbuf);
						state = 0;
						continue;
					}
					return;
				}
				else
				if (cmdp < &cmdbuf[BUFSIZ])
					*cmdp++ = c;
				continue;
			}
			state = 0;
		}
		else
		if (matchs[state] == WILD)
		{	if (allsw)
				selchar = c;
			if (c == selchar)
				state++;
			else	state = 0;
		}
		else
		if (c == matchs[state])
		{	if (state++ == 0)
			{	cmdp = cmdbuf; *cmdp = '\0';	}
		}
		else	state = 0;
	}
}

/*
 *	Call the shell
 */
execute(cmdp)
register char *cmdp;
{
	for (; isspace(*cmdp); cmdp++);
	if (allsw)
		fprintf(stderr, "%c\t%s\n", selchar, cmdp);
	else
		fprintf(stderr, "%s\n", cmdp);
	if (allsw)
		return;
	if (echosw)
		exit(0);
	execl("/bin/sh", "sh", "-c", cmdp, 0);
	perror("Exec");
	exit(1);
}

fatal(str)
char *str;
{	fprintf(stderr, str);
	exit(1);
}
