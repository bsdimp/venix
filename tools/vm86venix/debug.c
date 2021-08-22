#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
bool dodis = true;
bool dosyscall = false;
uint32_t mask = 0x1;

const char* filename;

void error(const char* operation)
{
    debug(dbg_error, "Error %s file %s: %s\n", operation, filename,
        strerror(errno));
    exit(1);
}

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
	if (type != dbg_syscall && type != dbg_load)
		return;
	if (dbg == NULL)
		dbg = fopen("/tmp/venix.dbg", "w");
	f = dbg;
#endif
	if (f == NULL)
		return;
	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
}

/*
 * Printing for ddb
 */
int
db_printf(const char *fmt, ...)
{
	va_list	listp;
	int retval;

	return 0;
	if (dbg == NULL)
		return 0;
	va_start(listp, fmt);
	retval = vfprintf(dbg, fmt, listp);
	va_end(listp);

	return (retval);
}
