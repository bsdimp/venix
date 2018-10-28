/* i/o to any port on 8088 (like /dev/mem) */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/conf.h>

/* read bytes starting at port in offset */

ioread(dev)
{
register int c;

	c = u.u_offset[1];
	spl5();
	while ((u.u_error == 0) && (passc(io_inb(c++)) >= 0));
	spl0();
}

/* write bytes starting at port in offset */

iowrite(dev)
{
register int c, cp;

	c = u.u_offset[1];
	spl5();
	while(((cp = cpass()) != -1) && (u.u_error == 0))
		io_outb(c++,cp);
	spl0();
}
