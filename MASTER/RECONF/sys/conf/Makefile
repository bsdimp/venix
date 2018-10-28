#
# Put the kernel together for various hardware configurations.	3/9/84
#
# Defines:
#	ACA	== include Async. Comm. Adapter (RS-232) driver.
#	PA	== include Printer Adapter driver.
#	FLP	== include Floppy driver.
#	WIN	== include Winchester driver.
#		ATASI	== select Atasi Winchester driver.
#	IOPORT	== include io-port driver
#	NPIPE	== include named-pipe driver
#	MOUSE	== include mouse driver
#	S_FLP	== system is floppy based.
#	S_WIN	== system is winchester based.
#	SMALL	== use parameters (from param.h) appropiate for a small system.
#
# Makes:
#	venix	== winchester based with floppy driver
#	venix.f	== floppy based with winchester driver
#	venix.fo== floppy based with no winchester driver
#
AS	= as
CC	= cc
LD	= ld

venix:		../syslib.a ../devlib.a low.o c.o
	$(LD) -x -o venix low.o c.o ../syslib.a ../devlib.a
	chmod 444 venix

c.o:	c.c
	$(CC) -DACA -DPA -DFLP -DWIN -DS_WIN -I.. -c c.c

venix.f:	../syslib.a ../devlib.a low.o c.f.o
	$(LD) -x -o venix.f low.o c.f.o ../syslib.a ../devlib.a
	chmod 444 venix.f

c.f.o:	c.c
	$(CC) -DACA -DPA -DFLP -DWIN -DS_FLP -I.. -c c.c
	mv c.o c.f.o

venix.fo:	../syslib.a ../devlib.a low.o c.fo.o
	$(LD) -x -o venix.fo low.o c.fo.o ../syslib.a ../devlib.a
	chmod 444 venix.fo

c.fo.o:	c.c
	$(CC) -DACA -DPA -DFLP -DS_FLP -DSMALL -I.. -c c.c
	mv c.o c.fo.o

low.o:	low.s
	${AS} -o low.o low.s
