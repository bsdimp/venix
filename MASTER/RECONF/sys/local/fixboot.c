#include	<stdio.h>
#include	<sys/xtblk0.h>

#define	BSIZE	512

extern	int	errno;

struct	xp	*ixp, *oxp;

char	*ifname, *ofname, dfname[20], *rindex(), *bcopy();
char	ibuf[BSIZE], obuf[BSIZE];

FILE	*dfin;

long	start, size;

int	ifd, ofd, i, n;
int	p, boot, head, sect, cyl, sys;

main(argc,argv)
int	argc;
char	*argv[];
{
	init(argc,argv);
	readtable();
	dumptable(ixp);
	newtable(argv);
	dumptable(oxp);
	ask();
	writetable();
}

init(argc,argv)
int	argc;
char	*argv[];
{
	setbuf(stdin,(char *)NULL);
	setbuf(stdout,(char *)NULL);
	if (argc > 1)
		ifname = argv[1];
	else
		ifname = "/dev/w0.phy";
	if (*ifname == '-')
		ifname = "/dev/w0.phy";
	if (argc > 2)
		ofname = argv[2];
	else
		ofname = "/dev/w0.phy";
	if (*ofname == '-')
		ofname = "/dev/w0.phy";
}

readtable()
{
	if ((ifd = open(ifname,0)) < 0)
		exit(printf("Error opening: %s  errno = %d\n",
			ifname,errno));
	if ((n = read(ifd,ibuf,BSIZE)) != BSIZE)
		exit(printf("Error reading: %s  errno = %d: read %d not %d\n",
			ifname,errno,n,BSIZE));
	close(ifd);
	ixp = (struct xp *) &ibuf[0];
}

writetable()
{
	if ((ofd = open(ofname,1)) < 0)
		if ((ofd = creat(ofname,644)) < 0)
			exit(printf("Error creating: %s  errno = %d\n",
				ofname,errno));
	if ((n = write(ofd,obuf,BSIZE)) != BSIZE)
		exit(printf("Error writing: %s  errno = %d: wrote %d not %d\n",
			ofname,errno,n,BSIZE));
	close(ofd);
}

ask()
{
char	answer[10];

	fprintf(stderr,
	"Are you sure that you want to update the partition table? ");
	fscanf(stdin,"%s",answer);
	if ((answer[0] != 'y') && (answer[0] != 'Y'))
		exit(fprintf(stderr,"No update done! Bye!\n"));
}

dumptable(xp)
struct	xp *xp;
{
	for (i = 0; i < 4; i++)
	{
		printf("\nPARTITION # %d\n",i+1);
		printf("\tboot indicator :\t%d\t0x%x\n",
			xp->xp_tab[i].xp_boot,
			xp->xp_tab[i].xp_boot);
		printf("\tstart head     :\t%d\t0x%x\n",
			xp->xp_tab[i].xp_s_h,
			xp->xp_tab[i].xp_s_h);
		sect = xp->xp_tab[i].xp_s_s & 077;
		cyl = (xp->xp_tab[i].xp_s_s & 0300) << 2;
		cyl |= xp->xp_tab[i].xp_s_c & 0377;
		printf("\tstart sector   :\t%d\t0x%x\n",sect,sect);
		printf("\tstart cylinder :\t%d\t0x%x\n",cyl,cyl);
		printf("\tsystem type    :\t%d\t0x%x\n",
			xp->xp_tab[i].xp_sys,
			xp->xp_tab[i].xp_sys);
		printf("\tend head       :\t%d\t0x%x\n",
			xp->xp_tab[i].xp_e_h,
			xp->xp_tab[i].xp_e_h);
		sect = xp->xp_tab[i].xp_e_s & 077;
		cyl = (xp->xp_tab[i].xp_e_s & 0300) << 2;
		cyl |= xp->xp_tab[i].xp_e_c & 0377;
		printf("\tend sector     :\t%d\t0x%x\n",sect,sect);
		printf("\tend cylinder   :\t%d\t0x%x\n",cyl,cyl);
		printf("\tsector offset  :\t%D\t0x%X\n",
			xp->xp_tab[i].xp_start,
			xp->xp_tab[i].xp_start);
		printf("\tsector size    :\t%D\t0x%X\n",
			xp->xp_tab[i].xp_size,
			xp->xp_tab[i].xp_size);
	}
	printf("\n\tblock signature:\t%d\t0x%x\n",
		xp->xp_sig, xp->xp_sig);
}

