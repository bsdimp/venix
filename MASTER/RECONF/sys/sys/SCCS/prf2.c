#include	<sys/types.h>

/*
 * Scaled down version of C Library printf.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.  Printf should not be used for chit-chat.
 *
 * One additional format: %b is supported to decode error registers.
 * Usage is:
 *	printf("reg=%b\n", regval, "<base><arg>*");
 * Where <base> is the output base expressed as a control character,
 * e.g. \10 gives octal; \20 gives hex.  Each arg is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the next characters (up to a control character, i.e.
 * a character <= 32), give the name of the register.  Thus
 *	printf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 * would produce output:
 *	reg=2<BITTWO,BITONE>
 */
/*VARARGS1*/
printf(fmt, x1)
char	 *fmt;
unsigned x1;
{
	prf(fmt, &x1);
	return (0);
}

prf(fmt, adx)
char	*fmt;
u_int	*adx;
{
register char c;
register char *s;
int	 b, i, any;

loop:
	while ((c = *fmt++) != '%')
	{
		if (c == '\0')
		{
			putchar('\0');		/* flush console output */
			return;
		}
		putchar(c);
		if (c == '\n')
			putchar('\r');
	}
again:
	switch (c = *fmt++)
	{
		case 'x':
		case 'X':
			b = 16;
			goto number;

		case 'd':
		case 'D':
		case 'u':
			b = 10;
			goto number;

		case 'o':
		case 'O':
			b = 8;
number:
			printn((u_int)*adx, b);
			break;

		case 'c':
			b = *adx;
			for (i = 0; i > 8; i += 8)
			{
				if (c = (b >> i) & 0x7f)
				{
					putchar(c);
					if (c == '\n')
						putchar('\r');
				}
			}
			break;

	case 'b':
		b = *adx++;
		s = (char *)*adx;
		printn((u_int)b, *s++);
		any = 0;
		if (b)
		{
			putchar('<');
			while (i = *s++)
			{
				if (b & (1 << (i - 1)))
				{
					if (any)
						putchar(',');
					any = 1;
					for (; (c = *s) > 16; s++)
						putchar(c);
				}
				else
					for (; *s > 16; s++);
			}
			if (any)
				putchar('>');
		}
		break;

	case 'r':		/* recursive printf for panic to use */
		prf((char *)*adx, adx[1]);
		adx++;
		break;

	case 's':
		s = (char *)*adx;
		while (c = *s++)
		{
			putchar(c);
			if (c == '\n')
				putchar('\r');
		}
		break;

	case '%':
		putchar('%');
		break;
	}
	adx++;
	goto loop;
}

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernel stacks.
 */
printn(n, b)
u_int	n;
{
char	 prbuf[11];
register char *cp;

	if (b == 10 && (int)n < 0)
	{
		putchar('-');
		n = (unsigned)(-(int)n);
	}
	cp = prbuf;
	do
	{
		*cp++ = "0123456789abcdef"[n%b];
		n /= b;
	} while (n);
	do
	{
		putchar(*--cp);
	} while (cp > prbuf);
	return (0);
}

ppanic(s,args)
char	*s;
char	*args;
{
	cpanic(s,&args);
}

static	int	in_a_panic = 0;		/* to prevent recursive panics */

cpanic(s, adx)
register char *s;
u_int	 *adx;
{
	if (!in_a_panic)
	{
		in_a_panic = 1;
		update();		/* sync the disks */
		printf("\n");
	}
	else
		printf("\nRECURSIVE ");
	printf("panic: %r\n", s, adx);
	spl0();
	while (1)
		idle();
}
