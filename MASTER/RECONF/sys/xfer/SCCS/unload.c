#include	<sys/devparm.h>

struct	diskparm dp;

main(argc,argv)
int	argc;
char	*argv[];
{
int	fd;
extern	errno;

	if (argc < 2)
		exit (printf("usage: %s /dev/rw?.???\n",argv[0]));
	if ((fd = open(argv[1],2)) < 0)
	{
		printf("%s: can't open\n",argv[1]);
		exit (perror("open"));
	}
	if (!strcmp(argv[0],"getdpp"))
	{
		if (ioctl(fd,I_GETDPP,&dp) >= 0)
		{
			if (dp.d_nblock < 0)
				printf("nblock = %d\n",dp.d_nblock);
			else
				printf("nblock = %u\n",dp.d_nblock);
			printf("offset = %u\n",dp.d_offset);
			printf("nsect  = %u\n",dp.d_nsect);
			printf("nhead  = %u\n",dp.d_nhead);
			printf("ntrack = %u\n",dp.d_ntrack);
		}
	}
	else
		if (!strcmp(argv[0],"load"))
			ioctl(fd,I_LOAD,0);
		else
			if (!strcmp(argv[0],"unload"))
				ioctl(fd,I_UNLOAD,0);
			else
				if (!strcmp(argv[0],"ddump"))
					ioctl(fd,I_DUMP,0);
				else
					if (!strcmp(argv[0],"reread"))
						ioctl(fd,I_REREAD,0);
					else
						printf("%s: invalid command\n",
							argv[0]);
	if (errno)
		perror("ioctl");
	close(fd);
}
