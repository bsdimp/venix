.TH IOPORT 4
.SH NAME
ioport \- i/o capability to any 8088 port
.SH DESCRIPTION
.PP
Ioport is a special file that allows input and output to any i/o port
of the 8088 processor.
This allows for simple non-interupt driven devices drivers to be written and
callable from C.
It is similar to /dev/mem and typical usage is:

		lseek(filedes,(long) port, 0);
		read(filedes,&c,1);

.SH FILES
/dev/ioport
.SH BUGS
Many devices use registers that are reset when read so
care must be taken when writing to many ports.
