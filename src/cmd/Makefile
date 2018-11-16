
PROGS=basename cat chgrp chmod chown cmp comm cp date \
	ln mv yes
BROKEN=ar

CFLAGS_N=-z -O -s
CC_N=	${CC} ${CFLAGS_N} -o $@ $@.c
CFLAGS_I=-z -i -O -s
CC_I=	${CC} ${CFLAGS_I} -o $@ $@.c

all:	${PROGS}

clean:	${PROGS}
	rm ${PROGS}

ar:	ar.c
	${CC_I}
basename:	basename.c
	${CC_N}
cat:	cat.c
	${CC_N}
chgrp:	chgrp.c
	${CC_N}
chmod:	chmod.c
	${CC_N}
chown:	chown.c
	${CC_N}
cmp:	cmp.c
	${CC_N}
comm:	comm.c
	${CC_N}
cp:	cp.c
	${CC_I}
date:	date.c
	${CC_N}
ln:	ln.c
	${CC_N}
mv:	mv.c
	${CC_N}
yes:	yes.c
	${CC_N}
