	.data
	.even
_i:
	.zerow	1
	.text
	.globl	_panic
_panic:
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	bp

	mov	_i,*1
L0:
	mov	ax,2(bp)
	cmp	ax,*0
	beq	L1
	cmp	ax,#_etext
	bhis	L1

	push	ax
	mov	ax,_i
	inc	_i
	push	ax
	mov	ax,#LD24
	push	ax
	call	_printf
	add	sp,*6

	mov	ax,0(bp)
	mov	bp,ax
	cmp	ax,sp
	bhis	L0
L1:
	pop	bp

	mov	ax,#LD28
	push	ax
	call	_printf
	pop	cx

	pushf
	pop	ax
	push	ax		| _ps
	push	*2(bp)		| _pc
	mov	ax,#LD29
	push	ax
	call	_printf
	add	sp,*6

	push	es
	push	ds
	push	ss
	push	cs
	mov	ax,#LD30
	push	ax
	call	_printf
	add	sp,*10

	push	dx
	push	cx
	push	bx
	push	ax
	mov	ax,#LD31
	push	ax
	call	_printf
	add	sp,*10

	push	bp
	push	sp
	push	di
	push	si
	mov	ax,#LD32
	push	ax
	call	_printf
	add	sp,*10

	push	*6(bp)
	push	*4(bp)
	call	_ppanic
	add	sp,*4

	xor 	ax,ax
	pop	di
	pop	si
	pop	bp
	ret

	.data
LD24:
	.byte	99,97,108,108,101,114,32,37
	.byte	100,32,112,99,58,32,48,120
	.byte	37,120,10,0
LD28:
	.byte	10,80,65,78,73,67,58,32,99
	.byte	97,108,108,101,114,115,32,114
	.byte	101,103,105,115,116,101,114,115
	.byte	58,10,0
LD29:
	.byte	9,80,67,32,61,32,48,120
	.byte	37,120,9,80,83,32,61,32
	.byte	48,120,37,120,10,0
LD30:
	.byte	9,67,83,32,61,32,48,120
	.byte	37,120,9,83,83,32,61,32
	.byte	48,120,37,120,9,68,83,32
	.byte	61,32,48,120,37,120,9,69
	.byte	83,32,61,32,48,120,37,120
	.byte	10,0
LD31:
	.byte	9,65,88,32,61,32,48,120
	.byte	37,120,9,66,88,32,61,32
	.byte	48,120,37,120,9,67,88,32
	.byte	61,32,48,120,37,120,9,68
	.byte	88,32,61,32,48,120,37,120
	.byte	10,0
LD32:
	.byte	9,83,73,32,61,32,48,120
	.byte	37,120,9,68,73,32,61,32
	.byte	48,120,37,120,9,83,80,32
	.byte	61,32,48,120,37,120,9,66
	.byte	80,32,61,32,48,120,37,120
	.byte	10,0
