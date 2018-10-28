/*
 * npipe.c	Version 1.x and 2.x pseudo named pipe driver.
 *			May 29, 1984.
 */

/*
 * Basic strategy is to provide a device driver interface between
 * special files and the internal read/write subroutines.  This provides
 * a named pipe capability with the exception that channels are
 * differenitiated on a minor device number basis rather then inode.
 * Only the O_NDELAY option is implementated here.
 */
#include	<sys/param.h>
#include	<sys/conf.h>
#include	<sys/user.h>
#include	<sys/file.h>
#include	<sys/inode.h>

#define	NCHAN		32
#define	O_NDELAY	04
#define	F_GETFL		1
#define	F_SETFL		2

char	npmode[NCHAN];
int	npoff[NCHAN];
struct	inode *npinode[NCHAN];

npopen(dev,mode)
{
register struct inode *ip;
register int i;
extern	 int pipedev;

	if ((i = dev.d_minor) >= NCHAN)
	{
		u.u_error = ENXIO;
		return;
	}
	printf("NPOPEN(0x%x,0x%x) npinode = 0x%x\n",dev,mode,npinode[i]);
	if (npinode[i] == NULL)
	{
		if ((ip = ialloc(pipedev)) == NULL)
			return;
		npoff[i] = 0;
		npinode[i] = ip;
		ip->i_flag = IACC | IUPD | IPIPE;
		ip->i_mode = IALLOC;
		ip->i_count = 1;
	}
	if (mode & 01)
		npmode[i] = mode;
}

npclose(dev)
{
register int i;

	i = dev.d_minor;
	printf("NPCLOSE(0x%x)\n",dev);
	iput(npinode[i]);
	npinode[i] = NULL;
}

npioctl(dev,cmd,addr)
char	*addr;
{
register int i;

	printf("NPIOCTL(0x%x,0x%x,0x%x)\n",dev,cmd,addr);
	i = dev.d_minor;
	if (cmd == F_GETFL)
		subyte(addr,npmode[i]);
	else
		if (cmd == F_SETFL)
			npmode[i] = fubyte(addr);
}

/*
 * This code is pretty much ripped out of `pipe.c'.
 */
#define	PIPSIZ	4096

npwrite(dev)
{
register struct inode *ip;
register int c;

	ip = npinode[dev.d_minor];
	c = u.u_count;
	printf("NPWRITE(0x%x), ucount = %d, isize = %d\n",dev,c,ip->i_size1);
loop:
	plock(ip);
	if (c == 0)
	{
		prele(ip);
		u.u_count = 0;
		return;
	}
	if (ip->i_size1 >= PIPSIZ)
	{
		ip->i_mode |= IWRITE;
		prele(ip);
		sleep((int)ip+1,PPIPE);
		goto loop;
	}
	u.u_offset[0] = 0;
	u.u_offset[1] = ip->i_size1;
	u.u_count = (c < PIPSIZ) ? c : PIPSIZ;
	c -= u.u_count;
	writei(ip);
	prele(ip);
	if (ip->i_mode & IREAD)
	{
		ip->i_mode &= ~IREAD;
		wakeup((int)ip+2);
	}
	goto loop;
}

npread(dev)
{
register struct inode *ip;
register int i;

	i = dev.d_minor;
	ip = npinode[i];
	if (((npmode[i] & O_NDELAY) != 0) &&
		((ip->i_size1 - npoff[i]) < u.u_count))
		return;
	printf("NPREAD(0x%x) offset = %d, inode = 0x%x\n",dev,npoff[i],ip);
loop:
	plock(ip);
	if (ip->i_size1 == 0)
	{
		prele(ip);
		ip->i_mode |= IREAD;
		sleep((int)ip+2,PPIPE);
		goto loop;
	}
	u.u_offset[0] = 0;
	u.u_offset[1] = npoff[i];
	readi(ip);
	npoff[i] = u.u_offset[1];
	if (npoff[i] == ip->i_size1)
	{
		npoff[i] = 0;
		ip->i_size1 = 0;
		if (ip->i_mode & IWRITE)
		{
			ip->i_mode &= ~IWRITE;
			wakeup((int)ip+1);
		}
	}
	prele(ip);
}
