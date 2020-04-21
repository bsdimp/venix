	.comm	_errno,2
	.globl	_errno
	.comm	_environ,2
	.globl	_environ
	.globl	_execl
_execl:
	push	bp
	mov	bp,sp
	mov	bx,#59
	mov	ax,*4(bp)
	mov	cx,_environ
	mov	dx,bp
	add	dx,#6
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
