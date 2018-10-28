int	pc, ps, cs, ss, ds, es, ax, bx, cx, dx, si, di, sp, bp;
int	zero_bp, two_bp;
extern	unsigned int etext;
static	int i;


panic(s,args)
char	*s;
char	*args;
{
	i = 1;
	while (pc && (pc < etext))
	{
		pc = two_bp;
		printf("caller %d pc: 0x%x\n",i++,pc);
		ax = zero_bp;
		bp = ax;
		if (bp < sp)
			break;
	}
	printf("\nPANIC: callers registers:\n");
	printf("\tPC = 0x%x\tPS = 0x%x\n",
		pc,ps);
	printf("\tCS = 0x%x\tSS = 0x%x\tDS = 0x%x\tES = 0x%x\n",
		cs,ss,ds,es);
	printf("\tAX = 0x%x\tBX = 0x%x\tCX = 0x%x\tDX = 0x%x\n",
		ax,bx,cx,dx);
	printf("\tSI = 0x%x\tDI = 0x%x\tSP = 0x%x\tBP = 0x%x\n",
		si,di,sp,bp);
	ppanic(s,args);
	cpanic(s,&args);
	return (0);
}
