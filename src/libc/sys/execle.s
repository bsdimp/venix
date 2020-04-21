	.comm	_errno,2
	.globl	_errno
	.comm	_environ,2
	.globl	_environ
	.globl	_execle
_execle:
	push	bp
	mov	bp,sp
	mov	bx,#59
	mov	dx,bp
	add	dx,#6
L000:
	add	bp,*2
	cmp	*-1(bp),*0
	jnz	L000		| loc 0000f
	mov	cx,*0(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
