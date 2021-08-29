	| 
	| Exit with cleanup, trivial implementation. _exit won't return so the
	| last two lines are likely unneeded, but are there in case some super
	| weird error happens. _exit is defined as void in modern systems.
	|

	.globl	_exit,__cleanup,__exit
_exit:
	push	bp
	mov	bp,sp
	call	__cleanup
	call	__exit
	pop	bp
	ret
