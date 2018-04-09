/ C library -- open

/ file = open(string, mode)
/ file == -1 means error

.globl	_open, _errno

_open:
	push	bp
	mov	bp,sp
	mov	bx,#5
	mov	ax,*4(bp)	| string
	mov	dx,*6(bp)	| mode
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:
	pop	bp
	ret
