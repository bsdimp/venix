PROG=venix_ddb
SRCS=venix_ddb.c db_disasm.c opt_ddb.h db_access.c db_reloc.c db_sym.c opt_kstack_pages.h
CFLAGS+=-I. -O0
MAN=

opt_ddb.h:
	touch $@
opt_kstack_pages.h:
	touch $@

.include <bsd.prog.mk>
