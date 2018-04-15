#include <err.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

#include <ddb/ddb.h>
#include <ddb/db_output.h>
#include <ddb/db_sym.h>

#include "../dis88/venix_a_out.h"

static void
usage(void)
{
	fprintf(stderr, "usage: ddb fn.o\n");
	exit(1);
}

struct exec *aout;

int
main(int argc, char **argv)
{
	const char *fn;
	int fd;
	struct stat sb;
	void *file;
	vm_offset_t ptr;

	if (argc < 2)
		usage();
	fn = argv[1];
	fd = open(fn, O_RDONLY);
	if (fd == -1)
		err(1, "open");
	if (fstat(fd, &sb) == -1)
		err(1, "fstat");
	file = malloc(sb.st_size);
	if (file == NULL)
		errx(1, "Can't allocate %lld bytes", (long long)sb.st_size);
	if (read(fd, file, sb.st_size) != sb.st_size)
		err(1, "read");
	aout = (struct exec *)file;
	if (N_BADMAG(*aout))
		errx(1, "Bad magic number 0%o\n", aout->a_magic);

	ptr = 0;
	while (ptr < aout->a_text) {
		uint8_t ret, ch;

		printf("%#x:\t", ptr);
		db_read_bytes(ptr, 1, (char *)&ret);
		ptr = db_disasm(ptr, false);
		db_read_bytes(ptr, 1, (char *)&ch);
		if (ptr == 0x35 || ((ptr & 1) && (ret == 0xc3 && ch == 0))) {
			printf("%#x:\t.byte\t0\n", ptr);
			ptr++;
		}
	}
}

struct trapframe *kdb_frame;

void
kdb_reenter()
{
}

int
db_segsize(struct trapframe *tfp)
{
	return 16;
}

/*
 * Read bytes from kernel address space for debugger.
 */
int
db_read_bytes(vm_offset_t addr, size_t size, char *data)
{
	char *core = (char *)aout + sizeof(*aout);

	// check addr for bounds
	for (int i = 0; i < size; i++)
		data[i] = core[i + addr];

	return 0;
}

/*
 * Write bytes to kernel address space for debugger.
 */
int
db_write_bytes(vm_offset_t addr, size_t size, char *data)
{
	return 0;
}

void
db_printsym(db_expr_t off, db_strategy_t strategy)
{
	db_printf("%#lx", (long)off);
}

/*
 * Printing
 */
int
db_printf(const char *fmt, ...)
{
	va_list	listp;
	int retval;

	va_start(listp, fmt);
	retval = vprintf(fmt, listp);
	va_end(listp);

	return (retval);
}
