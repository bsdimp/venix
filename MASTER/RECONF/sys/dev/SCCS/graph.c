#include <sys/param.h>
#include <sys/user.h>
#include <sys/conf.h>

gropen(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	return;
}

grclose(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	return;
}

grread(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	return;
}

grwrite(dev)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	return;
}

grioctl(dev,cmd,arg)
{
int	i;

	i = minor(dev);
	u.u_error = 0;
	switch (cmd)
	{
		case	GR_PLOT:
		case	GR_MOVE:
		case	GR_LINE:
		case	GR_BOX:
		case	GR_ARC:
		case	GR_CIRCLE:
		case	GR_FILL:
		case	GR_BYTE:
		case	GR_WORD:
		case	GR_RASTOP:
		default:
	}
	return;
}
