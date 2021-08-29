	.globl	_abort
	.text
_abort:
	push	bp
	mov	bp,sp
	int	0xf3
	j	.
