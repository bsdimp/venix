_f_add:		push	bp
_f_add+1:	mov	bp,sp
_f_add+3:	push	si
_f_add+4:	push	di
_f_add+5:	sub	sp,#10
_f_add+9:	mov	di,#4(bp)
_f_add+d:	mov	si,#6(bp)
_f_add+11:	cmp	*2(si),*0
_f_add+15:	jne	_f_add+1a
_f_add+17:	jmp	_f_add+d8
_f_add+1a:	cmp	*2(di),*0
_f_add+1e:	jne	_f_add+33
_f_add+20:	push	si
_f_add+21:	lea	bx,(di)
_f_add+23:	mov	si,si
_f_add+25:	mov	cx,#6
_f_add+28:	lodsw
_f_add+29:	mov	(bx),ax
_f_add+2b:	inc	bx
_f_add+2c:	inc	bx
_f_add+2d:	loop	_f_add+28
_f_add+2f:	pop	si
_f_add+30:	jmp	_f_add+d8
_f_add+33:	mov	ax,*2(si)
_f_add+36:	cmp	ax,*2(di)
_f_add+39:	jle	_f_add+6f
_f_add+3b:	push	si
_f_add+3c:	lea	bx,*-16(bp)
_f_add+3f:	mov	si,di
_f_add+41:	mov	cx,#6
_f_add+44:	lodsw
_f_add+45:	mov	(bx),ax
_f_add+47:	inc	bx
_f_add+48:	inc	bx
_f_add+49:	loop	_f_add+44
_f_add+4b:	pop	si
_f_add+4c:	push	si
_f_add+4d:	lea	bx,(di)
_f_add+4f:	mov	si,si
_f_add+51:	mov	cx,#6
_f_add+54:	lodsw
_f_add+55:	mov	(bx),ax
_f_add+57:	inc	bx
_f_add+58:	inc	bx
_f_add+59:	loop	_f_add+54
_f_add+5b:	pop	si
_f_add+5c:	lea	ax,*-16(bp)
_f_add+5f:	push	si
_f_add+60:	lea	bx,(si)
_f_add+62:	mov	si,ax
_f_add+64:	mov	cx,#6
_f_add+67:	lodsw
_f_add+68:	mov	(bx),ax
_f_add+6a:	inc	bx
_f_add+6b:	inc	bx
_f_add+6c:	loop	_f_add+67
_f_add+6e:	pop	si
_f_add+6f:	mov	ax,*2(di)
_f_add+72:	sub	ax,*2(si)
_f_add+75:	mov	cx,ax
_f_add+77:	movb	*-19(bp),cl
_f_add+7a:	cmpb	*-19(bp),*3e
_f_add+7e:	jle	_f_add+82
_f_add+80:	j	_f_add+d8
_f_add+82:	mov	ax,(di)
_f_add+84:	cmp	ax,(si)
_f_add+86:	je	_f_add+8d
_f_add+88:	push	si
_f_add+89:	call	__neg
_f_add+8c:	pop	cx
_f_add+8d:	decb	*-19(bp)
_f_add+90:	movb	al,*-19(bp)
_f_add+93:	cbw
_f_add+94:	addb	al,*1
_f_add+96:	je	_f_add+a6
_f_add+98:	sar	*10(si)
_f_add+9b:	rcr	*8(si)
_f_add+9e:	rcr	*6(si)
_f_add+a1:	rcr	*4(si)
_f_add+a4:	j	_f_add+8d
_f_add+a6:	mov	ax,*4(si)
_f_add+a9:	add	*4(di),ax
_f_add+ac:	mov	ax,*6(si)
_f_add+af:	adc	*6(di),ax
_f_add+b2:	mov	ax,*8(si)
_f_add+b5:	adc	*8(di),ax
_f_add+b8:	mov	ax,*10(si)
_f_add+bb:	adc	*10(di),ax
_f_add+be:	mov	ax,(di)
_f_add+c0:	cmp	ax,(si)
_f_add+c2:	je	_f_add+d3
_f_add+c4:	cmpb	*11(di),*0
_f_add+c8:	jnl	_f_add+d3
_f_add+ca:	push	di
_f_add+cb:	call	__neg
_f_add+ce:	pop	cx
_f_add+cf:	mov	ax,(si)
_f_add+d1:	mov	(di),ax
_f_add+d3:	push	di
_f_add+d4:	call	_normali
_f_add+d7:	pop	cx
_f_add+d8:	lea	sp,*-4(bp)
_f_add+db:	pop	di
_f_add+dc:	pop	si
_f_add+dd:	pop	bp
_f_add+de:	ret
_f_sub:
_f_sub:		push	bp
_f_sub+1:	mov	bp,sp
_f_sub+3:	push	si
_f_sub+4:	push	di
_f_sub+5:	sub	sp,#0
_f_sub+9:	mov	di,#4(bp)
_f_sub+d:	mov	si,#6(bp)
_f_sub+11:	mov	ax,(si)
_f_sub+13:	not	ax
_f_sub+15:	mov	(si),ax
_f_sub+17:	push	si
_f_sub+18:	push	di
_f_sub+19:	call	0
_f_sub+1c:	add	sp,*4
_f_sub+1f:	lea	sp,*-4(bp)
_f_sub+22:	pop	di
_f_sub+23:	pop	si
_f_sub+24:	pop	bp
_f_sub+25:	ret
_normali:
_normali:	push	bp
_normali+1:	mov	bp,sp
_normali+3:	push	si
_normali+4:	push	di
_normali+5:	sub	sp,#2
_normali+9:	mov	di,#4(bp)
_normali+d:	mov	si,*4(di)
_normali+10:	or	si,*6(di)
_normali+13:	or	si,*8(di)
_normali+16:	or	si,*10(di)
_normali+19:	jne	_normali+26
_normali+1b:	mov	(di),#0
_normali+1f:	mov	*2(di),#0
_normali+24:	j	_normali+68
_normali+26:	movb	al,*11(di)
_normali+29:	cbw
_normali+2a:	test	ax,#80
_normali+2d:	je	_normali+37
_normali+2f:	push	di
_normali+30:	call	0
_normali+33:	pop	cx
_normali+34:	inc	*2(di)
_normali+37:	movb	al,*11(di)
_normali+3a:	cbw
_normali+3b:	test	ax,#40
_normali+3e:	jne	_normali+51
_normali+40:	shl	*4(di)
_normali+43:	rcl	*6(di)
_normali+46:	rcl	*8(di)
_normali+49:	rcl	*10(di)
_normali+4c:	dec	*2(di)
_normali+4f:	j	_normali+37
_normali+51:	cmp	*2(di),#3fff
_normali+56:	jnle	_normali+5e
_normali+58:	cmp	*2(di),*1
_normali+5c:	jnl	_normali+68
_normali+5e:	mov	ax,#8
_normali+61:	push	ax
_normali+62:	call	0
_normali+65:	pop	cx
_normali+66:	j	_normali+68
_normali+68:	lea	sp,*-4(bp)
_normali+6b:	pop	di
_normali+6c:	pop	si
_normali+6d:	pop	bp
_normali+6e:	ret
_f_mult:
_f_mult:	push	bp
_f_mult+1:	mov	bp,sp
_f_mult+3:	push	si
_f_mult+4:	push	di
_f_mult+5:	sub	sp,#0
_f_mult+9:	mov	di,#4(bp)
_f_mult+d:	mov	si,#6(bp)
_f_mult+11:	mov	ax,(si)
_f_mult+13:	xor	(di),ax
_f_mult+15:	mov	ax,*2(si)
_f_mult+18:	sub	ax,#2001
_f_mult+1b:	add	*2(di),ax
_f_mult+1e:	push	si
_f_mult+1f:	push	di
_f_mult+20:	call	_mult_ac
_f_mult+23:	add	sp,*4
_f_mult+26:	push	di
_f_mult+27:	call	_normali
_f_mult+2a:	pop	cx
_f_mult+2b:	lea	sp,*-4(bp)
_f_mult+2e:	pop	di
_f_mult+2f:	pop	si
_f_mult+30:	pop	bp
_f_mult+31:	ret
_f_div:
_f_div:		push	bp
_f_div+1:	mov	bp,sp
_f_div+3:	push	si
_f_div+4:	push	di
_f_div+5:	sub	sp,#0
_f_div+9:	mov	di,#4(bp)
_f_div+d:	mov	si,#6(bp)
_f_div+11:	cmp	*2(si),*0
_f_div+15:	jne	_f_div+21
_f_div+17:	mov	ax,#4
_f_div+1a:	push	ax
_f_div+1b:	call	0
_f_div+1e:	pop	cx
_f_div+1f:	j	_f_div+3b
_f_div+21:	mov	ax,(si)
_f_div+23:	xor	(di),ax
_f_div+25:	mov	ax,*2(si)
_f_div+28:	sub	ax,#2000
_f_div+2b:	sub	*2(di),ax
_f_div+2e:	push	si
_f_div+2f:	push	di
_f_div+30:	call	_div_ac
_f_div+33:	add	sp,*4
_f_div+36:	push	di
_f_div+37:	call	_normali
_f_div+3a:	pop	cx
_f_div+3b:	lea	sp,*-4(bp)
_f_div+3e:	pop	di
_f_div+3f:	pop	si
_f_div+40:	pop	bp
_f_div+41:	ret
__neg:
__neg:		push	bp
__neg+1:	mov	bp,sp
__neg+3:	push	si
__neg+4:	push	di
__neg+5:	sub	sp,#0
__neg+9:	mov	di,#4(bp)
__neg+d:	not	*4(di)
__neg+10:	not	*6(di)
__neg+13:	not	*8(di)
__neg+16:	not	*10(di)
__neg+19:	add	*4(di),*1
__neg+1d:	adc	*6(di),*0
__neg+21:	adc	*8(di),*0
__neg+25:	adc	*10(di),*0
__neg+29:	lea	sp,*-4(bp)
__neg+2c:	pop	di
__neg+2d:	pop	si
__neg+2e:	pop	bp
__neg+2f:	ret
_bump:
_bump:		push	bp
_bump+1:	mov	bp,sp
_bump+3:	push	si
_bump+4:	push	di
_bump+5:	sub	sp,#0
_bump+9:	mov	di,#4(bp)
_bump+d:	mov	si,#6(bp)
_bump+11:	add	*4(di),si
_bump+14:	adc	*6(di),*0
_bump+18:	adc	*8(di),*0
_bump+1c:	adc	*10(di),*0
_bump+20:	lea	sp,*-4(bp)
_bump+23:	pop	di
_bump+24:	pop	si
_bump+25:	pop	bp
_bump+26:	ret
_mult_ac:
_mult_ac:	push	bp
_mult_ac+1:	mov	bp,sp
_mult_ac+3:	push	si
_mult_ac+4:	push	di
_mult_ac+5:	sub	sp,#c
_mult_ac+9:	mov	ax,*6(bp)
_mult_ac+c:	add	ax,#4
_mult_ac+f:	mov	di,ax
_mult_ac+11:	lea	ax,*-14(bp)
_mult_ac+14:	mov	si,ax
_mult_ac+16:	mov	*-16(bp),#5
_mult_ac+1b:	mov	ax,*-16(bp)
_mult_ac+1e:	dec	*-16(bp)
_mult_ac+21:	or	ax,ax
_mult_ac+23:	je	_mult_ac+30
_mult_ac+25:	mov	bx,si
_mult_ac+27:	add	si,*2
_mult_ac+2a:	mov	(bx),#0
_mult_ac+2e:	j	_mult_ac+1b
_mult_ac+30:	mov	ax,*4(bp)
_mult_ac+33:	add	ax,#4
_mult_ac+36:	mov	si,ax
_mult_ac+38:	lea	bx,*-14(bp)
_mult_ac+3b:	mov	cx,#4
_mult_ac+3e:	push	si
_mult_ac+3f:	push	cx
_mult_ac+40:	push	bx
_mult_ac+41:	call	_mult_ac+60
_mult_ac+44:	pop	bx
_mult_ac+45:	mov	cx,#4
_mult_ac+48:	push	bx
_mult_ac+49:	mov	ax,*2(bx)
_mult_ac+4c:	mov	(bx),ax
_mult_ac+4e:	inc	bx
_mult_ac+4f:	inc	bx
_mult_ac+50:	loop	_mult_ac+49
_mult_ac+52:	mov	(bx),#0
_mult_ac+56:	pop	bx
_mult_ac+57:	pop	cx
_mult_ac+58:	pop	si
_mult_ac+59:	inc	di
_mult_ac+5a:	inc	di
_mult_ac+5b:	loop	_mult_ac+3e
_mult_ac+5d:	jmp	_mult_ac+80
_mult_ac+60:	mov	cx,#4
_mult_ac+63:	mov	ax,(si)
_mult_ac+65:	mul	(di)
_mult_ac+67:	add	(bx),ax
_mult_ac+69:	inc	bx
_mult_ac+6a:	inc	bx
_mult_ac+6b:	adc	(bx),dx
_mult_ac+6d:	jnb	_mult_ac+7b
_mult_ac+6f:	adc	*2(bx),*0
_mult_ac+73:	adc	*4(bx),*0
_mult_ac+77:	adc	*6(bx),*0
_mult_ac+7b:	inc	si
_mult_ac+7c:	inc	si
_mult_ac+7d:	loop	_mult_ac+63
_mult_ac+7f:	ret
_mult_ac+80:	sbb	ax,ax
_mult_ac+82:	lea	ax,*-14(bp)
_mult_ac+85:	mov	di,ax
_mult_ac+87:	mov	*-16(bp),#4
_mult_ac+8c:	mov	ax,*-16(bp)
_mult_ac+8f:	dec	*-16(bp)
_mult_ac+92:	or	ax,ax
_mult_ac+94:	je	_mult_ac+a6
_mult_ac+96:	mov	bx,di
_mult_ac+98:	add	di,*2
_mult_ac+9b:	mov	ax,(bx)
_mult_ac+9d:	mov	bx,si
_mult_ac+9f:	add	si,*2
_mult_ac+a2:	mov	(bx),ax
_mult_ac+a4:	j	_mult_ac+8c
_mult_ac+a6:	push	*4(bp)
_mult_ac+a9:	call	0
_mult_ac+ac:	pop	cx
_mult_ac+ad:	push	*4(bp)
_mult_ac+b0:	call	0
_mult_ac+b3:	pop	cx
_mult_ac+b4:	lea	sp,*-4(bp)
_mult_ac+b7:	pop	di
_mult_ac+b8:	pop	si
_mult_ac+b9:	pop	bp
_mult_ac+ba:	ret
_div_ac:
_div_ac:	push	bp
_div_ac+1:	mov	bp,sp
_div_ac+3:	push	si
_div_ac+4:	push	di
_div_ac+5:	sub	sp,#12
_div_ac+9:	mov	di,#4(bp)
_div_ac+d:	mov	si,#6(bp)
_div_ac+11:	lea	ax,*-16(bp)
_div_ac+14:	push	ax
_div_ac+15:	call	0
_div_ac+18:	pop	cx
_div_ac+19:	push	si
_div_ac+1a:	push	di
_div_ac+1b:	call	0
_div_ac+1e:	add	sp,*4
_div_ac+21:	mov	cx,#40
_div_ac+24:	lea	bx,*-16(bp)
_div_ac+27:	shl	*4(bx)
_div_ac+2a:	rcl	*6(bx)
_div_ac+2d:	rcl	*8(bx)
_div_ac+30:	rcl	*10(bx)
_div_ac+33:	shl	*4(di)
_div_ac+36:	rcl	*6(di)
_div_ac+39:	rcl	*8(di)
_div_ac+3c:	rcl	*10(di)
_div_ac+3f:	jnb	_div_ac+5b
_div_ac+41:	mov	ax,*4(si)
_div_ac+44:	add	*4(di),ax
_div_ac+47:	mov	ax,*6(si)
_div_ac+4a:	adc	*6(di),ax
_div_ac+4d:	mov	ax,*8(si)
_div_ac+50:	adc	*8(di),ax
_div_ac+53:	mov	ax,*10(si)
_div_ac+56:	adc	*10(di),ax
_div_ac+59:	j	_div_ac+77
_div_ac+5b:	mov	ax,*4(si)
_div_ac+5e:	sub	*4(di),ax
_div_ac+61:	mov	ax,*6(si)
_div_ac+64:	sbb	*6(di),ax
_div_ac+67:	mov	ax,*8(si)
_div_ac+6a:	sbb	*8(di),ax
_div_ac+6d:	mov	ax,*10(si)
_div_ac+70:	sbb	*10(di),ax
_div_ac+73:	orb	*-12(bp),*1
_div_ac+77:	loop	_div_ac+27
_div_ac+79:	mov	cx,#4
_div_ac+7c:	lea	di,*4(di)
_div_ac+7f:	lea	si,*-12(bp)
_div_ac+82:	mov	ax,(si)
_div_ac+84:	mov	(di),ax
_div_ac+86:	inc	di
_div_ac+87:	inc	si
_div_ac+88:	inc	di
_div_ac+89:	inc	si
_div_ac+8a:	loop	_div_ac+82
_div_ac+8c:	lea	sp,*-4(bp)
_div_ac+8f:	pop	di
_div_ac+90:	pop	si
_div_ac+91:	pop	bp
_div_ac+92:	ret
_ftoi:
_ftoi:		push	bp
_ftoi+1:	mov	bp,sp
_ftoi+3:	push	si
_ftoi+4:	push	di
_ftoi+5:	sub	sp,#2
_ftoi+9:	mov	di,#4(bp)
_ftoi+d:	mov	ax,#40
_ftoi+10:	push	ax
_ftoi+11:	push	di
_ftoi+12:	call	_bump
_ftoi+15:	add	sp,*4
_ftoi+18:	push	di
_ftoi+19:	call	0
_ftoi+1c:	pop	cx
_ftoi+1d:	mov	ax,#200f
_ftoi+20:	sub	ax,*2(di)
_ftoi+23:	mov	si,ax
_ftoi+25:	cmp	si,*10
_ftoi+28:	jle	_ftoi+2f
_ftoi+2a:	mov	ax,#0
_ftoi+2d:	j	_ftoi+60
_ftoi+2f:	or	si,si
_ftoi+31:	jnl	_ftoi+3a
_ftoi+33:	mov	*-6(bp),#7fff
_ftoi+38:	j	_ftoi+4e
_ftoi+3a:	mov	ax,si
_ftoi+3c:	dec	si
_ftoi+3d:	or	ax,ax
_ftoi+3f:	je	_ftoi+48
_ftoi+41:	push	di
_ftoi+42:	call	0
_ftoi+45:	pop	cx
_ftoi+46:	j	_ftoi+3a
_ftoi+48:	mov	ax,*10(di)
_ftoi+4b:	mov	*-6(bp),ax
_ftoi+4e:	cmp	(di),*0
_ftoi+51:	je	_ftoi+5b
_ftoi+53:	mov	ax,*-6(bp)
_ftoi+56:	neg	ax
_ftoi+58:	mov	*-6(bp),ax
_ftoi+5b:	mov	ax,*-6(bp)
_ftoi+5e:	j	_ftoi+60
_ftoi+60:	lea	sp,*-4(bp)
_ftoi+63:	pop	di
_ftoi+64:	pop	si
_ftoi+65:	pop	bp
_ftoi+66:	ret
_itof:
_itof:		push	bp
_itof+1:	mov	bp,sp
_itof+3:	push	si
_itof+4:	push	di
_itof+5:	sub	sp,#0
_itof+9:	mov	si,#4(bp)
_itof+d:	mov	di,#6(bp)
_itof+11:	push	si
_itof+12:	call	0
_itof+15:	pop	cx
_itof+16:	or	di,di
_itof+18:	jnl	_itof+26
_itof+1a:	mov	(si),#ffff
_itof+1e:	mov	ax,di
_itof+20:	neg	ax
_itof+22:	mov	di,ax
_itof+24:	j	_itof+2a
_itof+26:	mov	(si),#0
_itof+2a:	mov	*2(si),#200f
_itof+2f:	mov	*10(si),di
_itof+32:	push	si
_itof+33:	call	_normali
_itof+36:	pop	cx
_itof+37:	lea	sp,*-4(bp)
_itof+3a:	pop	di
_itof+3b:	pop	si
_itof+3c:	pop	bp
_itof+3d:	ret
_ftol:
_ftol:		push	bp
_ftol+1:	mov	bp,sp
_ftol+3:	push	si
_ftol+4:	push	di
_ftol+5:	sub	sp,#4
_ftol+9:	mov	di,#4(bp)
_ftol+d:	mov	ax,#40
_ftol+10:	push	ax
_ftol+11:	push	di
_ftol+12:	call	_bump
_ftol+15:	add	sp,*4
_ftol+18:	push	di
_ftol+19:	call	0
_ftol+1c:	pop	cx
_ftol+1d:	mov	ax,#201f
_ftol+20:	sub	ax,*2(di)
_ftol+23:	mov	si,ax
_ftol+25:	cmp	si,*20
_ftol+28:	jle	_ftol+32
_ftol+2a:	mov	ax,#0
_ftol+2d:	mov	dx,#0
_ftol+30:	j	_ftol+7c
_ftol+32:	or	si,si
_ftol+34:	jnl	_ftol+42
_ftol+36:	mov	*-8(bp),#ffff
_ftol+3b:	mov	*-6(bp),#7fff
_ftol+40:	j	_ftol+5c
_ftol+42:	mov	ax,si
_ftol+44:	dec	si
_ftol+45:	or	ax,ax
_ftol+47:	je	_ftol+50
_ftol+49:	push	di
_ftol+4a:	call	0
_ftol+4d:	pop	cx
_ftol+4e:	j	_ftol+42
_ftol+50:	mov	ax,*8(di)
_ftol+53:	mov	dx,*10(di)
_ftol+56:	mov	*-8(bp),ax
_ftol+59:	mov	*-6(bp),dx
_ftol+5c:	cmp	(di),*0
_ftol+5f:	je	_ftol+74
_ftol+61:	mov	ax,*-8(bp)
_ftol+64:	mov	dx,*-6(bp)
_ftol+67:	neg	dx
_ftol+69:	neg	ax
_ftol+6b:	sbb	dx,*0
_ftol+6e:	mov	*-8(bp),ax
_ftol+71:	mov	*-6(bp),dx
_ftol+74:	mov	ax,*-8(bp)
_ftol+77:	mov	dx,*-6(bp)
_ftol+7a:	j	_ftol+7c
_ftol+7c:	lea	sp,*-4(bp)
_ftol+7f:	pop	di
_ftol+80:	pop	si
_ftol+81:	pop	bp
_ftol+82:	ret
_ltof:
_ltof:		push	bp
_ltof+1:	mov	bp,sp
_ltof+3:	push	si
_ltof+4:	push	di
_ltof+5:	sub	sp,#0
_ltof+9:	mov	di,#4(bp)
_ltof+d:	push	di
_ltof+e:	call	0
_ltof+11:	pop	cx
_ltof+12:	cmp	*8(bp),*0
_ltof+16:	jnle	_ltof+39
_ltof+18:	jl	_ltof+20
_ltof+1a:	cmp	*6(bp),*0
_ltof+1e:	jnb	_ltof+39
_ltof+20:	mov	(di),#ffff
_ltof+24:	mov	ax,*6(bp)
_ltof+27:	mov	dx,*8(bp)
_ltof+2a:	neg	dx
_ltof+2c:	neg	ax
_ltof+2e:	sbb	dx,*0
_ltof+31:	mov	*6(bp),ax
_ltof+34:	mov	*8(bp),dx
_ltof+37:	j	_ltof+3d
_ltof+39:	mov	(di),#0
_ltof+3d:	mov	*2(di),#201f
_ltof+42:	mov	ax,*6(bp)
_ltof+45:	mov	dx,*8(bp)
_ltof+48:	mov	*8(di),ax
_ltof+4b:	mov	*10(di),dx
_ltof+4e:	push	di
_ltof+4f:	call	_normali
_ltof+52:	pop	cx
_ltof+53:	lea	sp,*-4(bp)
_ltof+56:	pop	di
_ltof+57:	pop	si
_ltof+58:	pop	bp
_ltof+59:	ret
_floadon:
_floadon:	push	bp
_floadon+1:	mov	bp,sp
_floadon+3:	push	si
_floadon+4:	push	di
_floadon+5:	sub	sp,#0
_floadon+9:	mov	ax,#1
_floadon+c:	push	ax
_floadon+d:	push	*4(bp)
_floadon+10:	call	_itof
_floadon+13:	add	sp,*4
_floadon+16:	lea	sp,*-4(bp)
_floadon+19:	pop	di
_floadon+1a:	pop	si
_floadon+1b:	pop	bp
_floadon+1c:	ret
_fsetcw:
_fsetcw:	push	bp
_fsetcw+1:	mov	bp,sp
_fsetcw+3:	push	si
_fsetcw+4:	push	di
_fsetcw+5:	sub	sp,#0
_fsetcw+9:	mov	di,#4(bp)
_fsetcw+d:	mov	si,#0
_fsetcw+10:	cmp	*2(di),*0
_fsetcw+14:	jne	_fsetcw+1a
_fsetcw+16:	or	si,#4000
_fsetcw+1a:	cmp	(di),*0
_fsetcw+1d:	je	_fsetcw+23
_fsetcw+1f:	or	si,#100
_fsetcw+23:	mov	ax,si
_fsetcw+25:	j	_fsetcw+27
_fsetcw+27:	lea	sp,*-4(bp)
_fsetcw+2a:	pop	di
_fsetcw+2b:	pop	si
_fsetcw+2c:	pop	bp
_fsetcw+2d:	ret