newtable(argv)
char	*argv[];
{
	bcopy(obuf,ibuf,BSIZE);
	oxp = (struct xp *) &obuf[0];
	if (rindex(argv[0],'/'))
		strcpy(dfname,(rindex(argv[0],'/') + 1));
	else
		strcpy(dfname,argv[0]);
	strcat(dfname,".dat");
	printf("\nReading: %s\n",dfname);
	if ((dfin = fopen(dfname,"r")) == NULL)
		exit(printf("Error fopening: %s  errno = %d\n",
			dfname,errno));
	for (i = 0; i < 4; i++)
	{
		if ((n = fscanf(dfin,"%d",&p)) == EOF)
			exit(printf("Premature EOF on %s: header %d\n",
				dfname,i));
		if (p != i)
			exit(printf("Missing header i=%d p=%d n=%d\n",
				i,p,n));
		if (fscanf(dfin,"%x",&boot) == EOF)
			exit(printf("Premature EOF on %s: boot %d\n",
				dfname,i));
		if (fscanf(dfin,"%d",&head) == EOF)
			exit(printf("Premature EOF on %s: start head %d\n",
				dfname,i));
		if (fscanf(dfin,"%d",&sect) == EOF)
			exit(printf("Premature EOF on %s: start sector %d\n",
				dfname,i));
		if (fscanf(dfin,"%d",&cyl) == EOF)
			exit(printf("Premature EOF on %s: start cylinder %d\n",
				dfname,i));
		oxp->xp_tab[i].xp_boot = boot & 0377;
		oxp->xp_tab[i].xp_s_h = head & 0377;
		oxp->xp_tab[i].xp_s_c = cyl & 0377;
		sect &= 077;
		cyl &= 0x0300;
		sect |= cyl / 4;
		oxp->xp_tab[i].xp_s_s = sect;
		if (fscanf(dfin,"%x",&sys) == EOF)
			exit(printf("Premature EOF on %s: sys %d\n",
				dfname,i));
		if (fscanf(dfin,"%d",&head) == EOF)
			exit(printf("Premature EOF on %s: end head %d\n",
				dfname,i));
		if (fscanf(dfin,"%d",&sect) == EOF)
			exit(printf("Premature EOF on %s: end sector %d\n",
				dfname,i));
		if (fscanf(dfin,"%d",&cyl) == EOF)
			exit(printf("Premature EOF on %s: end cylinder %d\n",
				dfname,i));
		oxp->xp_tab[i].xp_sys = sys & 0377;
		oxp->xp_tab[i].xp_e_h = head & 0377;
		oxp->xp_tab[i].xp_e_c = cyl & 0377;
		sect &= 077;
		cyl &= 0x0300;
		sect |= cyl / 4;
		oxp->xp_tab[i].xp_e_s = sect;
		if (fscanf(dfin,"%D",&start) == EOF)
			exit(printf("Premature EOF on %s: starting sector %d\n",
				dfname,i));
		if (fscanf(dfin,"%D",&size) == EOF)
			exit(printf("Premature EOF on %s: size in sectors %d\n",
				dfname,i));
		oxp->xp_tab[i].xp_start = start;
		oxp->xp_tab[i].xp_size = size;
	}
	fclose(dfin);
}

char *
bcopy(s1,s2,n)
char	*s1, *s2;
int	n;
{
char	*p1 = s1, *p2 = s2;
int	i;

	for (i = 0; i < n; i++)
		*p1++ = *p2++;
	return (s1);
}
