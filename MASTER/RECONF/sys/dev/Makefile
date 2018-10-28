CC	= cc
CFLAGS	= -O -DSCCS -I../
DEVLIB	= ../devlib.a

NEWSRC	= at.c ioport.c npipe.c
OLDSRC	= xt.c
STDSRC	= ca.c console.c floppy.c mem.c msg.c pa.c
SOURCES	= ${NEWSRC} ${OLDSRC} ${STDSRC}

NEWOBJ	= at.o ioport.o npipe.o
OLDOBJ	= xt.o
STDOBJ	= ca.o console.o floppy.o mem.o msg.o pa.o
OBJECTS	= ${NEWOBJ} ${OLDOBJ} ${STDOBJ}

all:	${OBJECTS}
	ar rv ${DEVLIB} $?
	ranlib ${DEVLIB}

at.o	: at/*.c at/*.h
	${CC} ${CFLAGS} -DATASI -Iat/ -c at/at.c

xt.o	: xt/*.c xt/*.h
	${CC} ${CFLAGS} -DATASI -Ixt/ -c xt/xt.c

console.o: console.c
	${CC} ${CFLAGS} -DDEBUG -c console.c

clean:
	rm -f *.o core a.out ${OBJECTS}
