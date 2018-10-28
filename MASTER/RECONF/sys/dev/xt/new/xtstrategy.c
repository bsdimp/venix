xtstrategy(abp)
struct	buf *abp;
{
register struct buf *bp;
register struct xp *xp;
register struct xh *xh;
register int unit, drive, part, tunit;
static	 char xtfirst = 0;		/* driver already called */
int	 s, xtintr();

	drive = minor(abp->b_dev) / NUNIT;
	unit  = minor(abp->b_dev) % NUNIT;
	if (++xd_parm[drive].xd_pcnt <= 0)
	{
		if (xt_debug)
			printf("xtstrategy: drive = %u: pass count overflow\n",
				drive);
		xd_parm[drive].xd_pcnt = 0;
	}
	if (xtfirst == 0)
	{
		xtfirst++;
		/*
		 * Set up the interrupt vector.
		 */
		setiva(0x34, xtintr, xtvec);

		/*
		 * Calculate number of heads.  First try max,
		 * if error then decrease by 1, try again.
		 */
		xtnoerr++;
		while (xd_parm[0].xd_nhead > 0)
		{
			/*
				Must insure major device number to be the one of
				the block device and not character.
				It happens to be that major device number for
				the winchester is 1 and character is 5.
				Also, preserve the original minor device number
				and then OR in the minor number for physical
				partition.  Then try to read the last
				physical block on the disk as dictated by
				the supplied number of heads and sectors
				per head variables.
			*/
			bp = (struct buf *)bread(((abp->b_dev & 0x01ff) | 7),
				xd_parm[0].xd_sechd *
				(xd_parm[0].xd_nhead - 1));
			brelse(bp);
			if (bp->b_flags & B_ERROR)
			{
				extern lbolt;

				xd_parm[0].xd_nhead--;
				u.u_error = 0;
				sleep(&lbolt,PWAIT);	/* delay */
			}
			else
				break;
		}
		xtnoerr = 0;
		xd_parm[0].xd_blkscyl = xd_parm[0].xd_nhead *
			xd_parm[0].xd_sechd;
		if (xt_debug)
			printf("HD: strat: drive 0 has %u heads: pass %u\n",
				xd_parm[0].xd_nhead,xd_parm[0].xd_pcnt);
		/*
		 * Get a buffer for raw i/o which stradles a 64kb boundry.
		 */
		xt_bp = (struct buf *)getblk(NODEV);
	}
	if ((tabinmem(drive) == 0) && (unit < xd_parm[drive].xd_npart))
	{
		if (xt_debug)
		printf("HD: strat: first access: drive=%u unit=%u pass=%u\n",
			drive,unit,xd_parm[drive].xd_pcnt);
		tabinon(drive);
		/*
			Must insure major device number to be the one of
			the block device and not character.
			It happens to be that major device number for
			the winchester is 1 and character is 5.
			Also, preserve the original minor device number
			and then OR in the minor number for physical
			partition.
		*/
		bp = (struct buf *)bread(((abp->b_dev & 0x01ff) | 7), 0);
		xp = (struct xp  *)bp->b_addr;
		xh = (struct xh  *)&xp->xp_code[XH_START];
		if ((xh->xh_sig1 == XH_SIG1) && (xh->xh_sig2 == XH_SIG2))
		{
			xd_parm[drive].xd_npart   = xh->xh_npart;
			xd_parm[drive].xd_nunit   = xh->xh_nunit;
			xd_parm[drive].xd_ntrack  = xh->xh_ntrack;
			xd_parm[drive].xd_nhead   = xh->xh_nhead;
			xd_parm[drive].xd_sechd   = xh->xh_sechd;
			xd_parm[drive].xd_ncyls   = xh->xh_ncyls;
			xd_parm[drive].xd_blkscyl = xh->xh_blkscyl;
			xd_parm[drive].xd_blksize = xh->xh_blksize;
			xd_parm[drive].xd_cntf    = xh->xh_cntf;
			xd_parm[drive].xd_maxfer  = xh->xh_maxfer;
		}
		else
		{
#ifdef	ATASI
printf("WARNING: no parm header on block 0, using 7 head 19.5 Meg default\n");
#else	ATASI
printf("WARNING: no parm header on block 0, using 4 head 10 Meg default\n");
#endif	ATASI
		}
		if (xp->xp_sig == XP_SIG)
		{
			for (part = 0; part < xd_parm[drive].xd_npart; part++)
			{
				switch (xp->xp_tab[part].xp_sys)
				{
					case XP_SYS:
					case XP_SYS_1:
					case XP_SYS_2:
					case XP_SYS_3:
						tunit = 0;
						break;

					case XP_TMP:
					case XP_TMP_1:
					case XP_TMP_2:
					case XP_TMP_3:
						tunit = 1;
						break;

					case XP_USR:
					case XP_USR_1:
					case XP_USR_2:
					case XP_USR_3:
						tunit = 2;
						break;

					case XP_DOS:
					case XP_DOS_4:
						tunit = 3;
						break;

					default:
						if (xt_debug)
	printf("HD: unused partition %u on drive %u unit %u\n",part,drive,unit);
					/*
						usage of part in follwing
						two assignments needs to
						be rethought!!!
					*/
					xt_sizes[drive][part].nblock = 0;
					xt_sizes[drive][part].oblock = 0;
					continue;
				}
				xt_sizes[drive][tunit].nblock =
					xp->xp_tab[part].xp_size;
				xt_sizes[drive][tunit].oblock =
					xp->xp_tab[part].xp_start;
			}
		}
		else
		{
	printf("WARNING: invalid signature on block 0 of drive %u unit %u\n",
			drive,unit);
		}
		brelse(bp);
	}

	/*
	 * Verify minor device and request block range.
	 */
	bp = abp;
	if ((unit >= NUNIT) || (drive >= xt_ndrv) ||
		((bp->b_blkno + ((255 - bp->b_wcount) >> 8)) >
		xt_sizes[drive][unit].nblock))
	{
		if (xt_debug && (xtnoerr == 0))
		{
			printf("HD: strat: something is wrong: pass %u\n",
				xd_parm[drive].xd_pcnt);
			printf("    unit    = %u\t>=  NUNIT     = %u\n",
				unit,NUNIT);
			printf("    drive   = %u\t>=  xt_ndrv   = %u\n",
				drive,xt_ndrv);
			printf("    b_blkno = %u\t    b_wcount  = %u\n",
				bp->b_blkno,bp->b_wcount);
			printf("    blkno   = %u\t>=  nblock    = %u\n",
				(bp->b_blkno + ((255 - bp->b_wcount) >> 8)),
				xt_sizes[drive][unit].nblock);
		}
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

	/*
	 * Calculate approximate cylinder for sorting,
	 * queue the request, and
	 * start the transfer if none in progress.
	 */
	bp->b_cylin = (bp->b_blkno + xt_sizes[drive][unit].oblock) /
		xd_parm[drive].xd_blkscyl;
	spl5();
	disksort(&xttab, bp);
	if (xttab.d_active == 0)
		xtstart();
	spl0();
}
