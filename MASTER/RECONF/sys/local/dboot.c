#include	<sys/xtblk0.h>

#define	BSIZE	512

extern	int	errno;

struct	xp	*xp;

char	*fname, ibuf[BSIZE];

int	fd;

main(argc,argv)
int	argc;
char	*argv[];
{
int	i, n, sect, cyl;

	if (argc > 1)
		fname = argv[1];
	else
		fname = "/dev/w0.phy";
	if ((fd = open(fname,0)) < 0)
	{
		perror("open");
		exit(printf("Error opening: %s  errno = %d\n",
			fname,errno));
	}
	if ((n = read(fd,ibuf,BSIZE)) != BSIZE)
	{
		perror("read");
		exit(printf("Error reading: %s  errno = %d: got %d not %d\n",
			fname,errno,n,BSIZE));
	}
	close(fd);
	xp = (struct xp *) &ibuf[0];
	for (i = 0; i < 4; i++)
	{
		printf("\nPARTITION # %d\n",i+1);
		printf("\tboot indicator :\t%8u\t0x%x\n",
			xp->xp_tab[i].xp_boot,
			xp->xp_tab[i].xp_boot);
		printf("\tstart head     :\t%8u\t0x%x\n",
			xp->xp_tab[i].xp_s_h,
			xp->xp_tab[i].xp_s_h);
		sect = xp->xp_tab[i].xp_s_s & 077;
		cyl = (xp->xp_tab[i].xp_s_s & 0300) << 2;
		cyl |= xp->xp_tab[i].xp_s_c & 0377;
		printf("\tstart sector   :\t%8u\t0x%x\n",sect,sect);
		printf("\tstart cylinder :\t%8u\t0x%x\n",cyl,cyl);
		printf("\tsystem type    :\t%8u\t0x%x\n",
			xp->xp_tab[i].xp_sys,
			xp->xp_tab[i].xp_sys);
		printf("\tend head       :\t%8u\t0x%x\n",
			xp->xp_tab[i].xp_e_h,
			xp->xp_tab[i].xp_e_h);
		sect = xp->xp_tab[i].xp_e_s & 077;
		cyl = (xp->xp_tab[i].xp_e_s & 0300) << 2;
		cyl |= xp->xp_tab[i].xp_e_c & 0377;
		printf("\tend sector     :\t%8u\t0x%x\n",sect,sect);
		printf("\tend cylinder   :\t%8u\t0x%x\n",cyl,cyl);
		printf("\tsector offset  :\t%8u\t0x%lx\n",
			xp->xp_tab[i].xp_start,
			xp->xp_tab[i].xp_start);
		printf("\tsector size    :\t%8u\t0x%lx\n",
			xp->xp_tab[i].xp_size,
			xp->xp_tab[i].xp_size);
	}
	printf("\n\tblock signature:\t%8u\t0x%x\n",
		xp->xp_sig, xp->xp_sig);
}
