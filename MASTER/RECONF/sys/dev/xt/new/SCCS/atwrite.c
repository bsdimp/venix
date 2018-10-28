atwrite(dev)
{
register int drive, unit;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= at_ndrv) || (unit >= NUNIT))
	{
		u.u_error = ENXIO;
		return;
	}
	if (drvloaded(drive) == 0)
	{
		u.u_error = ENXIO;
		return;
	}
	aphysio(atstrategy, dev, B_WRITE);
}
