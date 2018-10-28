atreread(dev,drive)
{
register struct buf *bp;
register unsigned int unit, part, tmpdev;
register struct ap *ap;
register struct ah *ah;

	ad_parm[drive].ad_tab_in |= AD_READ;
	printf("HD: reread: calling bread() on drive %u\n",drive);
	bp = (struct buf *)bread(dev | 7,0);
	if (bp->b_flags & B_ERROR)
	{
		extern lbolt;

		printf("HD: reread: got a B_ERROR on drive %u\n",drive);
		brelse(bp);
		printf("HD: reread: will sleep now on drive %u\n",drive);
		sleep(&lbolt,PWAIT);
		if (!atnoerr)
		{
			printf("HD: reread: set u_error to ENXIO on drive %u\n",
				drive);
			if ((drive + 1) >= at_ndrv)
				u.u_error = ENXIO;
		}
		printf("HD: reread: B_ERROR return on drive %u\n",drive);
		return;
	}
	printf("HD: reread: no error from bread() on drive %u\n",drive);
	ap = (struct ap *)bp->b_addr;
	ah = (struct ah *)&(ap->ap_code[AH_START]);
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
	printf("WARNING: no header on block 0, using 4 head 10 Meg default\n");
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
						unit = 0;
						break;
				case AP_TMP:
				case AP_TMP_1:
				case AP_TMP_2:
				case AP_TMP_3:
						unit = 1;
						break;
				case AP_USR:
				case AP_USR_1:
				case AP_USR_2:
				case AP_USR_3:
						unit = 2;
						break;
				case AP_DOS:
				case AP_DOS_4:
						unit = 3;
						break;
				default:
					printf("HD: unused partition %u\n",
						part);
					brelse(bp);
					return;
			}
			at_sizes[drive][unit].nblock =
				ap->ap_tab[part].ap_size;
			at_sizes[drive][unit].oblock =
				ap->ap_tab[part].ap_start;
		}
	}
	else
	{
		printf("WARNING: invalid signature on block 0\n");
	}
	printf("HD: reread: calling brelse() on drive %u\n",drive);
	brelse(bp);
	printf("HD: reread: normal return on drive %u\n",drive);
	return (0);
}
