/ C library -- creat

/ file = creat(string, mode);
/ file == -1 if error

.globl	_creat, _errno

_creat:
	push	bp
	mov	bp,sp
	mov	bx,#8
	mov	ax,*4(bp)	| string
	mov	dx,*6(bp)	| mode
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:
	pop	bp
	ret
