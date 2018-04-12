PROG_CXX=86sim
MAN=

SRCS=86sim.cc
CFLAGS += -DVENIX

.include <bsd.prog.mk>

# ../dis88 sysent.o | grep .globl | grep -v sysent | awk '{if (foo[$2] != "yes") { printf("/* %d %s */\nvoid\nvenix%s()\n{\n\n\terror(\"Unimplemented system call %d %s\\n\");\n}\n\n", call, $2, $2, call, $2); foo[$2]="yes";} call++;}'
# ../dis88 sysent.o | grep .globl | grep -v sysent | awk '{printf("\t&Venix::venix%s,\n", $2);}' > ../../86sim/venix_table
