	.comm	_errno,2
	.globl	_errno
	.globl	_locking
	|
	| clearly this was copied from lseek since it has the same bug.
	|
_locking:
	push	bp
	push	si
	mov	bp,sp
	mov	bx,#64
	mov	ax,*6(bp)
	mov	dx,*10(bp)
	mov	cx,*12(bp)
	mov	si,*8(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	si
	pop	bp
	ret
