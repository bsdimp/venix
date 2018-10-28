xtioctl(dev,cmd,addr)
char	*addr; 
{
register int i;
struct	 diskparm buf;

	if (cmd == I_GETDPP)
	{
		if (xt_tab_in == 0)	/* make sure data is accurate */
			brelse(bread(dev,0));
		i = dev & 037;
		buf.d_nblock = xt_sizes[i].nblock;
		buf.d_offset = xt_sizes[i].oblock;
		buf.d_nsect = xt_sechd;
		buf.d_nhead = xt_nhead;
		buf.d_ntrack = 0;		/* don't care */
		if (copyout( &buf, addr, sizeof(buf)))
			u.u_error = EFAULT;
	}
	else
		u.u_error = EINVAL;
}
