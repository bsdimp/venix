xtstrategy(abp)
struct	buf *abp;
{
register struct buf *bp;
register int unit;
static	 char xtfirst = 0;	/* driver already called */
int	 xtintr();

	if (xtfirst == 0)
	{
		xtfirst++;
		/*
		 * Set up the interrupt vector.
		 */
		setiva(0x34, xtintr, xtvec);

		/*
		 * Calculate number of heads.  First try max,
		 * if error then decrease by 2, try again.
		 */
		xtnoerr++;
		while (xt_nhead > 0)
		{
			bp = (struct buf *)bread(abp->b_dev | 7,
				xt_sechd * (xt_nhead - 1));
			brelse(bp);
			if (bp->b_flags & B_ERROR)
			{
				extern lbolt;

				xt_nhead -= 2;
				u.u_error = 0;
				sleep(&lbolt,PWAIT);	/* delay */
			}
			else
				break;
		}
		xtnoerr = 0;

		/*
		 * Get a buffer for raw i/o which stradles a 64kb boundry.
		 */
		xt_bp = (struct buf *)getblk(NODEV);
	}

	/*
	 * Read in partition table from physical block 0
	 * if this is the first 0-3 partition table access.
	 */
	if ((xt_tab_in == 0) && ((abp->b_dev & 037) < 4))
	{
		int	part;
				
		xt_tab_in++;
		bp = (struct buf *)bread( abp->b_dev | 7, 0);
		if (((struct xp *)bp->b_addr)->xp_sig == XP_SIG)
		{
		    for (unit = 0; unit < 4; unit++)
		    {
			switch (((struct xp *)bp->b_addr)->xp_tab[unit].xp_sys)
			{
				case XP_SYS:
					part = 0;
					break;

				case XP_TMP:
					part = 1;
					break;

				case XP_USR:
					part = 2;
					break;

				default:
					part = 3;
					break;
			}
			xt_sizes[part].nblock =
			    ((struct xp *)bp->b_addr)->xp_tab[unit].xp_size;
			xt_sizes[part].oblock =
			    ((struct xp *)bp->b_addr)->xp_tab[unit].xp_start;
		    }
		}
		brelse(bp);
	}

	/*
	 * Verify minor device and request block range.
	 */
	unit = (bp = abp)->b_dev&037;
	if ((unit >= (8 * NDRV)) ||
	   (bp->b_blkno + ((255-bp->b_wcount) >> 8)) > xt_sizes[unit].nblock)
	{
		if (xtnoerr == 0)
		{
			printf("HD: strat: something is wrong\n");
			printf("    dev   = 0x%x\n",bp->b_dev);
			printf("    unit  = %u\t>=  (8 * NDRV) = %u\n",
				unit,(8 * NDRV));
			printf("    b_blkno = %u\t    b_wcount  = %u\n",
				bp->b_blkno,bp->b_wcount);
			printf("    blkno   = %u\t>=  nblock    = %u\n",
				(bp->b_blkno + ((255 - bp->b_wcount) >> 8)),
				xt_sizes[unit].nblock);
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
	bp->b_cylin = (bp->b_blkno + xt_sizes[unit].oblock) / (NHEAD * SECHEAD);
	spl5();
	disksort(&xttab, bp);
	if (xttab.d_active == 0)
		xtstart();
	spl0();
}
