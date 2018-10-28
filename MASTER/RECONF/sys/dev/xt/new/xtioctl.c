xtioctl(dev,cmd,addr)
char    *addr;
{
register int unit, drive;
struct   diskparm buf;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= xt_ndrv) || (unit >= NUNIT))
	{
		printf("drive = %u  >=  xt_ndrv = %u\n",drive,xt_ndrv);
		printf("unit  = %u  >=  NUNIT   = %u\n",unit,NUNIT);
		printf("dev   = 0x%x\n",dev);
		u.u_error = ENXIO;
		return;
	}
	switch (cmd)
	{
		case I_GETDPP:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if (unit >= xd_parm[drive].xd_npart)
			{
				printf("unit  = %u  >=  xd_npart = %u\n",
					unit,xd_parm[drive].xd_npart);
				printf("drive = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
			{
		printf("partition table for drive %u is not in memory\n",
			drive);
				printf("unit = %u  dev = 0x%x\n",unit,dev);
				/* change to block device */
				dev = makedev(1,minor(dev));
				brelse(bread(dev,0));
			}
			buf.d_nblock = xt_sizes[drive][unit].nblock;
			buf.d_offset = xt_sizes[drive][unit].oblock;
			buf.d_nsect  = xd_parm[drive].xd_sechd;
			buf.d_nhead  = xd_parm[drive].xd_nhead;
			buf.d_ntrack = xd_parm[drive].xd_ntrack;
			if (copyout(&buf, addr, sizeof(buf)))
				u.u_error = EFAULT;
			return;

		case I_REREAD:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			tabinoff(drive);
			/* change to block device */
			dev = makedev(1,minor(dev));
			brelse(bread(dev,0));
			return;

		case I_SETNDRV:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
			{
		printf("partition table for drive %u is not in memory\n",
			drive);
				printf("unit = %u  dev = 0x%x\n",unit,dev);
				/* change to block device */
				dev = makedev(1,minor(dev));
				brelse(bread(dev,0));
			}
			if (copyin(addr, &buf, sizeof(buf)))
			{
				u.u_error = EFAULT;
				return;
			}
			if ((buf.d_nblock == 0) && (buf.d_offset == 0))
			{
				if ((0 < buf.d_ntrack) && (buf.d_ntrack <=NDRV))
					xt_ndrv = buf.d_ntrack;
				else
					u.u_error = EINVAL;
			}
			else
				u.u_error = EINVAL;
			return;

		case I_LOAD:
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (drvloaded(drive))
				return;
			drvlodon(drive);
			tabinoff(drive);
			/* change to block device */
			dev = makedev(1,minor(dev));
			brelse(bread(dev,0));
			return;

		case I_UNLOAD:
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (drvloaded(drive) == 0)
				return;
			drvlodoff(drive);
			tabinoff(drive);
			return;

		case I_DUMP:
			if (drvloaded(drive) == 0)
			{
				printf("drive = %u is not loaded\n",drive);
				printf("unit  = %u  dev = 0x%x\n",unit,dev);
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				printf("unit+1 = %u  !=  NUNIT = %u\n",
					unit+1,NUNIT);
				printf("drive  = %u  dev = 0x%x\n",drive,dev);
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
			{
		printf("partition table for drive %u is not in memory\n",
			drive);
				printf("unit = %u  dev = 0x%x\n",unit,dev);
				/* change to block device */
				dev = makedev(1,minor(dev));
				brelse(bread(dev,0));
			}
			printf("HD: drive   = %u\n",drive);
			printf("HD: unit    = %u\n",unit);
			printf("HD: npart   = %u\n",xd_parm[drive].xd_npart);
			printf("HD: nunit   = %u\n",xd_parm[drive].xd_nunit);
			printf("HD: ntrack  = %u\n",xd_parm[drive].xd_ntrack);
			printf("HD: nhead   = %u\n",xd_parm[drive].xd_nhead);
			printf("HD: sechd   = %u\n",xd_parm[drive].xd_sechd);
			printf("HD: ncyls   = %u\n",xd_parm[drive].xd_ncyls);
			printf("HD: blkscyl = %u\n",xd_parm[drive].xd_blkscyl);
			printf("HD: blksize = %u\n",xd_parm[drive].xd_blksize);
			printf("HD: cntf    = %u\n",xd_parm[drive].xd_cntf);
			printf("HD: maxfer  = %u\n",xd_parm[drive].xd_maxfer);
			return;

		default:
			u.u_error = EINVAL;
			return;
	}
}
