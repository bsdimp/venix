/ C library -- write

/ nwritten = write(file, buffer, count);
/
/ nwritten == -1 means error
.globl	_write, _errno

_write:
	push	bp
	mov	bp,sp
	mov	bx,#4
	mov	ax,*4(bp)
	mov	dx,*6(bp)
	mov	cx,*8(bp)
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:
	pop	bp
	ret
