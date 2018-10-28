SOURCES	= \
	xt.h \
	xtstrategy.c xtstart.c xtintr.c xtcmd.c \
	xtread.c xtwrite.c xtioctl.c

all:	../xt.c

../xt.c: ${SOURCES}
	rm -f ../xt.c
	for i in ${SOURCES}; \
	do \
		cat $$i >> ../xt.c; \
		echo >> ../xt.c; \
	done
