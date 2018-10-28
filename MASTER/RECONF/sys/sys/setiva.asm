|
| %M%: version %I% of %G%
|
|	This routine sets up an interrupt vector to be used by venix.
|	The interrupt vector actually points to a 9 byte array in
|	kernel data space that is actually set up by this routine
|	to be a 3 instruction transfer code.  The first instruction
|	saves ax on the stack.  The second instruction moves address
|	of interrupt routine to register ax.  And, the last instruction
|	is actually a long inter-segment jump to trap() routine with
|	the ax register as an argument.
|	This routine takes its second argument, which is the address
|	of interrupt routine, and stuffs it into the transfer code.
|	Then, the address of the transfer code is stuffed into the
|	specified vector space in the low memory.
|
|	setiva(vector, routine, xfercode)
|	int	vector;
|	int	routine();
|	char	xfercode[9];

	.text
	.globl	_Setiva
_Setiva:
	push	bp			| save bp
	mov	bp,sp			| point to arguments
	mov	bx,*4(bp)		| bx = vector
	mov	ax,*6(bp)		| ax = address of interrupt routine
	mov	bp,*8(bp)		| bp = address of 9 char array
	movb	*0(bp),*50		| bp[0] = 0x50	push	ax
	movb	*1(bp),*b8		| bp[1] = 0xb8	mov	ax,routine()
	mov	*2(bp),ax		| bp[2,3] = routine()
	movb	*4(bp),*ea		| bp[4] = 0xea	jmp	trap(),cs
	mov	*5(bp),#_trap		| bp[5,6] = trap()
	mov	*7(bp),cs		| bp[7,8] = code segment
	sub	ax,ax			| clear ax
	mov	es,ax			| point to physical address 0
	seg	es			| override default data segment
	mov	(bx),bp			| stuff address of our code into vector
	seg	es			| override default data segment
	mov	*2(bx),ds		| stuff our data segment into vector
	pop	bp			| restore bp
	ret				| return to caller

	.text
	.globl	_getcs
_getcs:
	mov	ax,cs
	ret

	.text
	.globl	_getss
_getss:
	mov	ax,ss
	ret

	.text
	.globl	_Getds
_Getds:
	mov	ax,ds
	ret

	.text
	.globl	_getes
_getes:
	mov	ax,es
	ret
