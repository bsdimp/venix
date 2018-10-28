#define	MAIN_LINE	1

#include "fdisk.h"

/*
1
2
3              Boot     Partition       Cyl    Cyl     Cyl     Block     Block
4 Partition   Status       Type       Start    End    Size      Size    Offset
5 ----------------------------------------------------------------------------
6     1         x      xxxxx xxxxxx   xxxxx  xxxxx   xxxxx  xxxxxxxx  xxxxxxxx
7     2         x      xxxxx xxxxxx   xxxxx  xxxxx   xxxxx  xxxxxxxx  xxxxxxxx
8     3         x      xxxxx xxxxxx   xxxxx  xxxxx   xxxxx  xxxxxxxx  xxxxxxxx
9     4         x      xxxxx xxxxxx   xxxxx  xxxxx   xxxxx  xxxxxxxx  xxxxxxxx
0
*/

main(argc,argv)
int	argc;
char	*argv[];
{
struct	xp *xp, *getblk0();
int	fd, drive, doreread;

	if (argc > 1)
		drive = atoi(argv[1]);
	else
		drive = 0;
	if ((drive < 0) || (drive > 4))
		drive = 0;
	fd = opendrive(drive);
	xp = getblk0(fd);
	parminit(xp,drive);
	partinit(xp,drive);
	alldump(xp,drive);
repeat:
	while(!prompt("Are these parameters acceptable?",0,0))
	{
		cpos(1,10);
		clreop();
		parmnew(xp,drive);
		sleep(2);
		cpos(1,10);
		clreop();
		parmdump(xp,drive);
		cpos(1,25);
	}
	cpos(1,25);
	clreop();
	while(!prompt("Are these partitions acceptable?",0,0))
	{
		cpos(1,10);
		clreop();
		dosyspart(xp,drive);
		dotmppart(xp,drive);
		dousrpart(xp,drive);
		dodospart(xp,drive);
		sleep(2);
		alldump(xp,drive);
	}
	cpos(1,25);
	clreop();
	if (!prompt("Are you done with both parameters and partitions?",0,0))
	{
		cpos(1,25);
		clreop();
		goto repeat;
	}
	cpos(1,25);
	clreop();
	while (prompt("Do you want to change active boot partition?",0,0))
	{
		cpos(1,25);
		clreop();
		partact(xp,drive);
		sleep(2);
		alldump(xp,drive);
	}
	cpos(1,25);
	clreop();
	if (prompt("Are you sure that you want to update the disk drive?",0,0))
	{
		cpos(1,25);
		clreop();
		printf("Start updating disk drive %d",drive);
		putblk0(fd,xp);
		cpos(1,25);
		clreop();
		printf("Done updating disk drive %d\n",drive);
		doreread = 1;
	}
	else
	{
		cpos(1,25);
		clreop();
		printf("Disk drive %d was not updated\n",drive);
		doreread = 0;
	}
	closedrive(fd);
	if (doreread)
		reread(drive);
	exit (0);
}

alldump(xp,drive)
struct	xp *xp;
int	drive;
{
	chome();
	clreop();
	anorm();
	cpos(1,2);
	partdump(xp,drive);
	cpos(1,10);
	parmdump(xp,drive);
	cpos(1,25);
}
