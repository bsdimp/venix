	.globl	_errno
	.comm	_errno,2
	.globl	_wait
_wait:
	push	bp
	mov	bp,sp
	mov	bx,#7
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L000:
	pop	bp
	ret
L001:
	mov	bx,*4(bp)
	cmp	bx,*0
	jz	L000
	mov	(bx),dx
	j	L000
