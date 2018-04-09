Venix/86 ABI

This document tries to describe the Venix86 ABI. It is reverse
engineered from a copy of Venix86R 2.0 and Venix86 2.1. It also uses
the ancient unix 7th edition sources (aka V7) to fill in some of the
blanks.

# Venix Origins

Venix is a v7-based port to the 8088, including the Dec Rainbow and
IBM-PC, XT and AT. A number of changes have been made to the original
v7 sources to accomodate the 8088's quirks.

## Binary Format

Venix, like v7 Unux, uses the old-school a.out format. There's
a header on each file that looks like this:
```C
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
```
After the header comes the text section, the data section, the text
relcoation data, the data relocation data and finally the symbols. After
the symbol table, the remainder of the file is the classic string table.
All sizes are in bytes, as far as we know (the code in a.out.h certainly
implies that).

### Binary Types

There's two kinds of binaries as well:
```C
#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0411		/* read-only text (seperate I&D) */
```

#### OMAGIC

Tagged as the 'old impure' format. For fully linked binaries, this is
the classic 'tiny' model.  64k segment, cs = ds = es = ss. Text is
loaded at offset 0, data follows immediately afterwards (though
there's a requirement it start on a paragraph boundary, it seems). bss
is after that, though it's of course not stored in the file. Stack grows
down from the top of the segment. There's some hints that ss may be its own
segment, though that's not been confirmed.

#### NMAGIC

The PDP-11 called this separate I&D. It appears, though it hasn't been
confirmed, that this model is similar to the 'small' model that other
compilers would later use. There's two segments (though maybe three, for
the stack): cs for code, es and ds are the same and ss may be different.

I think these are created with -z.

### Relocation Information

This is fairly typical reloation information:

```C
struct relocation_info {
	int32_t		r_address;	/* address which is relocated */
	int16_t		r_symbolnum;	/* local symbol ordinal */
	int16_t		r_pcrel:1, 	/* was relocated pc relative already */
			r_length:2,	/* 0=byte, 1=word, 2=long */
			r_extern:1,	/* does not include value of sym referenced */
			:12;
};
```
No additional details are know at this time.

### Symbols
Although the classic struct nlist is defined here, it appears to use
the newer .stabs format from Berkeley to get the longer symbol names.

```
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
```
ns_name is commented out here because pointers are no longer 2 bytes
long :). It's constructs like this which make it hard to have a more
complicated memory model that supports far pointers.

ns_type is a define:
```C
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
```
Debugging symbols, such as they are, can be found in stab.h. When the
ns_type is N_STAB, the ns_desc has more info:

```C
#define	N_GSYM	0x20		/* global symbol: name,,0,type,0 */
#define	N_FNAME	0x22		/* procedure name (f77 kludge): name,,0 */
#define	N_FUN	0x24		/* procedure: name,,0,linenumber,address */
#define	N_STSYM	0x26		/* static symbol: name,,0,type,address */
#define	N_LCSYM	0x28		/* .lcomm symbol: name,,0,type,address */
#define	N_RSYM	0x40		/* register sym: name,,0,type,register */
#define	N_SLINE	0x44		/* src line: 0,,0,linenumber,address */
#define	N_SSYM	0x60		/* structure elt: name,,0,type,struct_offset */
#define	N_SO	0x64		/* source file name: name,,0,0,address */
#define	N_LSYM	0x80		/* local sym: name,,0,type,offset */
#define	N_SOL	0x84		/* #included file name: name,,0,0,address */
#define	N_PSYM	0xa0		/* parameter: name,,0,type,offset */
#define	N_ENTRY	0xa4		/* alternate entry: name,linenumber,address */
#define	N_LBRAC	0xc0		/* left bracket: 0,,0,nesting level,address */
#define	N_RBRAC	0xe0		/* right bracket: 0,,0,nesting level,address */
#define	N_BCOMM	0xe2		/* begin common: name,, */
#define	N_ECOMM	0xe4		/* end common: name,, */
#define	N_ECOML	0xe8		/* end common (local name): ,,address */
#define	N_LENG	0xfe		/* second stab entry with length information */
```
though I've found no symbols in the binaries I have available to me.
These must have one of the N_STAB bits on, and are subject to
relocation according to the masks in <a.out.h>. The name,, stuff is how
the symbol appears in the symbol string table.

