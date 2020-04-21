	.globl	_errno
	.comm	_errno,2
	.globl	_time

_time:
	push	bp
	mov	bp,sp
	mov	bx,#13
	int	0xf1
	jcxz	L001
	mov	_errno,cx
	j	L002
L001:
	mov	bx,*4(bp)
	or	bx,bx
	jz	L002
	mov	(bx),ax
	mov	*2(bx),dx
L002:
	pop	bp
	ret
