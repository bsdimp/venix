	.text
	.globl _getcs
_getcs:	push cs
	pop ax
	ret
	.globl _getds
_getds:	push ds
	pop ax
	ret
	.globl _getes
_getes:	push es
	pop ax
	ret
	.globl _getss
_getss:	push ss
	pop ax
	ret
