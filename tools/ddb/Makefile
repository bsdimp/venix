PROG=venix_ddb
SRCS=venix_ddb.c db_disasm.c opt_ddb.h db_access.c db_reloc.c db_sym.c opt_kstack_pages.h \
	db_run.c db_break.c db_watch.c db_command.c db_lex.c db_examine.c db_expr.c \
	db_variables.c db_output.c db_write_cmd.c db_input.c db_print.c
CFLAGS+=-I. -O0 -Werror
MAN=

opt_ddb.h:
	touch $@
opt_kstack_pages.h:
	touch $@

.include <bsd.prog.mk>
