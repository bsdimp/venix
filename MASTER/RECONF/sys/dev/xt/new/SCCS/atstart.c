atstart()
{
register struct buf *bp;
register unsigned int count;
unsigned int addr, unit, drive;
struct	 ib
	 {
		char	lobyte;
		char	hibyte;
	 };

	if ((bp = attab.d_actf) == NULL)
		return;
	attab.d_active++;
	atcntrl = 0;
	drive = minor(bp->b_dev) / NUNIT;
	unit = minor(bp->b_dev) % NUNIT;

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
			atcntrl--;
			if ((bp->b_flags & B_READ) == 0)
			{
				addr = spl6();
				count = u.u_ds;
				u.u_ds = (bp->b_xmem << 12) + 32;
				copyin(bp->b_addr - 512, at_bp->b_addr, 512);
				u.u_ds = count;
				splx(addr);
			}
			addr = (unsigned int) at_bp->b_addr;
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
		if ((count > ad_parm[drive].ad_maxfer) || 
		    ((((unsigned int) bp->b_addr) + count) < count))
		{
			atcntrl++;
			if (count > ad_parm[drive].ad_maxfer &&
			    ((((unsigned int) bp->b_addr) + count) >= count))
			{
				count = ad_parm[drive].ad_maxfer;
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
	at_cmd.c_bcnt = count >> 9;
	count--;
	io_outb(DMACOUNT, count);
	io_outb(DMACOUNT, count >> 8);

	/*
	 * Set up the rest of XT atasi command packet.
	 */
	count = bp->b_blkno + at_sizes[drive][unit].oblock;
	at_cmd.c_sect = count % ad_parm[drive].ad_sechd;
	count /= ad_parm[drive].ad_sechd;
	at_cmd.c_head = (count % ad_parm[drive].ad_nhead) | (drive << 5);
	count /= ad_parm[drive].ad_nhead;
	at_cmd.c_cyln = count;
	at_cmd.c_sect |= (count >> 2) & 0xC0;
	at_cmd.c_ctrl = ad_parm[drive].ad_cntf;

	/*
	 * Start the action.
	 */
	if (atcmd(bp->b_flags & B_READ ? READ : WRITE, 0x3))
	{
		atcntrl = 2;
		atintr();               /* print the error */
		return;
	}
	io_outb(0x0a, DMACHAN);
	io_outb(0x21, io_inb(0x21) & ~(01 << IOINT));
}
