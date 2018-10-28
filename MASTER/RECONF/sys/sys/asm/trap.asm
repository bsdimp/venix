_nmi:		push	bp
_nmi+1:		mov	bp,sp
_nmi+3:		push	si
_nmi+4:		push	di
_nmi+5:		call	0
_nmi+8:		mov	ax,#0
_nmi+b:		push	ax
_nmi+c:		call	0
_nmi+f:		pop	cx
_nmi+10:	pop	di
_nmi+11:	pop	si
_nmi+12:	pop	bp
_nmi+13:	ret

_uintr:
_uintr:		push	bp
_uintr+1:	mov	bp,sp
_uintr+3:	push	si
_uintr+4:	push	di
_uintr+5:	call	0
_uintr+8:	mov	ax,#0
_uintr+b:	push	ax
_uintr+c:	call	0
_uintr+f:	pop	cx
_uintr+10:	pop	di
_uintr+11:	pop	si
_uintr+12:	pop	bp
_uintr+13:	ret

_ttrap:
_ttrap:		push	bp
_ttrap+1:	mov	bp,sp
_ttrap+3:	push	si
_ttrap+4:	push	di
_ttrap+5:	and	*24(bp),#feff
_ttrap+a:	push	*10(bp)
_ttrap+d:	mov	ax,#5
_ttrap+10:	push	ax
_ttrap+11:	call	_tsignal
_ttrap+14:	add	sp,*4
_ttrap+17:	pop	di
_ttrap+18:	pop	si
_ttrap+19:	pop	bp
_ttrap+1a:	ret

_abort:
_abort:		push	bp
_abort+1:	mov	bp,sp
_abort+3:	push	si
_abort+4:	push	di
_abort+5:	push	*10(bp)
_abort+8:	mov	ax,#6
_abort+b:	push	ax
_abort+c:	call	_tsignal
_abort+f:	add	sp,*4
_abort+12:	pop	di
_abort+13:	pop	si
_abort+14:	pop	bp
_abort+15:	ret

_emt:
_emt:		push	bp
_emt+1:		mov	bp,sp
_emt+3:		push	si
_emt+4:		push	di
_emt+5:		push	*10(bp)
_emt+8:		mov	ax,#7
_emt+b:		push	ax
_emt+c:		call	_tsignal
_emt+f:		add	sp,*4
_emt+12:	pop	di
_emt+13:	pop	si
_emt+14:	pop	bp
_emt+15:	ret

_tsignal:
_tsignal:	push	bp
_tsignal+1:	mov	bp,sp
_tsignal+3:	push	si
_tsignal+4:	push	di
_tsignal+5:	cmp	*6(bp),*0
_tsignal+9:	jne	_tsignal+20
_tsignal+b:	push	*4(bp)
_tsignal+e:	mov	ax,#0
_tsignal+11:	push	ax
_tsignal+12:	call	0
_tsignal+15:	add	sp,*4
_tsignal+18:	mov	ax,#0
_tsignal+1b:	push	ax
_tsignal+1c:	call	0
_tsignal+1f:	pop	cx
_tsignal+20:	push	*4(bp)
_tsignal+23:	push	76
_tsignal+27:	call	0
_tsignal+2a:	add	sp,*4
_tsignal+2d:	call	0
_tsignal+30:	or	ax,ax
_tsignal+32:	je	_tsignal+37
_tsignal+34:	call	0
_tsignal+37:	push	76
_tsignal+3b:	call	0
_tsignal+3e:	pop	cx
_tsignal+3f:	pop	di
_tsignal+40:	pop	si
_tsignal+41:	pop	bp
_tsignal+42:	ret

