	.comm	_errno,2
	.globl	_errno
	.globl	_semtest
_semtest:
	push	bp
	mov	bp,sp
	mov	bx,#45
	mov	ax,#2
	mov	dx,*4(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
