	.comm	_errno,2
	.globl	_errno
	.globl	_lseek
_lseek:
	push	bp
	mov	bp,sp
	push	si
	mov	bx,#19
	mov	ax,*4(bp)
	mov	dx,*6(bp)
	mov	cx,*8(bp)
	mov	si,*10(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	si
	pop	bp
	ret
