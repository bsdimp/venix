	.comm	_errno,2
	.globl	_errno
	.globl	_fork
_fork:
	push	bp
	mov	bp,sp
	mov	bx,#2
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
