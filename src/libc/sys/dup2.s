	.comm	_errno,2
	.globl	_errno
	.globl	_dup2
_dup2:
	push	bp
	mov	bp,sp
	mov	bx,#41
	mov	ax,*4(bp)
	mov	dx,*6(bp)
	or	ax,#64
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
