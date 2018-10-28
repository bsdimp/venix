atioctl(dev,cmd,addr)
char    *addr;
{
register int unit, drive;
struct   diskparm buf;

	drive = minor(dev) / NUNIT;
	unit  = minor(dev) % NUNIT;
	if ((drive >= at_ndrv) || (unit >= NUNIT))
	{
		u.u_error = ENXIO;
		return;
	}
	switch (cmd)
	{
		case I_GETDPP:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if (unit >= ad_parm[drive].ad_npart)
			{
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
				brelse(bread(dev,0));
			buf.d_nblock = at_sizes[drive][unit].nblock;
			buf.d_offset = at_sizes[drive][unit].oblock;
			buf.d_nsect = ad_parm[drive].ad_sechd;
			buf.d_nhead = ad_parm[drive].ad_nhead;
			buf.d_ntrack = ad_parm[drive].ad_ntrack;
			if (copyout(&buf, addr, sizeof(buf)))
				u.u_error = EFAULT;
			return;

		case I_REREAD:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			tabinoff(drive);
			brelse(bread(dev,0));
			return;

		case I_SETNDRV:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
				brelse(bread(dev,0));
			if (copyin(addr, &buf, sizeof(buf)))
			{
				u.u_error = EFAULT;
				return;
			}
			if ((buf.d_nblock == 0) && (buf.d_offset == 0))
			{
				if ((0 < buf.d_ntrack) && (buf.d_ntrack <=NDRV))
					at_ndrv = buf.d_ntrack;
				else
					u.u_error = EINVAL;
			}
			else
				u.u_error = EINVAL;
			return;

		case I_LOAD:
			if (drvloaded(drive))
				return;
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			drvlodon(drive);
			tabinoff(drive);
			brelse(bread(dev,0));
			return;

		case I_UNLOAD:
			if (drvloaded(drive) == 0)
				return;
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			drvlodoff(drive);
			tabinoff(drive);
			return;

		case I_DUMP:
			if (drvloaded(drive) == 0)
			{
				u.u_error = ENXIO;
				return;
			}
			if ((unit + 1) != NUNIT)
			{
				u.u_error = ENODEV;
				return;
			}
			if (tabinmem(drive) == 0)
				brelse(bread(dev,0));
			printf("HD: drive   = %u\n",drive);
			printf("HD: unit    = %u\n",unit);
			printf("HD: npart   = %u\n",ad_parm[drive].ad_npart);
			printf("HD: nunit   = %u\n",ad_parm[drive].ad_nunit);
			printf("HD: ntrack  = %u\n",ad_parm[drive].ad_ntrack);
			printf("HD: nhead   = %u\n",ad_parm[drive].ad_nhead);
			printf("HD: sechd   = %u\n",ad_parm[drive].ad_sechd);
			printf("HD: ncyls   = %u\n",ad_parm[drive].ad_ncyls);
			printf("HD: blkscyl = %u\n",ad_parm[drive].ad_blkscyl);
			printf("HD: blksize = %u\n",ad_parm[drive].ad_blksize);
			printf("HD: cntf    = %u\n",ad_parm[drive].ad_cntf);
			printf("HD: maxfer  = %u\n",ad_parm[drive].ad_maxfer);
			return;

		default:
			u.u_error = EINVAL;
			return;
	}
}
