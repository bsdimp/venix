/ C library -- lseek

/ error = lseek(file, offset, ptr);
/ long offset
/ long lseek()

.globl	_lseek, _errno

_lseek:
	push	bp
	push	si
	mov	bp,sp
	mov	bx,#19
	mov	ax,*6(bp)	| file
	mov	dx,*8(bp)	| offset (lsw)
	mov	cx,*10(bp)	| offset (msw)
	mov	si,*12(bp)	| ptr
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:
	pop	si
	pop	bp
	ret

