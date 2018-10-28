#include	<stdio.h>

main(argc,argv)
int	argc;
char	*argv[];
{
int	fd, n, o;
char	c;

	if (argc > 1)
		o = atoi(argv[1]);
	else
		o = 0x60;
	if ((fd = open("/dev/ioport",0)) < 0)
		exit(perror("open"));
	if ((n = lseek(fd, o, 0)) < 0)
		exit(perror("lseek"));
	if ((n = read(fd, &c, 1)) != 1)
		exit(perror("read"));
	printf("n = %d, c = 0x%x\n",n,c);
	close(fd);
}
