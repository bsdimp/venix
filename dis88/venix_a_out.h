#include <stdint.h>

typedef int32_t		AOUT_T;

/*
 * Header prepended to each a.out file.
 */
struct exec {
int16_t		a_magic;	/* magic number */
uint16_t	a_stack;	/* size of stack if Z type, 0 otherwise */
int32_t		a_text;		/* size of text segment */
int32_t		a_data;		/* size of initialized data */
int32_t		a_bss;		/* size of uninitialized data */
int32_t		a_syms;		/* size of symbol table */
int32_t		a_entry;	/* entry point */
int32_t		a_trsize;	/* size of text relocation */
int32_t		a_drsize;	/* size of data relocation */
};

#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0411		/* read-only text (seperate I&D) */

#define SYMNMLEN	8	/* size of symbol name	*/

/*
 * Macros which take exec structures as arguments and tell whether
 * the file has a reasonable magic number or offsets to text|symbols|strings.
 */
#define	N_BADMAG(x) \
	(int32_t)(((x).a_magic)!=OMAGIC && ((x).a_magic)!=NMAGIC)
#define	N_TXTOFF(x) \
	(int32_t)sizeof(struct exec)
#define N_SYMOFF(x) \
	(int32_t)(N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize + (x).a_drsize)
#define	N_STROFF(x) \
	(int32_t)(N_SYMOFF(x) + (x).a_syms)

/*
 * Format of a relocation datum.
 */
struct relocation_info {
int32_t		r_address;	/* address which is relocated */
int16_t		r_symbolnum;	/* local symbol ordinal */
int16_t		r_pcrel:1, 	/* was relocated pc relative already */
		r_length:2,	/* 0=byte, 1=word, 2=long */
		r_extern:1,	/* does not include value of sym referenced */
		:12;
};

/* Format of the old symbol table entry. This is here for compatability.
 * The nlist subroutine takes an old symbol table format as its argument
 * and it knows how to read the format actually stored in the file.
 */
struct	nlist {
	char	n_name[SYMNMLEN];	/* symbol name */
	int16_t	n_type;			/* type  */
	int32_t	n_value;		/* value */
};

/*
 * Format of a symbol table entry as it really is in the a.out file.
 */
struct	symtb {
	union {
//		char	*ns_name;	/* for use when in-core */
		uint16_t ns_strx;	/* index into file string table */
	} ns_un;
	char	ns_type;	/* type flag, i.e. N_TEXT etc; see below */
	char	ns_other;	/* unused */
	int16_t	ns_desc;	/* see <stab.h> */
	int32_t	ns_value;	/* value of this symbol */
} __packed;

#define	ns_hash	ns_desc		/* used internally by ld */

/*
 * Simple values for n_type or ns_type.
 */
#define	N_UNDF	0x0		/* undefined */
#define	N_ABS	0x2		/* absolute */
#define	N_TEXT	0x4		/* text */
#define	N_DATA	0x6		/* data */
#define	N_BSS	0x8		/* bss */
#define	N_COMM	0x12		/* common (internal to ld) */
#define	N_FN	0x1f		/* file name symbol */

#define	N_EXT	01		/* external bit, or'ed in */
#define	N_TYPE	0x1e		/* mask for all the type bits */

#define	N_STAB	0xe0

/*
 * Format for namelist values.
 */
#define	N_FORMAT	"%08lx"
