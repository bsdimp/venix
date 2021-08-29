	.globl	_errno
	.comm	_errno,2
	.globl	_aiowait
_aiowait:
	push	bp
	mov	bp,sp
	mov	bx,#54			| ioctl
	mov	ax,*4(bp)		| fd
	mov	dh,*97			| 0x9700 ioctl cmd
	sub	dl,dl
	mov	cx,*6(bp)		| arg
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
