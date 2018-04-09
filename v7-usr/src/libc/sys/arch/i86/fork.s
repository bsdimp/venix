/ C library -- fork

/ pid = fork();
/
/ pid == 0 in child process; pid == -1 means error return
/ in child, parents id

.globl	_fork, _errno
_fork:
	push	bp
	mov	bp,sp
	mov	bx,#2
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:
	pop	bp
	ret
	
