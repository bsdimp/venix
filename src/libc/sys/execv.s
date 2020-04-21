	.comm	_errno,2
	.globl	_errno
	.comm	_environ,2
	.globl	_environ
	.globl	_execv
_execv:
	push	bp
	mov	bp,sp
	mov	bx,#59
	mov	dx,*6(bp)
	mov	cx,_environ
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
