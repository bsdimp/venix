#include <sys/param.h>
#include <sys/user.h>
#include <sys/conf.h>

#define	NMS	2			/* same as numb of serial ports	*/
#define	NCHR	128			/* number of graphic characters	*/
#define	BCHR	8			/* bytes per graphic character	*/

#define	USE_NO	0			/* serial port is not used	*/
#define	USE_MS	1			/* serial port used by mouse	*/
#define	USE_OT	2			/* serial port used by other	*/

#define	MS_FORCE	1		/* turn off port if used by anyone*/
#define	MS_RESET	2		/* turn off port if used by mouse */

int	use_port[NMS] = { 0, 0 };	/* usage of serial port		*/

char	gchars[NCHR][BCHR];		/* array of graphic characters	*/
char	gframe[BCHR] =
	{
		0xff,			/* 11111111			*/
		0x81,			/* 1      1			*/
		0x81,			/* 1      1			*/
		0x81,			/* 1      1			*/
		0x81,			/* 1      1			*/
		0x81,			/* 1      1			*/
		0x81,			/* 1      1			*/
		0xff			/* 11111111			*/
	};

struct	mcoord				/* mouse coordinates		*/
	{
		int	mode;		/* display mode			*/
		int	spx;		/* screen physical x coordinate	*/
		int	spy;		/* screen physical y coordinate	*/
		int	srx;		/* screen relative x coordinate	*/
		int	sry;		/* screen relative y coordinate	*/
		int	wpx;		/* window physical x coordinate	*/
		int	wpy;		/* window physical y coordinate	*/
		int	wrx;		/* window relative x coordinate	*/
		int	wry;		/* window relative y coordinate	*/
		char	sid;		/* screen id			*/
		char	wid;		/* window id			*/
	} mcoord;

msopen(dev)
{
int	i, j;
static	int msfirst = 0;

	i = minor(dev);
	u.u_error = 0;
	if (use_port[i] == USE_OT)
	{
		u.u_error = EACCES;
		return;
	}
	if (use_port[i] == USE_MS)
	{
		u.u_error = EBUSY;
		return;
	}
	use_port[i] = USE_MS;
	if (msfirst == 0)
	{
		for (i = 0; i < NCHR; i++)
			for (j = 0; j < BCHR; j++)
				gchars[i][j] = gframe[j];
		msfirst = 1;
	}
	return;
}

msclose(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	if (use_port[i] == USE_OT)
	{
		u.u_error = EACCES;
		return;
	}
	if (use_port[i] == USE_NO)
	{
		u.u_error = ENODEV;
		return;
	}
	use_port[i] = USE_NO;
	return;
}

msread(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	if (use_port[i] == USE_OT)
	{
		u.u_error = EACCES;
		return;
	}
	if (use_port[i] == USE_NO)
	{
		u.u_error = ENODEV;
		return;
	}
	return;
}

mswrite(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	if (use_port[i] == USE_OT)
	{
		u.u_error = EACCES;
		return;
	}
	if (use_port[i] == USE_NO)
	{
		u.u_error = ENODEV;
		return;
	}
	return;
}

msioctl(dev,cmd,arg)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	if (use_port[i] == USE_OT)
	{
		if (cmd == MS_FORCE)
			use_port[i] = USE_NO;
		else
			u.u_error = EACCES;
		return;
	}
	if (use_port[i] == USE_NO)
	{
		u.u_error = ENODEV;
		return;
	}
	switch (cmd)
	{
		case MS_RESET:
			use_port[i] = USE_NO;
			return;

		default:
			break;
	}
	return;
}
