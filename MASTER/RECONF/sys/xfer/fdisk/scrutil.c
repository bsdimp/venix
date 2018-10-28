cpos(col,row)
int	col, row;
{
char	cmd[4];

	cmd[0] = '\033';
	cmd[1] = 'Y';
	cmd[2] = 040 + (row - 1);
	cmd[3] = 040 + (col - 1);
	write(1,cmd,4);
}

chome()
{
	putchar('\033');
	putchar('H');
}

cup()
{
	putchar('\033');
	putchar('A');
}

cdown()
{
	putchar('\033');
	putchar('B');
}

cright()
{
	putchar('\033');
	putchar('C');
}

cleft()
{
	putchar('\033');
	putchar('D');
}

rscroll()
{
	putchar('\033');
	putchar('I');
}

clreol()
{
	putchar('\033');
	putchar('K');
}

clreop()
{
	putchar('\033');
	putchar('J');
}

insline()
{
	putchar('\033');
	putchar('L');
}

delline()
{
	putchar('\033');
	putchar('M');
}

anorm()
{
	putchar('\033');
	putchar('\016');		/* ^N */
}

ablink(on)
int	on;
{
	putchar('\033');
	if (on)
		putchar('\002');	/* ^B */
	else
		putchar('\001');	/* ^A */
}

ahigh(on)
int	on;
{
	putchar('\033');
	if (on)
		putchar('\006');	/* ^F */
	else
		putchar('\005');	/* ^E */
}

arev(on)
int	on;
{
	putchar('\033');
	if (on)
		putchar('\008');	/* ^H */
	else
		putchar('\007');	/* ^G */
}

auline(on)
int	on;
{
	putchar('\033');
	if (on)
		putchar('\004');	/* ^D */
	else
		putchar('\003');	/* ^C */
}
