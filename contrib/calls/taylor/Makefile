#	use 'make' to compile calls
#	use 'make install' to compile and install calls and install the man page
#	use 'make clean' to clean up the directory

CFLAGS = -O
DESTDIR = /usr/local/bin
DOCDIR = /usr/man/local/man1

$(DESTDIR)/calls: calls.c
	$(CC) $(CFLAGS) -s calls.c -o $(DESTDIR)/calls

everything: calls install clean

calls:	calls.c
	$(CC) $(CFLAGS) calls.c -o calls

install:	calls
	cp calls $(DESTDIR)/calls
	strip $(DESTDIR)/calls
	cp calls.l $(DOCDIR)/calls.l

clean:
	rm calls
