	.globl	_errno
	.comm	_errno,2
	.globl	_end

	.globl	_brk
_brk:
	push	bp
	mov	bp,sp
	mov	bx,#17
	mov	ax,*4(bp)
	int	0xf1
	jcxz	L000
	mov	_errno,cx
	j	L001
L000:
	mov	bx,*4(bp)
	mov	L005,bx
L001:
	pop	bp
	ret

	.globl	_sbrk
_sbrk:
	push	bp
	mov	bp,sp
	mov	bx,#17
	mov	ax,*4(bp)
	add	ax,L005
	int	0xf1
	jcxz	L002
	mov	_errno,cx
	j	L003
L002:
	mov	bx,L005
	mov	ax,bx
	add	bx,*4(bp)
	mov	L005,bx
L003:
	pop	bp
	ret

	.data
L005:
	.word	_end			| Current break

