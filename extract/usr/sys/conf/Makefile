#
# Put the kernel together for various hardware configurations.	3/9/84
#
# Defines:
#	ACA	== include Async. Comm. Adapter (RS-232) driver.
#	PA	== include Printer Adapter driver. (not needed for Rainbow
#		   because ACA is used for the printer also)
#	FLP	== include Floppy driver.
#	WIN	== include Winchester driver.
#	S_FLP	== system is floppy based.
#	S_WIN	== system is winchester based.
#	SMALL	== use parameters (from param.h) appropiate for a small system.
#
# Makes:
#	venix	== winchester based with floppy driver
#	venix.f	== floppy based with winchester driver
#	venix.fo== floppy based
#
CC=cc
LD=ld

venix:		../SYSLIB ../DEVLIB low.o c.o
	$(LD) -x -o venix low.o c.o ../SYSLIB ../DEVLIB
	vstrip venix
	chmod 444 venix

c.o:	c.c
	$(CC) -DACA -DFLP -DWIN -DS_WIN -I.. -c c.c

venix.f:	../SYSLIB ../DEVLIB low.o c.f.o
	$(LD) -x -o venix.f low.o c.f.o ../SYSLIB ../DEVLIB
	vstrip venix.f
	chmod 444 venix.f

c.f.o:	c.c
	$(CC) -DACA -DFLP -DWIN -DS_FLP -I.. -c c.c
	mv c.o c.f.o

venix.fo:	../SYSLIB ../DEVLIB low.o c.fo.o
	$(LD) -x -o venix.fo low.o c.fo.o ../SYSLIB ../DEVLIB
	vstrip venix.fo
	chmod 444 venix.fo

c.fo.o:	c.c
	$(CC) -DACA -DFLP -DS_FLP -DSMALL -I.. -c c.c
	mv c.o c.fo.o

