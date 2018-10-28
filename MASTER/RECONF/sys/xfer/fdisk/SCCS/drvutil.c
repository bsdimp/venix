#include	"fdisk.h"

opendrive(drive)
int	drive;
{
int	fd;

	devname[6] = drive + '0';
	if ((fd = open(devname,2)) < 0)
	{
		printf("ERROR: %s: ",devname);
		perror("open block device");
		fatal("Error in accessing winchester.\n");
	}
	return (fd);
}

closedrive(fd)
int	fd;
{
	close(fd);
	sync();
	return (0);
}

struct	xp*
getblk0(fd)
int	fd;
{
struct	xp *xp;

	xp = (struct xp *)malloc(sizeof(struct xp));
	if (read(fd, xp, sizeof(struct xp)) != sizeof(struct xp))
	{
		printf("ERROR: %s: ",devname);
		perror("read block device");
		fatal("Cannot read disk block zero.\n");
	}
	return (xp);
}

putblk0(fd,xp)
int	fd;
struct	xp *xp;
{
	if (lseek(fd, 0L, 0) != 0)
	{
		printf("ERROR: %s: ",devname);
		perror("lseek block device");
		fatal("Cannot lseek to disk block zero.\n");
	}
	if (write(fd, xp, sizeof(struct xp)) != sizeof(struct xp))
	{
		printf("ERROR: %s: ",devname);
		perror("write block device");
		fatal("Cannot write disk block zero.\n");
	}
	sync();
	free(xp);
	return (0);
}

reread(drive)
int	drive;
{
int	fd;
extern	errno;

	rdevname[7] = drive + '0';
	if ((fd = open(rdevname,2)) < 0)
	{
		printf("ERROR: %s: ",rdevname);
		perror("open character device");
		fatal("Error in accessing winchester.\n");
	}
	errno = 0;
	ioctl(fd,I_REREAD,0);
	if (errno)
		perror("reread");
	close(fd);
	sync();
	return (0);
}
