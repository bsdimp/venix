/*
 * Display disk partitions and status of the winnie partition table.
 */

#include	<sys/xtblk0.h>

char	*device[4] =
	{
		"/dev/w0.phy",
		"/dev/w1.phy",
		"/dev/w2.phy",
		"/dev/w3.phy"
	};

main(argc,argv)
int	argc;
char	*argv[];
{
register int dev = 1, i, j, k, notlong = 1, scyl, ecyl, drive, drv;
struct	 xp xp;
struct	 xh *xh;

	drive = -1;
	if (argc > 1)
	{
		if (*argv[1] == '-')
		{
			argv[1]++;
			while (*argv[1])
			{
				switch (*argv[1])
				{
					case 'l':
						notlong = 0;
						break;

					case 'n':
						dev = atoi(argv[2]);
						if (dev < 1 || dev > 4)
							dev = 1;
						break;

					case '0':
					case '1':
					case '2':
					case '3':
						drive = atoi(argv[1]);
						if (drive < 0 || drive > 3)
							drive = -1;
						break;

					default :
						usage(argv[0]);
				}
				argv[1]++;
			}
		}
		else
		{
			drive = atoi(argv[1]);
			if (drive < 0 || drive > 3)
				drive = -1;
		}
	}
	if (drive >= 0)
	{
		drv = drive;
		dev = drive + 1;
	}
	else
		drv = 0;
	for (k = drv; k < dev; k++)
	{
		printf("\nDrive %d\n",k);
		if ((i = open(device[k],2)) < 0)
		{
			printf("'%s': cannot open partition table.\n",
				device[k]);
			continue;
		}
		if (read(i,&xp, 512) != 512)
		{
			printf("'%s': cannot read partition table.\n",
				device[k]);
			close(i);
			continue;
		}
		close(i);
		if (xp.xp_sig != XP_SIG)
		{
			printf("'%s': partition table has invalid signature.\n",
				device[k]);
			continue;
		}
		printf("               Boot     Partition       Cyl    ");
		printf("Cyl     Cyl     Block     Block\n");
		printf("  Partition   Status       Type       Start    ");
		printf("End    Size      Size    Offset\n");
		printf("  ---------------------------------------------");
		printf("-------------------------------\n");
		for (i = 0; i < 4; i++ )
		{
			printf("      %d         %c     ",
				i + 1,
				(xp.xp_tab[i].xp_boot == XP_BOOT ? 'A' : 'N'));
			switch (xp.xp_tab[i].xp_sys)
			{
				case XP_DOS:
					printf("   DOS 2.x    ");
					break;

				case XP_DOS_4:
					printf("   DOS 3.x    ");
					break;

				case XP_SYS:
					printf(" VENIX system ");
					break;

				case XP_TMP:
					printf(" VENIX temp   ");
					break;

				case XP_USR:
					printf(" VENIX user   ");
					break;

				case XP_UNUSED:
					printf(" Unused  part ");
					break;

				default:
					printf(" Unknown part ");
			}
			scyl = xp.xp_tab[i].xp_s_c +
				((xp.xp_tab[i].xp_s_s & 0300) << 2);
			ecyl = xp.xp_tab[i].xp_e_c +
				((xp.xp_tab[i].xp_e_s & 0300) << 2);
			printf("  %5u ",scyl);
			printf(" %5u ",ecyl);
			if (xp.xp_tab[i].xp_size)
				printf("  %5u ",(ecyl - scyl + 1));
			else
				printf("  %5u ",0);
			printf(" %8u ",xp.xp_tab[i].xp_size);
			printf(" %8u\n",xp.xp_tab[i].xp_start);
		}
		if (notlong)
			continue;
		printf("\n");
		xh = (struct xh *)&xp.xp_code[XH_START];
		if ((xh->xh_sig1 != XH_SIG1) || (xh->xh_sig2 != XH_SIG2))
		{
			printf("'%s': No header information present.\n",
				device[k]);
			continue;
		}
		printf("Drive Number         : %d\n",xh->xh_ndrv);
		printf("Drive Name           : %s\n",xh->xh_name);
		printf("Number of Partitions : %d\n",xh->xh_npart);
		printf("Number of Units      : %d\n",xh->xh_nunit);
		printf("Number of Heads      : %d\n",xh->xh_nhead);
		printf("Number of Cylinders  : %d\n",xh->xh_ncyls);
		printf("Sectors per Head     : %d\n",xh->xh_sechd);
		printf("Tracks per Drive     : %d\n",xh->xh_ntrack);
		printf("Blocks per Cylinder  : %d\n",xh->xh_blkscyl);
		printf("Block Size           : %d\n",xh->xh_blksize);
		printf("Step Speed Code      : %d\n",xh->xh_cntf);
		printf("Max Single transfer  : %d\n",xh->xh_maxfer);
	}
}

usage(name)
char	*name;
{
	printf("usage: %s [-l[n N]]\n",name);
	printf("       l - long list\n");
	printf("       n - take the N arg as number of drives to display\n");
	exit (1);
}
