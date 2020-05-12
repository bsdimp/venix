#	use 'make' to compile calls
#	use 'make install' to compile and install calls and install the man page
#	use 'make clean' to clean up the directory

# CFLAGS = -g
CFLAGS = -O -p
DESTDIR = /u/ksbszabo/bin
DOCDIR = /u/ksbszabo/man/man1

calls:	calls.c
	$(CC) $(CFLAGS) calls.c -o calls

lint:
	lint calls.c

ctrace:
	ctrace -p'fprintf(stderr,' calls.c > /tmp/calls.c
	cc /tmp/calls.c -o ctcalls

everything: calls install clean

$(DESTDIR)/calls: calls.c
	$(CC) $(CFLAGS) -s calls.c -o $(DESTDIR)/calls

install:	calls
	cp calls $(DESTDIR)/calls
	strip $(DESTDIR)/calls
	cp calls.1 $(DOCDIR)/calls.1

clean:
	rm calls
