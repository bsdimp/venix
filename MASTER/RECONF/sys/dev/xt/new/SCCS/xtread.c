xtread(dev)
{
register int drive, unit;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= xt_ndrv) || (unit >= NUNIT))
	{
		printf("cdev read drive = %u  >=  xt_ndrv = %u\n",
			drive,xt_ndrv);
		printf("cdev read unit  = %u  >=  NUNIT   = %u\n",
			unit,NUNIT);
		printf("cdev read dev   = 0x%x\n",dev);
		u.u_error = ENXIO;
		return;
	}
	if (drvloaded(drive) == 0)
	{
		printf("cdev read drive = %u is not loaded\n",drive);
		printf("cdev read unit  = %u  dev = 0x%x\n",unit,dev);
		u.u_error = ENXIO;
		return;
	}
	if (xt_debug)
		printf("cdev read drive = %u  unit = %u  dev = 0x%x\n",
			drive,unit,dev);
	aphysio(xtstrategy, dev, B_READ);
}
