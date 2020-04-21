	.comm	_errno,2
	.globl	_errno
	.globl	_semtset
_semtset:
	push	bp
	mov	bp,sp
	mov	bx,#45
	mov	ax,#3
	mov	dx,*4(bp)
	mov	cx,*6(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
