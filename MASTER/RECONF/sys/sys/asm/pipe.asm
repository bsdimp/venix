_pipe:		push	bp
_pipe+1:	mov	bp,sp
_pipe+3:	push	si
_pipe+4:	push	di
_pipe+5:	sub	sp,*4
_pipe+8:	mov	ax,0
_pipe+b:	mov	*-8(bp),ax
_pipe+e:	mov	*-6(bp),#0
_pipe+13:	mov	ax,*-6(bp)
_pipe+16:	cmp	ax,0
_pipe+1a:	jnb	_pipe+35
_pipe+1c:	mov	bx,*-6(bp)
_pipe+1f:	mov	ax,(bx)
_pipe+21:	cmp	ax,0
_pipe+25:	jne	_pipe+2f
_pipe+27:	mov	ax,0
_pipe+2a:	mov	*-8(bp),ax
_pipe+2d:	j	_pipe+35
_pipe+2f:	add	*-6(bp),*6
_pipe+33:	j	_pipe+13
_pipe+35:	push	*-8(bp)
_pipe+38:	call	0
_pipe+3b:	pop	cx
_pipe+3c:	mov	di,ax
_pipe+3e:	or	ax,ax
_pipe+40:	je	_pipe+79
_pipe+42:	call	0
_pipe+45:	mov	si,ax
_pipe+47:	or	ax,ax
_pipe+49:	jne	_pipe+52
_pipe+4b:	push	di
_pipe+4c:	call	0
_pipe+4f:	pop	cx
_pipe+50:	j	_pipe+79
_pipe+52:	mov	bx,128
_pipe+56:	mov	ax,(bx)
_pipe+58:	mov	*-8(bp),ax
_pipe+5b:	call	0
_pipe+5e:	mov	*-6(bp),ax
_pipe+61:	or	ax,ax
_pipe+63:	jne	_pipe+7b
_pipe+65:	movb	*1(si),*0
_pipe+69:	mov	bx,*-8(bp)
_pipe+6c:	shl	bx
_pipe+6e:	mov	#a6(bx),#0
_pipe+74:	push	di
_pipe+75:	call	0
_pipe+78:	pop	cx
_pipe+79:	j	_pipe+b3
_pipe+7b:	mov	bx,128
_pipe+7f:	mov	ax,(bx)
_pipe+81:	mov	bx,128
_pipe+85:	mov	*-12(bx),ax
_pipe+88:	mov	bx,128
_pipe+8c:	mov	ax,*-8(bp)
_pipe+8f:	mov	(bx),ax
_pipe+91:	mov	bx,*-6(bp)
_pipe+94:	movb	(bx),*6
_pipe+97:	mov	ax,di
_pipe+99:	mov	bx,*-6(bp)
_pipe+9c:	mov	*2(bx),ax
_pipe+9f:	movb	(si),*5
_pipe+a2:	mov	ax,di
_pipe+a4:	mov	*2(si),ax
_pipe+a7:	movb	*1(di),*2
_pipe+ab:	movb	(di),*46
_pipe+ae:	mov	*6(di),#8000
_pipe+b3:	lea	sp,*-4(bp)
_pipe+b6:	pop	di
_pipe+b7:	pop	si
_pipe+b8:	pop	bp
_pipe+b9:	ret
_readp:
_readp:		push	bp
_readp+1:	mov	bp,sp
_readp+3:	push	si
_readp+4:	push	di
_readp+5:	mov	di,*4(bp)
_readp+8:	mov	si,*2(di)
_readp+b:	push	si
_readp+c:	call	_plock
_readp+f:	pop	cx
_readp+10:	cmp	*12(si),*0
_readp+14:	jne	_readp+37
_readp+16:	push	si
_readp+17:	call	_prele
_readp+1a:	pop	cx
_readp+1b:	cmpb	*1(si),*2
_readp+1f:	jb	_readp+76
_readp+21:	or	*6(si),#100
_readp+26:	mov	ax,#1
_readp+29:	push	ax
_readp+2a:	mov	ax,si
_readp+2c:	inc	ax
_readp+2d:	inc	ax
_readp+2e:	push	ax
_readp+2f:	call	0
_readp+32:	add	sp,*4
_readp+35:	j	_readp+b
_readp+37:	mov	7c,#0
_readp+3d:	mov	ax,*6(di)
_readp+40:	mov	7e,ax
_readp+43:	push	si
_readp+44:	call	0
_readp+47:	pop	cx
_readp+48:	mov	ax,7e
_readp+4b:	mov	*6(di),ax
_readp+4e:	cmp	ax,*12(si)
_readp+51:	jne	_readp+71
_readp+53:	mov	*6(di),#0
_readp+58:	mov	*12(si),#0
_readp+5d:
data address not found
_readp+5f:	push	es
_readp+60:	addb	(bx+si),*74
_readp+63:	or	ax,#6481
_readp+66:	push	es
_readp+67:	jnle	_readp+68
_readp+69:	mov	ax,si
_readp+6b:	inc	ax
_readp+6c:	push	ax
_readp+6d:	call	0
_readp+70:	pop	cx
_readp+71:	push	si
_readp+72:	call	_prele
_readp+75:	pop	cx
_readp+76:	pop	di
_readp+77:	pop	si
_readp+78:	pop	bp
_readp+79:	ret
_writep:
_writep:	push	bp
_writep+1:	mov	bp,sp
_writep+3:	push	si
_writep+4:	push	di
_writep+5:	push	cx
_writep+6:	mov	di,*4(bp)
_writep+9:	mov	si,*2(di)
_writep+c:	mov	ax,7a
_writep+f:	mov	*-6(bp),ax
_writep+12:	push	si
_writep+13:	call	_plock
_writep+16:	pop	cx
_writep+17:	cmp	*-6(bp),*0
_writep+1b:	jne	_writep+2a
_writep+1d:	push	si
_writep+1e:	call	_prele
_writep+21:	pop	cx
_writep+22:	mov	7a,#0
_writep+28:	j	_writep+48
_writep+2a:	cmpb	*1(si),*2
_writep+2e:	jnb	_writep+4a
_writep+30:	push	si
_writep+31:	call	_prele
_writep+34:	pop	cx
_writep+35:	movb	71,*20
_writep+3a:	mov	ax,#d
_writep+3d:	push	ax
_writep+3e:	push	76
_writep+42:	call	0
_writep+45:	add	sp,*4
_writep+48:	j	_writep+ad
_writep+4a:	cmp	*12(si),#1000
_writep+4f:	jb	_writep+6b
_writep+51:	or	*6(si),#80
_writep+56:	push	si
_writep+57:	call	_prele
_writep+5a:	pop	cx
_writep+5b:	mov	ax,#1
_writep+5e:	push	ax
_writep+5f:	mov	ax,si
_writep+61:	inc	ax
_writep+62:	push	ax
_writep+63:	call	0
_writep+66:	add	sp,*4
_writep+69:	j	_writep+12
_writep+6b:	mov	7c,#0
_writep+71:	mov	ax,*12(si)
_writep+74:	mov	7e,ax
_writep+77:	cmp	*-6(bp),#1000
_writep+7c:	jnl	_writep+83
_writep+7e:	mov	ax,*-6(bp)
_writep+81:	j	_writep+86
_writep+83:	mov	ax,#1000
_writep+86:	mov	7a,ax
_writep+89:	sub	*-6(bp),ax
_writep+8c:	push	si
_writep+8d:	call	0
_writep+90:	pop	cx
_writep+91:	push	si
_writep+92:	call	_prele
_writep+95:	pop	cx
_writep+96:
data address not found
_writep+98:	push	es
_writep+99:	addb	(bx+di),al
_writep+9b:	je	_writep+69
_writep+9d:	and	*6(si),#feff
_writep+a2:	mov	ax,si
_writep+a4:	inc	ax
_writep+a5:	inc	ax
_writep+a6:	push	ax
_writep+a7:	call	0
_writep+aa:	pop	cx
_writep+ab:	j	_writep+69
_writep+ad:	pop	cx
_writep+ae:	pop	di
_writep+af:	pop	si
_writep+b0:	pop	bp
_writep+b1:	ret
_plock:
_plock:		push	bp
_plock+1:	mov	bp,sp
_plock+3:	push	si
_plock+4:	push	di
_plock+5:	mov	di,*4(bp)
_plock+8:	movb	al,(di)
_plock+a:	and	ax,#ff
_plock+d:	test	ax,#1
_plock+10:	je	_plock+22
_plock+12:	orb	(di),*10
_plock+15:	mov	ax,#1
_plock+18:	push	ax
_plock+19:	push	di
_plock+1a:	call	0
_plock+1d:	add	sp,*4
_plock+20:	j	_plock+8
_plock+22:	orb	(di),*1
_plock+25:	pop	di
_plock+26:	pop	si
_plock+27:	pop	bp
_plock+28:	ret
_prele:
_prele:		push	bp
_prele+1:	mov	bp,sp
_prele+3:	push	si
_prele+4:	push	di
_prele+5:	mov	di,*4(bp)
_prele+8:	andb	(di),*fe
_prele+b:	movb	al,(di)
_prele+d:	and	ax,#ff
_prele+10:	test	ax,#10
_prele+13:	je	_prele+1d
_prele+15:	andb	(di),*ef
_prele+18:	push	di
_prele+19:	call	0
_prele+1c:	pop	cx
_prele+1d:	pop	di
_prele+1e:	pop	si
_prele+1f:	pop	bp
_prele+20:	ret