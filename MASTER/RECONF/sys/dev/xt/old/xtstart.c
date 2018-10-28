xtstart()
{
register struct buf *bp;
register unsigned int count;
unsigned int addr;
struct	 ib
	 {
		char	lobyte;
		char	hibyte;
	 };

	if ((bp = xttab.d_actf) == NULL)
		return;
	xttab.d_active++;
	xtcntrl = 0;

	/*
	 * Set up DMA controller.
	 */
	io_outb(0x0c, 1);		/* Clear first/last FF */
	if (bp->b_flags & B_READ)
	{
		io_outb(0x0c, DMAREAD);
		io_outb(0x0b, DMAREAD);
	}
	else
	{
		io_outb(0x0c, DMAWRITE);
		io_outb(0x0b, DMAWRITE);
	}
	addr = (unsigned int)bp->b_addr;
	if (bp->b_flags & B_PHYS)
	{
		/*
		 * Check for stradling of 64kb boundry.
		 */
		if (addr > 0xFE00)
		{
			xtcntrl--;
			if ((bp->b_flags & B_READ) == 0)
			{
				addr = spl6();
				count = u.u_ds;
				u.u_ds = (bp->b_xmem << 12) + 32;
				copyin(bp->b_addr - 512, xt_bp->b_addr, 512);
				u.u_ds = count;
				splx(addr);
			}
			addr = (unsigned int) xt_bp->b_addr;
			goto cont;
		}
		io_outb(DMAPAGE, bp->b_xmem);
		io_outb(DMAADDR, ((struct ib *)&bp->b_addr)->lobyte);
		io_outb(DMAADDR, ((struct ib *)&bp->b_addr)->hibyte);
		count = ((-bp->b_wcount << 1) + 0x1FF) & ~0x1FF;

		/*
		 * Long DMA transfers will delay the RAM refresh of memory
		 * on the I/O bus so that memory can get corrupted.
		 *
		 * Also check for 64kb DMA wraparound.
		 */
		if ((count > xt_mxfer) || 
		    ((((unsigned int) bp->b_addr) + count) < count))
		{
			xtcntrl++;
			if ((count > xt_mxfer) &&
			    ((((unsigned int) bp->b_addr) + count) >= count))
			{
				count = xt_mxfer;
			}
			else
			{
				count = -((unsigned int) bp->b_addr); 
				if ((count & 0x1FF) != 0)    /* bad boundry */
					count &= ~0x1FF;
				else
					bp->b_xmem++;
			}
			bp->b_addr += count;
			bp->b_wcount += count >> 1;
		}
	}
	else
	{
cont:		count = addr >> 4;
		count += getds();
		io_outb(DMAPAGE, count >> 12);
		io_outb(DMAADDR, (count << 4) + (addr & 017));
		io_outb(DMAADDR, count >> 4);
		count = 512;
	}
	xt_cmd.c_bcnt = count >> 9;
	count--;
	io_outb(DMACOUNT, count);
	io_outb(DMACOUNT, count >> 8);

	/*
	 * Set up the rest of XT command packet.
	 */
	count = bp->b_blkno + xt_sizes[addr = bp->b_dev & 037].oblock;
	xt_cmd.c_sect = count % xt_sechd;
	count /= xt_sechd;
	xt_cmd.c_head = (count % xt_nhead) | (addr&030) << 2;
	count /= xt_nhead;
	xt_cmd.c_cyln = count;
	xt_cmd.c_sect |= (count >> 2) & 0xC0;
	xt_cmd.c_ctrl = CNTF;

	/*
	 * Start the action.
	 */
	if (xtcmd(bp->b_flags & B_READ ? READ : WRITE, 0x3))
	{
		xtcntrl = 2;
		xtintr();		/* print the error */
		return;
	}
	io_outb(0x0a, DMACHAN);
	io_outb(0x21, io_inb(0x21) & ~(01 << IOINT));
}
