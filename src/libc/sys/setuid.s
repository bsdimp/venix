	.comm	_errno,2
	.globl	_errno
	.globl	_setuid
_setuid:
	push	bp
	mov	bp,sp
	mov	bx,#23
	mov	ax,*4(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
