INCD	= /usr/include
CFLAGS	= -O -I./ -I../ -I../../ -I../../sys

SOURCES	= \
	kernmods.c main.c parameters.c partitions.c \
	sysarea.c usrarea.c utils.c

OBJS	= \
	kernmods.o main.o parameters.o partitions.o \
	sysarea.o usrarea.o utils.o

init:	${OBJS}
	${CC} -o init ${OBJS}

${OBJS}: install.h

install.h: ${INCD}/signal.h ${INCD}/sgtty.h ${INCD}/a.out.h ${INCD}/ctype.h
install.h: ${INCD}/sys/param.h ${INCD}/sys/filsys.h ${INCD}/sys/devparm.h
install.h: ${INCD}/sys/xtblk0.h
	touch install.h

clean:
	rm -f *.o core a.out ${OBJS} init
