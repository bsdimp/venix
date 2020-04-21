	.comm	_errno,2
	.globl	_errno
	.globl	_semset
_semset:
	push	bp
	mov	bp,sp
	mov	bx,#45
	mov	ax,#0
	mov	dx,*4(bp)
	mov	cx,*6(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
