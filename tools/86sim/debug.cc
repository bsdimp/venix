#include <stdarg.h>
#include <stdio.h>
#include "debug.hh"
bool dodis = true;
bool dosyscall = false;
uint32_t mask = 0x1;

FILE *dbg = NULL;
void debug(enum dbg type, const char *fmt, ...)
{
	va_list ap;
	FILE *f = NULL;

#if 0
	f = dbg;
	if (dbg == NULL && type == dbg_error)
		f = stderr;
	if (f == NULL) // || type != dbg_load)
		return;
#else
	if (type == dbg_syscall)
		f = stderr;
#endif
	if (f == NULL)
		return;
	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
}

/*
 * Printing for ddb
 */
extern "C"
int
db_printf(const char *fmt, ...)
{
	va_list	listp;
	int retval;

	if (dbg == NULL)
		return 0;
	va_start(listp, fmt);
	retval = vfprintf(dbg, fmt, listp);
	va_end(listp);

	return (retval);
}
