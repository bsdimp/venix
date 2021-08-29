	.globl	_setjmp
	.globl	_longjmp

	.text

_setjmp:
	push	bp
	mov	bp,sp
	mov	dx,*4(bp)
	mov	cx,*2(bp)
	mov	bx,*0(bp)
	mov	bp,dx
	mov	*0(bp),bx
	mov	*2(bp),sp
	mov	*4(bp),cx
	mov	bx,0x0002
	mov	*6(bp),bx
	mov	cx,(bx)
	mov	*8(bp),cx
	mov	cx,*2(bx)
	mov	*10(bp),cx
	mov	ax,#0
	pop	bp
	ret

_longjmp:
	push	bp
	mov	bp,sp
	mov	bx,*4(bp)
	mov	ax,*6(bp)
	or	ax,ax
	jnz	L000
	inc	ax
L000:
	mov	cx,(bx)
	j	L002
L001:
	mov	bp,*0(bp)
	or	bp,bp
	jz	L003
L002:
	cmp	*0(bp),cx
	jnz	L001
	mov	si,*-2(bp)
	mov	di,*-4(bp)
L003:
	mov	cx,cs
	mov	dx,ds
	cmp	cx,dx
	jz	L004
	mov	bp,*6(bx)
	or	bp,bp
	jz	L004
	mov	0x0002,bp
	mov	cx,*8(bx)
	mov	*0(bp),cx
	mov	cx,*10(bx)
	mov	*2(bp),cx
L004:
	mov	bp,(bx)
	mov	sp,*2(bx)
	pop	cx
	pop	cx
	mov	cx,*4(bx)
	push	cx
	ret


