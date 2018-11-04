#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

extern "C" {
#include <ddb/ddb.h>
#include <ddb/db_output.h>
#include <ddb/db_sym.h>
#include <ddb/db_reloc.h>
}

#include "../dis88/venix_a_out.h"

#include "machos.h"

char *strtab;
struct symtb *symtb;
vm_map_t *kernel_map = (vm_map_t *)"The droids";

#if 0
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
	char *start, *end;

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

	if (aout->a_syms != 0) {
		start = (char *)file + N_SYMOFF(*aout);
		symtb = (struct symtb *)start;
		end = start + aout->a_syms;
		strtab = (char *)file + N_STROFF(*aout);
		db_add_symbol_table(start, end, "main symbols", strtab);
		if (aout->a_trsize + aout->a_drsize != 0) {
			start = (char *)file + N_TXTOFF(*aout) + aout->a_text + aout->a_data;
			end  = start + aout->a_trsize;
			db_add_reloc_table(start, end, "text reloc", NULL);
			start = end;
			end = start + aout->a_drsize;
			db_add_reloc_table(start, end, "data reloc", NULL);
		}
	}

	printf("\t.code16\n");
	ptr = 0;
	while (ptr < aout->a_text) {
		uint8_t		ret, ch;
		c_db_sym_t	cursym;
		const char	*name;
		db_expr_t	d;

		cursym = db_search_symbol(ptr, DB_STGY_ANY, &d);
		db_symbol_values(cursym, &name, NULL);
		if (d == 0 && name != NULL)
			db_printf("%s:\n", name);
		db_printf("\t");
		db_read_bytes(ptr, 1, (char *)&ret);
		ptr = db_disasm(ptr, false);
		db_read_bytes(ptr, 1, (char *)&ch);
		if ((aout->a_stack != 0 && ptr == 0x35) ||
		    ((ptr & 1) && (ret == 0xc3 && ch == 0))) {
			printf("LL%#lx:\t.byte\t0\n", ptr);
			ptr++;
		}
	}
}
#endif

struct trapframe *kdb_frame;

extern "C"
int
db_segsize(struct trapframe *tfp)
{
	return 16;
}

/*
 * Read bytes from kernel address space for debugger.
 */
extern "C"
int
db_read_bytes(vm_offset_t addr, size_t size, char *data)
{
	// check addr for bounds
	for (int i = 0; i < size; i++)
		data[i] = readByte((Word)addr + i, CSeg);

	return 0;
}

/*
 * Write bytes to kernel address space for debugger.
 */
extern "C"
int
db_write_bytes(vm_offset_t addr, size_t size, char *data)
{
	fprintf(stderr, "db_write_bytes\n");
	exit(1);
	return 0;
}

extern "C"
void
panic(const char *fmt, ...)
{
	va_list	listp;
	va_start(listp, fmt);
	vfprintf(stderr, fmt, listp);
	va_end(listp);
	exit(1);
}


extern "C"
bool
X_db_line_at_pc(db_symtab_t *symtab, c_db_sym_t sym, char **file, int *line,
    db_expr_t off)
{
	return (false);
}

extern "C"
bool
X_db_sym_numargs(db_symtab_t *symtab, c_db_sym_t sym, int *nargp, char **argnames)
{
	return false;
}

extern "C"
c_db_sym_t
X_db_lookup(db_symtab_t *symtab, const char *symbol)
{
#if 0
	c_linker_sym_t lsym;
	Elf_Sym *sym;

	if (symtab->private_ == NULL) {
		return ((c_db_sym_t)((!linker_ddb_lookup(symbol, &lsym))
			? lsym : NULL));
	} else {
		sym = (Elf_Sym *)symtab->start;
		while ((char *)sym < symtab->end) {
			if (sym->st_name != 0 &&
			    !strcmp(symtab->private_ + sym->st_name, symbol))
				return ((c_db_sym_t)sym);
			sym++;
		}
	}
#endif
	panic("Not yet lookup");
	return (NULL);
}

extern "C"
c_db_sym_t
X_db_search_symbol(db_symtab_t *symtab, db_addr_t off, db_strategy_t strat,
    db_expr_t *diffp)
{
	struct symtb *sym, *match;
	unsigned long diff;
	int type;

	if (symtab == NULL) {
		*diffp = off;
		return NULL;
	}
	diff = ~0UL;
	match = NULL;
	for (sym = (struct symtb *)symtab->start; (char*)sym < symtab->end; sym++) {
		type = sym->ns_type & N_TYPE;
		if (type != N_TEXT && type != N_DATA && type != N_BSS)
			continue;
		if (off < sym->ns_value)
			continue;
		if ((off - sym->ns_value) > diff)
			continue;
		if ((off - sym->ns_value) < diff) {
			diff = off - sym->ns_value;
			match = sym;
		}
		if (diff == 0)
			break;
	}

	*diffp = (match == NULL) ? off : diff;
	return ((c_db_sym_t)match);
}

extern "C"
void
X_db_symbol_values(db_symtab_t *symtab, c_db_sym_t sym, const char **namep,
    db_expr_t *valp)
{
	struct symtb *asym = (struct symtb *)sym;

	if (valp)
		*valp = asym->ns_value;
	*namep = symtab->private_ + asym->ns_un.ns_strx;
}

extern "C"
bool
X_db_printreloc(db_reloctab_t *relotab, db_expr_t offset, db_strategy_t strategy, db_expr_t loc)
{
	struct relocation_info *relo;

	for (relo = (struct relocation_info *)relotab->start;
	     (char*)relo < relotab->end; relo++) {
		if (relo->r_extern != 0 && relo->r_address == loc) {
			printf("%s", strtab + symtb[relo->r_symbolnum].ns_un.ns_strx);
			if (offset != 0)
				printf("+%#x", offset);
			return true;
		}
	}
	return false;
}

static void *kdb_jmpbufp = NULL;

/*
 * Handle contexts.
 */
extern "C"
void *
edb_jmpbuf(jmp_buf newjb)
{
	void *old;

	old = kdb_jmpbufp;
	kdb_jmpbufp = newjb;
	return (old);
}

extern "C"
void
edb_reenter(void)
{

	if (kdb_jmpbufp == NULL)
		return;

	printf("KDB: reentering\n");
	kdb_backtrace();
	::longjmp(*(jmp_buf *)kdb_jmpbufp, 1);
	/* NOTREACHED */
}

extern "C"
void
edb_reenter_silent(void)
{

	if (kdb_jmpbufp == NULL)
		return;

	::longjmp(*(jmp_buf *)kdb_jmpbufp, 1);
	/* NOTREACHED */
}

extern "C"
void
edb_backtrace(void)
{
	panic("need to implement edb_backtrace");
}

extern "C"
int
db_md_set_watchpoint(db_expr_t addr, db_expr_t size)
{
	panic("md set watchpoint");
	return EINVAL;
}

extern "C"
int
db_md_clr_watchpoint(db_expr_t addr, db_expr_t size)
{
	panic("md clr watchpoint");
	return EINVAL;
}

extern "C"
void
db_md_list_watchpoints(void)
{
	panic("md list watchpoint");
}

extern "C"
int
DB_CALL(db_expr_t call, db_expr_t *rv, int count, db_expr_t args[])
{
	panic("function call into child");
	return 0;
}
