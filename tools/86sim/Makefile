PROG_CXX=86sim
MAN=

SRCS=86sim.cc main.cc
SRCS+=	sim_db.cc debug.cc
CFLAGS += -DVENIX -g

# Add in ddb
.PATH: ../ddb
SRCS+=	db_disasm.c opt_ddb.h db_access.c db_reloc.c db_sym.c opt_kstack_pages.h \
	db_run.c db_break.c db_watch.c db_command.c db_lex.c db_examine.c db_expr.c \
	db_variables.c db_output.c db_write_cmd.c db_input.c db_print.c
CFLAGS+=-O0 -I../ddb -Werror

.include <bsd.prog.mk>

# ../dis88 sysent.o | grep .globl | grep -v sysent | awk '{if (foo[$2] != "yes") { printf("/* %d %s */\nvoid\nvenix%s()\n{\n\n\terror(\"Unimplemented system call %d %s\\n\");\n}\n\n", call, $2, $2, call, $2); foo[$2]="yes";} call++;}'
# ../dis88 sysent.o | grep .globl | grep -v sysent | awk '{printf("\t&Venix::venix%s,\n", $2);}' > ../../86sim/venix_table