## Interupt Usage

### $F1 -- System Calls

System Call number is in the `bx` register. The system calls appear to
be the same as in pdp-11 v7 unix, but that's not been completely
confirmed.

Venix Call number | function
----------------- | --------
1 | exit
2 | fork
3 | read
4 | write
5 | open
8 | creat
19 | lseek

v7 pdp11 call no | function
------------- | --------
indir   | 0
exit    | 1
fork    | 2
read    | 3
write   | 4
open    | 5
close   | 6
wait    | 7
creat   | 8
link    | 9
unlink  | 10
exec    | 11
chdir   | 12
time    | 13
mknod   | 14
chmod   | 15
chown   | 16
break   | 17
stat    | 18
lseek   | 19
getpid  | 20
mount   | 21
umount  | 22
setuid  | 23
getuid  | 24
stime   | 25
ptrace  | 26
alarm   | 27
fstat   | 28
pause   | 29
utime   | 30
smdate  | 30
stty    | 31
gtty    | 32
access  | 33
nice    | 34
sleep   | 35
sync    | 36
kill    | 37
csw     | 38
setpgrp | 39
dup     | 41
pipe    | 42
times   | 43
profil  | 44
setgid  | 46
getgid  | 47
signal  | 48
acct    | 51
phys    | 52
lock    | 53
ioctl   | 54
reboot  | 55
mpx     | 56
setinf  | 59
umask   | 60
getinf  | 60

Argument | Register
-------- | --------
call no | bx
1 | ax
2 | dx
3 | cx
4 | si

  long args take up two slots, least significant first (little endian machine).

errno is returned from the kernel in cx.

It's unknown how long values are returned, but it's believed to be the
same as the rest of the ABI: least significant word in ax, most
significant in dx (which also mirrors the system calls).

### $F2 -- EMT

This is 'emt' in the low.s. The comment says its a hold over from
pdp11. Looks like it generates SIGEMT. In the generated code, we often
see sequences like:
```assembler
    cmp     sp,*127
    ja      1f
    int     $f2
1f:
```
where various sanity checks are done on sp. Some programs just have
this in main, while others have it more extensively. See the EMT
section in the kernel reference.

### $F3 -- Abort

Seems unused, but low.s lists it as 'abort' which trap.o translates to
tsignal like emt.

### $F4 -- Floating Point

Floating point emulation. All 8087 opcodes are preceeded by this sequence:
```assembler
    int     $f4
    wait
```
as in the following:
```assembler
    int     $f4
    wait
    fld     (si)
```

All programs start with:
```assembler
    int     $f4
    wait
    .byte   0xd9            | esc   $36f4
    .byte   0x2e
    .byte   0xf4            | These two byts are different for every
    .byte   0x36            | program, not sure why
```
Note that on a 8087, bytes 0xd9 0x2e decode to FLDCW to load the next two bytes into code
word, but the mask of valid bits for the CW is $0f3f, so the above
number is nonsense. It's also different for every progam. It's not yet
clear what the extra bits mean, or what it's encoding. The 'base'
sequence in crt0.o is
```assembler
    int     $f4
    wait
   .byte   0xd9            | esc   '+56
   .byte   0x2e
   .byte   0x38
   .byte   0x00
```
Here $0038 does make some sense. It decodes as
* Round to nearest even
* 24-bits of precision
* ignore the Precision, Underflow and Overflow exceptions.
which seems like a reasonable default. It may be some kind of signal
or hint to the floating point emulation library inside the Venix
kernel to do things or not do things.

### $Fx -- Others reserved for redirect

For the xt version $fd is reseved for video redirect, $fe is reserved
for keyboard redirect, and $ff is reserved for timer interrupt.

The Rainbow version has not been examined in detail yet.

low.s stores these values in a table at the end of low.s, and then it
gets overwritten with the stack for the kernel after the kenrel is
relocated, but more about that in the kernel document.