_setustk:
_setustk:	push	bp
_setustk+1:	mov	bp,sp
_setustk+3:	push	si
_setustk+4:	push	di
_setustk+5:	mov	di,76
_setustk+9:	cmp	*18(di),*0
_setustk+d:	je	_setustk+1f
_setustk+f:	mov	ax,*4(bp)
_setustk+12:	mov	cx,#9
_setustk+15:	shr	ax,cl
_setustk+17:	mov	dx,#83
_setustk+1a:	sub	dx,ax
_setustk+1c:	mov	*18(di),dx
_setustk+1f:	pop	di
_setustk+20:	pop	si
_setustk+21:	pop	bp
_setustk+22:	ret

_sys:
_sys:		push	bp
_sys+1:		mov	bp,sp
_sys+3:		push	si
_sys+4:		push	di
_sys+5:		call	0
_sys+8:		cmp	*10(bp),*0
_sys+c:		jne	_sys+16
_sys+e:		mov	ax,#0
_sys+11:	push	ax
_sys+12:	call	0
_sys+15:	pop	cx
_sys+16:	mov	di,76
_sys+1a:	cmp	*18(di),*0
_sys+1e:	je	_sys+30
_sys+20:	mov	ax,*8(bp)
_sys+23:	mov	cx,#9
_sys+26:	shr	ax,cl
_sys+28:	mov	dx,#83
_sys+2b:	sub	dx,ax
_sys+2d:	mov	*18(di),dx
_sys+30:	lea	ax,*18(bp)
_sys+33:	mov	128,ax
_sys+36:	movb	71,*0
_sys+3b:	cmp	*16(bp),*48
_sys+3f:	jnl	_sys+62
_sys+41:	mov	ax,*18(bp)
_sys+44:	mov	92,ax
_sys+47:	mov	bx,*16(bp)
_sys+4a:	shl	bx
_sys+4c:	push	#0(bx)
_sys+50:	call	_trap1
_sys+53:	pop	cx
_sys+54:	cmpb	134,*0
_sys+59:	je	_sys+67
_sys+5b:	movb	71,*4
_sys+60:	j	_sys+67
_sys+62:	movb	71,*16
_sys+67:	movb	al,71
_sys+6a:	cbw
_sys+6b:	mov	*14(bp),ax
_sys+6e:	cmp	ax,#e
_sys+71:	je	_sys+86
_sys+73:	cmpb	71,*0
_sys+78:	je	_sys+94
_sys+7a:	mov	*18(bp),#ffff
_sys+7f:	mov	*6(bp),#ffff
_sys+84:	j	_sys+94
_sys+86:	mov	ax,#c
_sys+89:	push	ax
_sys+8a:	push	76
_sys+8e:	call	0
_sys+91:	add	sp,*4
_sys+94:	call	0
_sys+97:	or	ax,ax
_sys+99:	je	_sys+9e
_sys+9b:	call	0
_sys+9e:	push	76
_sys+a2:	call	0
_sys+a5:	pop	cx
_sys+a6:	pop	di
_sys+a7:	pop	si
_sys+a8:	pop	bp
_sys+a9:	ret

_trap1:
_trap1:		push	bp
_trap1+1:	mov	bp,sp
_trap1+3:	push	si
_trap1+4:	push	di
_trap1+5:	incb	134
_trap1+9:	mov	ax,#ce
_trap1+c:	push	ax
_trap1+d:	call	0
_trap1+10:	pop	cx
_trap1+11:	call	*4(bp)
_trap1+14:	movb	134,*0
_trap1+19:	pop	di
_trap1+1a:	pop	si
_trap1+1b:	pop	bp
_trap1+1c:	ret

_nosys:
_nosys:		push	bp
_nosys+1:	mov	bp,sp
_nosys+3:	push	si
_nosys+4:	push	di
_nosys+5:	movb	71,*16
_nosys+a:	pop	di
_nosys+b:	pop	si
_nosys+c:	pop	bp
_nosys+d:	ret

_nullsys:
_nullsys:	push	bp
_nullsys+1:	mov	bp,sp
_nullsys+3:	push	si
_nullsys+4:	push	di
_nullsys+5:	pop	di
_nullsys+6:	pop	si
_nullsys+7:	pop	bp
_nullsys+8:	ret
