	|
	| This could be generated, were it not for the name...
	|

	.globl	_errno
	.comm	_errno,2
	.globl	__exit
__exit:
	push	bp
	mov	bp,sp
	mov	bx,#1
	mov	ax,*4(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	bp
	ret