# Makefile for calls, Kevin Braunsdorf PUCC

BIN =	/usr/local/bin
#DEFS= -DBADCALLOC
DEFS=
CFLAGS = -g ${DEFS}

SRCm=	Makefile
HDR =	main.h getopt.h scan.h
SRCc=	main.c getopt.c scan.c
OBJ =	main.o getopt.o scan.o
SRC =	${SRCm} ${HDR} ${SRCc}

all: calls

calls: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@

install: calls
	install -s -m 751 -o binary -g system calls ${BIN}

list:
	lpr ${SRC}

lint:
	lint -hnx ${SRCc}

clean:
	rm -f ${OBJ}

spotless: clean
	ci ${SRC}

${SRC}:
	co -l $@

depend: ${SRCc}
	maketd ${SRCc}

# DO NOT DELETE THIS LINE - make depend DEPENDS ON IT

main.o: getopt.h main.c main.h scan.h

getopt.o: getopt.c getopt.h

scan.o: main.h scan.c scan.h

# *** Do not add anything here - It will go away. ***
