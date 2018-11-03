#ifndef _DDB_DB_RELOC_H
#define _DDB_DB_RELOC_H

/*
 * This module can handle multiple symbol tables
 */
typedef struct {
	char		*name;		/* symtab name */
	char		*start;		/* symtab location */
	char		*end;
	char		*private_;	/* optional machdep pointer */
} db_reloctab_t;

void	db_add_reloc_table(char *, char *, char *, char *);
bool	db_printreloc(db_expr_t, db_strategy_t, db_expr_t);
bool	X_db_printreloc(db_reloctab_t *, db_expr_t, db_strategy_t, db_expr_t);

#endif /* _DDB_DB_RELOC_H */
