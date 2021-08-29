	.comm	_errno,2
	.globl	_errno
	.globl	_pause
_pause:
	push	bp
	mov	bp,sp
	mov	bx,#29
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
