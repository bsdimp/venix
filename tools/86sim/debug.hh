enum dbg {
	dbg_load,
	dbg_emul,
	dbg_syscall,
	dbg_error,
};
void debug(enum dbg type, const char *fmt, ...);
extern bool dodis;
extern bool dosyscall;
typedef uint64_t db_addr_t;
extern "C" db_addr_t db_disasm(db_addr_t, bool);
