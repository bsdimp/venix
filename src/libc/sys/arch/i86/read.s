/ C library -- read

/ nread = read(file, buffer, count);
/ nread ==0 means eof; nread == -1 means error
.globl	_read, _errno

_read:
	push	bp
	mov	bp,sp
	mov	bx,#3
	mov	ax,*4(bp)
	mov	dx,*6(bp)
	mov	cx,*8(bp)
	int	$f1
	jcxz	1f
	mov	_errno,cx
1f:
	pop	bp
	ret
