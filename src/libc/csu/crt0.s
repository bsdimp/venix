	| C runtime startup
	| Reconstructed from v7 sources and disassembled crt0.o

	.globl	start
	.globl	_environ,_main,_exit

	.text

start:
	|
	| Initialize the FPU. int $f4 is the vector that Venix uses
	| to run real FP code on machines w/o 8087 hardware.
	|
	int	0xf4
	fldcw	L001
	mov	bp,sp
	mov	si,bp

	|
	| Find where envp starts. Save it globally as well as passing it to
	| main.
	|
L000:
	add	si,*2
	cmp	(si),*0
	jnz	L000
	add	si,*2
	push	si
	mov	_environ,si

	|
	| Compute where argv starts on the stack
	|
	mov	ax,bp
	add	ax,#2
	push	ax

	|
	| Push argc here
	|
	mov	ax,*0(bp)
	push	ax
	call	_main
	add	sp,*6
	push	ax
	call	_exit		| Libc's exit
	pop	cx
	mov	bx,#1		| and then try the system call if that didn't work.
	int	0xf1

	.data
	.byte	0,0		| v7 has these two bytes here for a NULL pointer...
L001:
	.word	0xfbf		| 8087 control word

	.bss
	.comm	_environ, 2

