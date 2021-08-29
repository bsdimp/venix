	.comm	_errno,2
	.globl	_errno
	.globl	_ioctl
_ioctl:
	push	bp
	mov	bp,sp
	mov	bx,#54
	mov	ax,*4(bp)
	mov	dx,*6(bp)
	mov	cx,*8(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
