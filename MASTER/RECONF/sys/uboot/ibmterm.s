| Terminal driver for IBM bios.		7/84

	.text
	.globl	getchar, getchk, putchar
getchar:			| get keyboard character into AL
	xor	ax,ax
L000:	int	0x16
	ret

getchk:				| check for input; Z=0 if true
	movb	ah,*1
	br	L000

putchar:			| print character from AL
	push	bp
	movb	ah,*14
	xor	bx,bx
	int	0x10
	pop	bp
	ret
