#
# Put the kernel together for various hardware configurations.
#
# Defines:
#	ACA	== include Async. Comm. Adapter {RS-232} driver.
#	PA	== include Printer Adapter driver.
#	FLP	== include Floppy driver.
#	WIN	== include Winchester driver.
#		ATASI	== driver with ATASI default partitioning information
#	IOPORT	== include io-port driver
#	NPIPE	== include named-pipe driver
#	MOUSE	== include mouse driver
#
#	S_FLP	== system is floppy based.
#	S_WIN	== system is winchester based.
#	SMALL	== use parameters {from param.h} appropiate for a small system.
#
# Makes:
#	venix		== winchester based with floppy driver
#	venix.n		== new winchester driver based with floppy driver
#	venix.f		== floppy based with winchester driver
#	venix.nf	== floppy based with new winchester driver
#	venix.fo	== floppy based with no winchester driver
#	venix.id	== winchester based with floppy driver
#			   with separated instruction and data
#	venix.f.id	== floppy based with winchester driver
#			   with separated instruction and data
#

AS	= as
CC	= cc
LD	= ld
OPTIONS	= -I.. -DSCCS -DDEBUG -DATASI
CFLAGS	= -O ${OPTIONS}
LDFLAGS	= -i -x -o

NEW	= -DIOPORT -DNPIPE -DMOUSE
COMMON	= -DACA -DPA -DFLP ${NEW}
W_BASE	= ${COMMON} -DWIN -DS_WIN
F_BASE	= ${COMMON} -DWIN -DS_FLP
SMALL	= ${COMMON} -DSMALL -DS_FLP

LOWSRC	= ../conf/low.s
CONFSRC	= ../conf/conf.c
SYSSRC	= ../sys/panic.s ../sys/prf2.c
OLDSRC	= ../dev/xt.c
NEWSRC	= ../dev/at.c
DEVSRC	= \
	../dev/ca.c \
	../dev/console.c \
	../dev/floppy.c \
	../dev/ioport.c \
	../dev/mem.c \
	../dev/msg.c \
	../dev/npipe.c \
	../dev/mouse.c \
	../dev/pa.c
SOURCES	= ${LOWSRC} ${CONFSRC} ${SYSSRC} ${DEVSRC} ${OLDSRC} ${NEWSRC}

CONFOBJ	= conf.o
LOWOBJ	= low.o
SYSOBJ	= panic.o prf2.o
SYSLIB	= syslib.a
OLDOBJ	= xt.o
NEWOBJ	= at.o
DEVOBJ	= \
	ca.o \
	console.o \
	floppy.o \
	ioport.o \
	mem.o \
	msg.o \
	npipe.o \
	mouse.o \
	pa.o
OBJECTS	= ${LOWOBJ} ${SYSLIB} ${DEVOBJ}

venix:	${OBJECTS} ${OLDOBJ} ${CONFOBJ}
	${LD} ${LDFLAGS} venix ${LOWOBJ} ${CONFOBJ} ${SYSLIB} ${DEVOBJ} \
		${OLDOBJ}
	chmod 444 $@

venix.n: ${OBJECTS} ${NEWOBJ} ${CONFOBJ}
	${LD} ${LDFLAGS} venix.n ${LOWOBJ} ${CONFOBJ} ${SYSLIB} ${DEVOBJ} \
		${NEWOBJ}
	chmod 444 $@

venix.f: ${OBJECTS} ${OLDOBJ} conf.f.o
	${LD} ${LDFLAGS} venix.f ${LOWOBJ} conf.f.o ${SYSLIB} ${DEVOBJ} \
		${OLDOBJ}
	chmod 444 $@

venix.nf: ${OBJECTS} ${NEWOBJ} conf.f.o
	${LD} ${LDFLAGS} venix.nf ${LOWOBJ} conf.f.o ${SYSLIB} ${DEVOBJ} \
		${NEWOBJ}
	chmod 444 $@

venix.fo: ${OBJECTS} ${OLDOBJ} conf.fo.o
	${LD} ${LDFLAGS} venix.fo ${LOWOBJ} conf.fo.o ${SYSLIB} ${DEVOBJ} \
		${OLDOBJ}
	chmod 444 $@

${CONFOBJ}: ${CONFSRC}
	${CC} ${CFLAGS} ${W_BASE} -c ${CONFSRC}
	mv ../conf/${CONFOBJ} $@

conf.f.o: ${CONFSRC}
	${CC} ${CFLAGS} ${F_BASE} -c ${CONFSRC}
	mv ../conf/${CONFOBJ} $@

conf.fo.o: ${CONFSRC}
	${CC} ${CFLAGS} ${SMALL} -c ${CONFSRC}
	mv ../conf/${CONFOBJ} $@

${LOWOBJ}: ${LOWSRC}
	${AS} -o ${LOWOBJ} ${LOWSRC}

${SYSLIB}: ${SYSOBJ}
	ranlib ${SYSLIB}

panic.o: ../sys/panic.s
	${AS} ../sys/panic.s
	mv ../sys/$@ $@
	ar rv ${SYSLIB} panic.o

prf2.o:	../sys/prf2.c
	${CC} ${CFLAGS} -c ../sys/prf2.c
	mv ../sys/$@ $@
	ar rv ${SYSLIB} prf2.o

ca.o:	   ../dev/ca.c
	${CC} ${CFLAGS} -c ../dev/ca.c
	mv ../dev/$@ $@

console.o: ../dev/console.c
	${CC} ${CFLAGS} -c ../dev/console.c
	mv ../dev/$@ $@

floppy.o:  ../dev/floppy.c
	${CC} ${CFLAGS} -c ../dev/floppy.c
	mv ../dev/$@ $@

ioport.o:  ../dev/ioport.c
	${CC} ${CFLAGS} -c ../dev/ioport.c
	mv ../dev/$@ $@

mem.o:	   ../dev/mem.c
	${CC} ${CFLAGS} -c ../dev/mem.c
	mv ../dev/$@ $@

msg.o:	   ../dev/msg.c
	${CC} ${CFLAGS} -c ../dev/msg.c
	mv ../dev/$@ $@

npipe.o:   ../dev/npipe.c
	${CC} ${CFLAGS} -c ../dev/npipe.c
	mv ../dev/$@ $@

mouse.o:   ../dev/mouse.c
	${CC} ${CFLAGS} -c ../dev/mouse.c
	mv ../dev/$@ $@

pa.o:	   ../dev/pa.c
	${CC} ${CFLAGS} -c ../dev/pa.c
	mv ../dev/$@ $@

xt.o:	   ../dev/xt.c
	${CC} ${CFLAGS} -c ../dev/xt.c
	mv ../dev/$@ $@

../dev/xt.c: ../dev/xt/com/*.c ../dev/xt/old/*.c ../dev/xt/old/*.h
	cd ../dev/xt;make ${MFLAGS} ../xt.c

at.o:	   ../dev/at.c
	${CC} ${CFLAGS} -c ../dev/at.c
	mv ../dev/$@ $@

../dev/at.c: ../dev/xt/com/*.c ../dev/xt/new/*.c ../dev/xt/new/*.h
	cd ../dev/xt;make ${MFLAGS} ../at.c
