CC	= cc
OPTIONS	= -DSCCS -I. -I.. -I../.. -I../../h
CFLAGS	= -O ${OPTIONS}

CPP	= cpp
CPPFLAGS= -M ${OPTIONS}

SOURCES	= doparm.c dopart.c drvutil.c main.c prfutil.c scrutil.c       

OBJECTS	= doparm.o dopart.o drvutil.o main.o prfutil.o scrutil.o       

PROG	= fdisk

all:	${PROG}

${PROG}: ${OBJECTS}
	${CC} -o ${PROG} ${OBJECTS}

clean :
	rm -f ${OBJECTS} *.o a.out core Makefile.bak

clobber: clean
	rm -f ${PROG}

depend:
	cp Makefile Makefile.bak
	echo '# DO NOT DELETE THIS LINE - make depend uses it' >makedep
	for i in ${SOURCES} ; \
	do \
		${CPP} ${CPPFLAGS} $$i >> makedep ; \
	done
	echo '/^# DO NOT DELETE THIS LINE/,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	ed - Makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE - make depend uses it
