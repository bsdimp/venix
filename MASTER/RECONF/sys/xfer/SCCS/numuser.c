#include <a.out.h>
#include <ctype.h>

#define NUSR_INIT	4L	/* offset to #users variable in /etc/init */

struct	exec header;

char	clear[] = "\033H\033J\033\016";	/* clear and reset screen */
char	hlon[] = "\033\006";		/* highlight */
char	hloff[] = "\033\005";
char	rvon[] = "\033\010";		/* reverse video */
char	rvoff[] = "\033\007";

int	nusr, i, fd;
long	lseek();

main(argc,argv)
int	argc;
char	*argv[];
{
	if (argc > 1)
		nusr = atoi(argv[1]);
	else
		exit(printf("%s%s: valid # of users range is 1 thru 16%s\n",
			hlon,argv[0],hloff));
	if ((nusr < 1) || (nusr > 16))
		exit(printf("%s%s: valid # of users range is 1 thru 16%s\n",
			hlon,argv[0],hloff));

	if ((fd = open("/etc/init",2)) < 0)
		exit(printf("%s%s: cannot open /etc/init%s\n",
			hlon,argv[0],hloff));

	/* NUSR_INIT tells us positition of
	 * symbol determining # of users,
	 * offset from end of code area 
	 */

	read(fd, &header, sizeof(struct exec));
	lseek(fd,header.a_text + NUSR_INIT + sizeof(struct exec),0);
	read(fd, &i, sizeof(i));
	if ((i < 1) || (i > 16))	/* check we're in right spot */
		exit(printf("%s%s: read invalid number of users = %d%s\n",
			hlon,argv[0],i,hloff));
	lseek(fd,header.a_text + NUSR_INIT + sizeof(header),0);
	write(fd, &nusr, sizeof(nusr));
	close(fd);
	printf("%s%s: number of users changed: old = %d  new = %d%s\n",
		hlon,argv[0],i,nusr,hloff);
}
