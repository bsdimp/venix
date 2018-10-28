atstrategy(abp)
struct	buf *abp;
{
register struct buf *bp;
register struct ap *ap;
register struct ah *ah;
register int unit, drive, part, tunit;
static	 char atfirst = 0;		/* driver already called */
int	 s, atintr();

	drive = minor(abp->b_dev) / NUNIT;
	unit  = minor(abp->b_dev) % NUNIT;
	if (atfirst == 0)
	{
		atfirst++;
		/*
		 * Set up the interrupt vector.
		 */
		setiva(0x34, atintr, atvec);

		/*
		 * Calculate number of heads.  First try max,
		 * if error then decrease by 1, try again.
		 */
		atnoerr++;
		while (ad_parm[0].ad_nhead > 0)
		{
			bp = (struct buf *)bread(abp->b_dev | 7,
				ad_parm[0].ad_sechd *
				(ad_parm[0].ad_nhead - 1));
			brelse(bp);
			if (bp->b_flags & B_ERROR)
			{
				extern lbolt;

				ad_parm[0].ad_nhead--;
				u.u_error = 0;
				sleep(&lbolt,PWAIT);	/* delay */
			}
			else
				break;
		}
		atnoerr = 0;
		ad_parm[0].ad_blkscyl = ad_parm[0].ad_nhead *
			ad_parm[0].ad_sechd;
		printf("HD: strat: drive 0 has %u heads\n",
			ad_parm[0].ad_nhead);
		/*
		 * Get a buffer for raw i/o which stradles a 64kb boundry.
		 */
		at_bp = (struct buf *)getblk(NODEV);
	}
	if ((tabinmem(drive) == 0) && (unit < ad_parm[drive].ad_npart))
	{
		printf("HD: strat: first access: drive=%u unit=%u\n",
			drive,unit);
		tabinon(drive);
		bp = (struct buf *)bread(abp->b_dev | 7, 0);
		ap = (struct ap  *)bp->b_addr;
		ah = (struct ah  *)&ap->ap_code[AH_START];
		if ((ah->ah_sig1 == AH_SIG1) && (ah->ah_sig2 == AH_SIG2))
		{
			ad_parm[drive].ad_npart   = ah->ah_npart;
			ad_parm[drive].ad_nunit   = ah->ah_nunit;
			ad_parm[drive].ad_ntrack  = ah->ah_ntrack;
			ad_parm[drive].ad_nhead   = ah->ah_nhead;
			ad_parm[drive].ad_sechd   = ah->ah_sechd;
			ad_parm[drive].ad_ncyls   = ah->ah_ncyls;
			ad_parm[drive].ad_blkscyl = ah->ah_blkscyl;
			ad_parm[drive].ad_blksize = ah->ah_blksize;
			ad_parm[drive].ad_cntf    = ah->ah_cntf;
			ad_parm[drive].ad_maxfer  = ah->ah_maxfer;
		}
		else
		{
#ifdef	ATASI
printf("WARNING: no parm header on block 0, using 7 head 19.5 Meg default\n");
#else	ATASI
printf("WARNING: no parm header on block 0, using 4 head 10 Meg default\n");
#endif	ATASI
		}
		if (ap->ap_sig == AP_SIG)
		{
			for (part = 0; part < ad_parm[drive].ad_npart; part++)
			{
				switch (ap->ap_tab[part].ap_sys)
				{
					case AP_SYS:
					case AP_SYS_1:
					case AP_SYS_2:
					case AP_SYS_3:
						tunit = 0;
						break;

					case AP_TMP:
					case AP_TMP_1:
					case AP_TMP_2:
					case AP_TMP_3:
						tunit = 1;
						break;

					case AP_USR:
					case AP_USR_1:
					case AP_USR_2:
					case AP_USR_3:
						tunit = 2;
						break;

					case AP_DOS:
					case AP_DOS_4:
						tunit = 3;
						break;

					default:
	printf("HD: unused partition %u on drive %u unit %u\n",part,drive,unit);
					/*
						usage of part in follwing
						two assignments needs to
						be rethought!!!
					*/
					at_sizes[drive][part].nblock = 0;
					at_sizes[drive][part].oblock = 0;
					continue;
				}
				at_sizes[drive][tunit].nblock =
					ap->ap_tab[part].ap_size;
				at_sizes[drive][tunit].oblock =
					ap->ap_tab[part].ap_start;
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
	if ((unit >= NUNIT) || (drive >= at_ndrv) ||
		((bp->b_blkno + ((255 - bp->b_wcount) >> 8)) >
		at_sizes[drive][unit].nblock))
	{
		printf("HD: strat: something is wrong\n");
		printf("    unit  = %u  >=  NUNIT   = %u\n",unit,NUNIT);
		printf("    drive = %u  >=  at_ndrv = %u\n",drive,at_ndrv);
		printf("    blkno = %u  >=  nblock  = %u\n",
			(bp->b_blkno + ((255 - bp->b_wcount) >> 8)),
			at_sizes[drive][unit].nblock);
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

	/*
	 * Calculate approximate cylinder for sorting,
	 * queue the request, and
	 * start the transfer if none in progress.
	 */
	bp->b_cylin = (bp->b_blkno + at_sizes[drive][unit].oblock) /
		ad_parm[drive].ad_blkscyl;
	spl5();
	disksort(&attab, bp);
	if (attab.d_active == 0)
		atstart();
	spl0();
}
