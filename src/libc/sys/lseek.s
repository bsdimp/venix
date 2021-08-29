	.comm	_errno,2
	.globl	_errno
	.globl	_lseek
	|
	| Normally, we'd be able to generate this and get an exact
	| match. However, the push si and the mov bp,sp are swapped.
	| This makes all the other offsets off by two because of it.
	| It's not right stylistically, but is no faster or slower
	| than the stylistic code. Both produce the same results.
	|
_lseek:
	push	bp
	push	si
	mov	bp,sp
	mov	bx,#19
	mov	ax,*6(bp)
	mov	dx,*8(bp)
	mov	cx,*10(bp)
	mov	si,*12(bp)
	int	0xf1
	jcxz	L001
	mov	_errno,cx
L001:
	pop	si
	pop	bp
	ret
