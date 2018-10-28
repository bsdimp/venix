CFLAGS	= -O -I../ -I../sys

SOURCES	= install.c numuser.c ddisk.c load.c

OBJS	= init numuser ddisk load

all:	${OBJS}

init:	../dev/xtblk0.h install.c
	${CC} ${CFLAGS} -o init install.c

numuser: numuser.c
	${CC} ${CFLAGS} -o numuser numuser.c

ddisk:	ddisk.c
	${CC} ${CFLAGS} -o ddisk ddisk.c

load:	load.c
	${CC} ${CFLAGS} -o load load.c

unload:	load
	ln load unload

ddump:	load
	ln load ddump

clean:
	rm -f *.o core a.out ${OBJS}
