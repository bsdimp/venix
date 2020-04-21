	.comm	_errno,2
	.globl	_errno
	.globl	_ptrace
_ptrace:
	push	bp
	mov	bp,sp
	push	si
	mov	bx,#26
	mov	si,*4(bp)		| si/ax backwards
	mov	dx,*6(bp)
	mov	cx,*8(bp)
	mov	ax,*10(bp)		| ax/si backwards
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	si
	pop	bp
	ret
