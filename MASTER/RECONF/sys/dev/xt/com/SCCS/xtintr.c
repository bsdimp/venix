xtintr()
{
register struct buf *bp;
register int i;
char	 j = 0xFF;

	bp = xttab.d_actf;
	if (xtcntrl == 2)
		goto error;
	io_outb(0x21, io_inb(0x21) | (01 << IOINT));
	io_outb(0x20, 0x20);
	io_outb(0x0a, 07);
	io_outb(&IOADD->x_mask,0);
	if (xttab.d_active != 0)
	{
		if (io_inb(&IOADD->x_data) & 02)
		{
			if (xtcmd(RSTAT,0))
				goto error;
			for (i = 100; (io_inb(&IOADD->x_stat) & 01) == 0;)
				if (i-- == 0)
					goto error;
			j = io_inb(&IOADD->x_data) & 0x3f;
error:			if (xtnoerr == 0)
			{
				for (i = 0; xtmsg[i].num != 0; i++)
					if (xtmsg[i].num == j)
						break;
				deverror(bp, xtmsg[i].msg, j);
			}
			io_outb(&IOADD->x_stat,0);
			if (j != 0x18)
			{
				bp->b_flags |= B_ERROR;
				goto done;
			}
		}
		/*
		 * Finished with this transfer ?
		 */
		if (xtcntrl)
		{
			if (xtcntrl < 0)
			{
				if (bp->b_flags & B_READ)
				{
					i = u.u_ds;
					u.u_ds = (bp->b_xmem << 12) + 32;
					copyout(xt_bp->b_addr,
						bp->b_addr - 512, 512);
					u.u_ds = i;
				}
				bp->b_addr += 512;
				bp->b_xmem++;
				bp->b_wcount += 256;
			}
			if (bp->b_wcount >= 0)
				goto done;
			bp->b_blkno += xt_cmd.c_bcnt;
		}
		else
		{
done:			xttab.d_active = 0;
			xttab.d_actf = bp->av_forw;
			bp->b_resid = 0;
			iodone(bp);
		}
	}
	xtstart();
}
