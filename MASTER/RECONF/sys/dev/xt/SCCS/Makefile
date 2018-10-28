COMDIR	= com
NEWDIR	= new
OLDDIR	= old

COMSRC	= ${COMDIR}/xtcmd.c ${COMDIR}/xtintr.c

NEWSRC	= \
	${NEWDIR}/xt.h ${NEWDIR}/xtstrategy.c ${NEWDIR}/xtstart.c \
	${NEWDIR}/xtread.c ${NEWDIR}/xtwrite.c ${NEWDIR}/xtioctl.c

OLDSRC	= \
	${OLDDIR}/xt.h ${OLDDIR}/xtstrategy.c ${OLDDIR}/xtstart.c \
	${OLDDIR}/xtread.c ${OLDDIR}/xtwrite.c ${OLDDIR}/xtioctl.c

all:	../xt.c ../at.c

../xt.c: ${OLDSRC} ${COMSRC}
	rm -f ../xt.c
	for i in ${OLDSRC} ${COMSRC}; \
	do \
		cat $$i >> ../xt.c; \
		echo >> ../xt.c; \
	done

../at.c: ${NEWSRC} ${COMSRC}
	rm -f ../at.c
	for i in ${NEWSRC} ${COMSRC}; \
	do \
		cat $$i >> ../at.c; \
		echo >> ../at.c; \
	done
