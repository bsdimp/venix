#include <stdio.h>
#include <stdlib.h>

#include <sys/param.h>

#include <ddb/ddb.h>
#include <ddb/db_sym.h>
#include <ddb/db_reloc.h>

#ifndef MAXNORELOCTABS
#define  MAXNORELOCTABS	2	/* text, data */
#endif
static db_reloctab_t	db_reloctabs[MAXNORELOCTABS] = { { 0, }, };
static int		db_nreloctab = 0;

void
db_add_reloc_table(char *start, char *end, char *name, char *extra)
{
	if (db_nreloctab >= MAXNORELOCTABS) {
		printf ("No slots left for %s relocation table", name);
		panic ("db_sym.c: db_add_reloc_table");
	}

	db_reloctabs[db_nreloctab].start = start;
	db_reloctabs[db_nreloctab].end = end;
	db_reloctabs[db_nreloctab].name = name;
	db_reloctabs[db_nreloctab].private_ = extra;
	db_nreloctab++;
}

bool
db_printreloc(db_expr_t offset, db_strategy_t strategy, db_expr_t loc)
{
	int i;

	for (i = 0; i < db_nreloctab; i++)
		if (X_db_printreloc(&db_reloctabs[i], offset, strategy, loc))
			return true;

	return false;
}

