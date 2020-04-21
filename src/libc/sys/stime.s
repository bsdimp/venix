	.globl	_errno
	.comm	_errno,2
	.globl	_stime

_stime:
	push	bp
	mov	bp,sp
	mov	bx,#25
	mov	cx,*4(bp)
	mov	bp,cx
	mov	ax,*0(bp)
	mov	dx,*2(bp)
	int	0xf1
	jcxz	L001		| loc 00019
	mov	_errno,cx
L001:
	pop	bp
	ret