## Data types

The data model is IP16.

Type | Size
---- | ----
char | 1
short | 2
int | 2
long | 4
char * | 2
float | unk
double | unk

Arguments are pushed onto the stack. Return value is in ax (and we
think dx for long values, flaot returnas are not known at this
time. Call pushes and pops args.  Callee saves and restores bp, si and
di.  Functions that call setjmp have to hidden local variables at
-2(bp) and -4(bp) that are si and di respectively.

By convention, globals have _ prepended to them. Symbols without _ are
used to implement things by the compiler, or sometimes the assembler
such that they can't be called by C routines.

The C compiler generates typical stack frames in the preamble
```Assembler
_func
    push    bp
    mov     bp,sp
```
And cleans up in the postamble
```Assembler
    pop     bp
    ret
```

The assembler will pack functions tightly, however, the linker appears to pad to the next
even boundary when stitching files together, so often times you will see
```Assembler
    ret
    .byte 0
_newfunc:
```
which messes with the disassembly.

The setjmp jmp_buf is just 6 ints long.

offset | use
------ | ---
0 | bp of caller
2 | sp + 2 (after the push bp)
4 | ip of return point
6 | 2 (maybe the offset of fp_ctx?)
8 | value of fp_ctx[0]?
10 | value of fp_ctx[1]?

```Assembler
_setjmp:
	push	bp
	mov	bp,sp
	mov	dx,*4(bp)	| jum_buf env
	mov	cx,*2(bp)	| IP of caller
	mov	bx,*0(bp)	| old bp
	mov	bp,dx
	mov	*0(bp),bx	| env[0] = bp
	mov	*2(bp),sp	| env[1] = sp + 2
	mov	*4(bp),cx	| env[2] = IP of caller
	mov	bx,$0002		| 2? Not sure about 2, offset of current FP context?
	mov	*6(bp),bx	| env[3] = 2 ??? FP ctx addr?
	mov	cx,(bx)		| 
	mov	*8(bp),cx	| env[4] = cur ctx
	mov	cx,*2(bx)	|
	mov	*10(bp),cx	| env[5] = cur ctx 2nd part 
	mov	ax,#0		| return 0 the first time
	pop	bp
	ret
```

But longjmp is more complicated
```Assembler
_longjmp:				| longjmp(jmp_buf env, int val)
	push	bp
	mov	bp,sp
	mov	bx,*4(bp)		| bx = env
	mov	ax,*6(bp)		| ax = val
	or	ax,ax			| if (ax == 0) ax++
	jnz	L000
	inc	ax
L000:
	mov	cx,(bx)			| cx = env[0] (bp of caller?)
	j	L002

|
| Try to find the caller's frame by assuming it's stored bp is unique...
|
L001:
	mov	bp,*0(bp)		| bp is different, walk up the stack one more
	or	bp,bp
	jz	L003			| 0 == no calling frame found, stop loop as failsafe
L002:
	cmp	*0(bp),cx		| Is the old bp == the bp on the stack?
	jnz	L001
	mov	si,*-2(bp)		| Found caller's bp, snag saved si and di from
	mov	di,*-4(bp)		| the local stack frame (compiler assist? setjmp didn't save)
L003:
	mov	cx,cs
	mov	dx,ds
	cmp	cx,dx
	jz	L004			| ds == cs, skip a bit
	mov	bp,*6(bx)		| bp = env[3]
	or	bp,bp
	jz	L004
	mov	$0002,bp		| if (bp != 0) { *(int *)2 = bp ??? <-- not sure about this...
	mov	cx,*8(bx)		| restore ctx[0] and 1 from env }
	mov	*0(bp),cx
	mov	cx,*10(bx)
	mov	*2(bp),cx
L004:
	mov	bp,(bx)			| bp = env[0]
	mov	sp,*2(bx)		| sp = env[1]
	pop	cx			| pop push of bp for setenv
	pop	cx			| pop IP return from setenv
	mov	cx,*4(bx)		| grab IP from env[2]
	push	cx			| and arrange to go there...
	ret
```
