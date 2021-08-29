/ C library -- close

/ error =  close(file);

.globl	_close, _errno

_close:
	push	bp
	mov	bp,sp
	mov	bx,#6
	mov	ax,*4(bp)
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:
	pop	bp
	ret
