/ C library -- unlink

/ error = unlink(string);

.globl	_unlink,
.globl	cerror
.unlink = 10.

_unlink:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.unlink; 0:..
