/ C library -- wait

/ pid = wait(0);
/   or,
/ pid = wait(&status);
/
/ pid == -1 if error
/ status indicates fate of process, if given
.globl	_wait, _errno
_wait:
	push	bp
	mov	bp,sp
	mov	bx,#7
	int	$f1
	jcxz	2f		| loc 00010
	mov	_errno,cx
1:
	pop	bp
	ret
2:
	mov	bx,*4(bp)
	cmp	bx,*0
	jz	1r
	mov	(bx),dx
	j	1r

