	.globl	_errno
	.comm	_errno,2
	.globl	_pipe

_pipe:
	push	bp
	mov	bp,sp
	mov	bx,#42
	int	0xf1
	jcxz	L002
	mov	_errno,cx
L001:
	pop	bp
	ret
L002:
	mov	cx,*4(bp)		| pipe is passed a pointer to 2-deep array
	mov	bp,cx
	mov	*0(bp),ax
	mov	*2(bp),dx
	mov	ax,#0
	jmp	L001
