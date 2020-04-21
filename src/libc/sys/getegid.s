	.comm	_errno,2
	.globl	_errno
	.globl	_getgid
_getgid:
	push	bp
	mov	bp,sp
	mov	bx,#47
	int	0xf1
	mov	ax,dx
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret
