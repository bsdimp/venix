CPP	= /lib/cpp
CPPFLAGS= -DATASI

.s.o:
	${CPP} ${CPPFLAGS} < $*.s > x.s
	as x.s
	mv x.o $*.o
	rm -f x.s

all:	fboot xtboot xtboot.2hd

fboot:	fsboot.o ibmterm.o ibmflp.o
	ld -s fsboot.o ibmterm.o ibmflp.o
	cp magic.f fboot
	dd bs=32 skip=1 if=a.out of=fboot
	rm -f a.out

xtboot:	fsboot.o ibmterm.o ibmxt.o
	ld -s fsboot.o ibmterm.o ibmxt.o
	cp magic.w xtboot
	dd bs=32 skip=1 if=a.out of=xtboot
	rm -f a.out

xtboot.2hd: fsboot.o ibmterm.o ibmxt.2hd.o
	ld -s fsboot.o ibmterm.o ibmxt.2hd.o
	cp magic.w xtboot.2hd
	dd bs=32 skip=1 if=a.out of=xtboot.2hd
	rm -f a.out

clean:
	rm -f *.o core a.out Makefile.bak x.s

clobber: clean
	rm -f fboot xtboot xtboot.2hd
