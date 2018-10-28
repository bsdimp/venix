SOURCES	= \
	at.h \
	atstrategy.c atstart.c atintr.c atcmd.c \
	atread.c atwrite.c atioctl.c

ORIG	= atreread.c

all:	../at.c

../at.c: ${SOURCES}
	rm -f ../at.c
	for i in ${SOURCES}; \
	do \
		cat $$i >> ../at.c; \
		echo >> ../at.c; \
	done
