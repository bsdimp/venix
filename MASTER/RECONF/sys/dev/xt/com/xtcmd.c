xtcmd(command,mask)
{
register char *x;
register int i;

	xt_cmd.c_cmmd = command;
	io_outb(&IOADD->x_selt,0);
	io_outb(&IOADD->x_mask,mask);
	for (i = 1000; (io_inb(&IOADD->x_stat) & 0xD) != 0xD;)
		if (i-- == 0)
			return(-1);
	x = (char *)&xt_cmd;
	i = sizeof(xt_cmd);
	while (i--)
		io_outb(&IOADD->x_data, *x++);
	return(0);
}
