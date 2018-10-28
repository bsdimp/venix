trap:		push	bx
trap+1:		push	cx
trap+2:		push	ds
trap+3:		mov	bx,cs
trap+5:		add	bx,#0

dsoff+2:	mov	ds,bx
dsoff+4:	cmpb	0,*0
dsoff+9:	je	dsoff+50
dsoff+b:	movb	0,*0
dsoff+10:	pop	_endic+ea
dsoff+14:	pop	_endic+ec
dsoff+18:	pop	_endic+ee
dsoff+1c:	pop	_endic+f0
dsoff+20:	pop	_endic+f2
dsoff+24:	pop	_endic+f4
dsoff+28:	pop	_endic+f6
dsoff+2c:	mov	cx,_endic+f4
dsoff+30:	push	cx
dsoff+31:	sub	cx,4
dsoff+35:	pop	4
dsoff+39:	add	a,cx
dsoff+3d:	mov	cx,sp
dsoff+3f:	mov	_endic+e8,#1
dsoff+45:	mov	ss,bx
dsoff+47:	mov	sp,#_endic+e8
dsoff+4a:	incb	10
dsoff+4e:	j	dsoff+53
dsoff+50:	xor	cx,cx
dsoff+52:	push	cx
dsoff+53:	push	cx
dsoff+54:	push	dx
dsoff+55:	push	es
dsoff+56:	call	ax
dsoff+58:	cli
dsoff+59:	pop	es
dsoff+5a:	pop	dx
dsoff+5b:	pop	ax
dsoff+5c:	pop	bx
dsoff+5d:	cmp	bx,*0
dsoff+60:	je	dsoff+ae
dsoff+62:	cmpb	0,*0
dsoff+67:	je	dsoff+75
dsoff+69:	push	dx
dsoff+6a:	push	ax
dsoff+6b:	call	0
dsoff+6e:	sti
dsoff+6f:	call	0
dsoff+72:	cli
dsoff+73:	pop	ax
dsoff+74:	pop	dx
dsoff+75:	cmpb	10,*2
dsoff+7a:	jne	dsoff+82
dsoff+7c:	wait
dsoff+7d:	esc	5,12
dsoff+81:	wait
dsoff+82:	movb	10,*0
dsoff+87:	pop	cx
dsoff+88:	pop	cx
dsoff+89:	pop	bx
dsoff+8a:	mov	ss,6
dsoff+8e:	mov	sp,ax
dsoff+90:	push	_endic+f6
dsoff+94:	push	4
dsoff+98:	push	_endic+f2
dsoff+9c:	push	_endic+f0
dsoff+a0:	mov	es,8
dsoff+a4:	incb	0
dsoff+a8:	mov	ds,6
dsoff+ac:	pop	ax
dsoff+ad:	iret
dsoff+ae:	pop	ds
dsoff+af:	pop	cx
dsoff+b0:	pop	bx
dsoff+b1:	pop	ax
dsoff+b2:	iret

ip_call:
ip_call:	pop	cx
ip_call+1:	pop	cx
ip_call+2:	or	dx,dx
ip_call+4:	je	ip_call+1d
ip_call+6:	add	cx,dx
ip_call+8:	mov	bx,2
ip_call+c:	add	bx,*4
ip_call+f:	mov	2,bx
ip_call+13:	mov	(bx),dx
ip_call+15:	pop	dx
ip_call+16:	pop	*2(bx)
ip_call+19:	xor	bx,bx
ip_call+1b:	push	bx
ip_call+1c:	push	dx
ip_call+1d:	push	cx
ip_call+1e:	push	ax
ip_call+1f:	iret

ip_ret:
ip_ret:		pop	cx
ip_ret+1:	pop	cx
ip_ret+2:	mov	bx,2
ip_ret+6:	sub	cx,(bx)
ip_ret+8:	push	cx
ip_ret+9:	push	*2(bx)
ip_ret+c:	sub	bx,*4
ip_ret+f:	mov	2,bx
ip_ret+13:	iret

_fuibyte:
_fuibyte:	mov	es,4
_fuibyte+4:	j	_fubyte+4

_fubyte:
_fubyte:	mov	es,6
_fubyte+4:	pop	cx
_fubyte+5:	pop	bx
_fubyte+6:	push	bx
_fubyte+7:	sub	ax,ax
_fubyte+9:	seg	es
_fubyte+a:	movb	al,(bx)
_fubyte+c:	jmp	cx

_fuiword:
_fuiword:	mov	es,4
_fuiword+4:	j	_fuword+4

_fuword:
_fuword:	mov	es,6
_fuword+4:	pop	cx
_fuword+5:	pop	bx
_fuword+6:	push	bx
_fuword+7:	seg	es
_fuword+8:	mov	ax,(bx)
_fuword+a:	jmp	cx

_suibyte:
_suibyte:	mov	es,4
_suibyte+4:	j	_subyte+4

_subyte:
_subyte:	mov	es,6
_subyte+4:	pop	cx
_subyte+5:	pop	bx
_subyte+6:	pop	ax
_subyte+7:	push	ax
_subyte+8:	push	bx
_subyte+9:	seg	es
_subyte+a:	movb	(bx),al
_subyte+c:	sub	ax,ax
_subyte+e:	jmp	cx

_suiword:
_suiword:	mov	es,4
_suiword+4:	j	_suword+4

_suword:
_suword:	mov	es,6
_suword+4:	pop	cx
_suword+5:	pop	bx
_suword+6:	pop	ax
_suword+7:	push	ax
_suword+8:	push	bx
_suword+9:	seg	es
_suword+a:	mov	(bx),ax
_suword+c:	sub	ax,ax
_suword+e:	jmp	cx

_copyiin:
_copyiin:	mov	ax,4
_copyiin+3:	j	_copyin+3

_copyin:
_copyin:	mov	ax,6
_copyin+3:	push	bp
_copyin+4:	mov	bp,sp
_copyin+6:	push	di
_copyin+7:	push	si
_copyin+8:	mov	di,*6(bp)
_copyin+b:	mov	si,*4(bp)
_copyin+e:	mov	cx,*8(bp)
_copyin+11:	mov	ds,ax
_copyin+13:	mov	ax,ss
_copyin+15:	mov	es,ax
_copyin+17:	cld
_copyin+18:	repz
_copyin+19:	movb
_copyin+1a:	mov	ds,ax
_copyin+1c:	pop	si
_copyin+1d:	pop	di
_copyin+1e:	pop	bp
_copyin+1f:	sub	ax,ax
_copyin+21:	ret

_copyiout:
_copyiout:	mov	es,4
_copyiout+4:	j	_copyout+4

_copyout:
_copyout:	mov	es,6
_copyout+4:	push	bp
_copyout+5:	mov	bp,sp
_copyout+7:	push	di
_copyout+8:	push	si
_copyout+9:	mov	si,*4(bp)
_copyout+c:	mov	di,*6(bp)
_copyout+f:	mov	cx,*8(bp)
_copyout+12:	cld
_copyout+13:	repz
_copyout+14:	movb
_copyout+15:	pop	si
_copyout+16:	pop	di
_copyout+17:	pop	bp
_copyout+18:	sub	ax,ax
_copyout+1a:	ret

_clearse:
_clearse:	pop	bx
_clearse+1:	pop	es
_clearse+2:	push	es
_clearse+3:	push	di
_clearse+4:	sub	ax,ax
_clearse+6:	mov	di,ax
_clearse+8:	mov	cx,#100
_clearse+b:	cld
_clearse+c:	repz
_clearse+d:	stosw
_clearse+e:	pop	di
_clearse+f:	mov	ax,bx
_clearse+11:	jmp	ax

_copyseg:
_copyseg:	pop	ax
_copyseg+1:	pop	bx
_copyseg+2:	pop	es
_copyseg+3:	push	es
_copyseg+4:	push	bx
_copyseg+5:	push	ds
_copyseg+6:	mov	ds,bx
_copyseg+8:	mov	cx,#100
_copyseg+b:	push	si
_copyseg+c:	push	di
_copyseg+d:	sub	si,si
_copyseg+f:	sub	di,di
_copyseg+11:	cld
_copyseg+12:	repz
_copyseg+13:	movw
_copyseg+14:	pop	di
_copyseg+15:	pop	si
_copyseg+16:	pop	ds
_copyseg+17:	jmp	ax

_bcopy:
_bcopy:		push	bp
_bcopy+1:	mov	bp,sp
_bcopy+3:	push	si
_bcopy+4:	push	di
_bcopy+5:	mov	ax,ds
_bcopy+7:	mov	es,ax
_bcopy+9:	mov	cx,*8(bp)
_bcopy+c:	mov	si,*4(bp)
_bcopy+f:	mov	di,*6(bp)
_bcopy+12:	cld
_bcopy+13:	repz
_bcopy+14:	movb
_bcopy+15:	pop	di
_bcopy+16:	pop	si
_bcopy+17:	pop	bp
_bcopy+18:	ret

_savfp:
_savfp:		cli
_savfp+1:	cmpb	_endic,*1
_savfp+6:	jne	_savfp+19
_savfp+8:	cmpb	10,*1
_savfp+d:	jne	_savfp+19
_savfp+f:	incb	10
_savfp+13:	wait
_savfp+14:	esc	5,12
_savfp+18:	wait
_savfp+19:	sti
_savfp+1a:	ret

_savu:
_savu:		pop	ax
_savu+1:	pop	bx
_savu+2:	push	bx
_savu+3:	mov	(bx),bp
_savu+5:	jmp	ax

_aretu:
_aretu:		pop	ax
_aretu+1:	pop	bx
_aretu+2:	mov	bp,(bx)
_aretu+4:	lea	sp,*-6(bp)
_aretu+7:	jmp	ax

_retu:
_retu:		pop	ax
_retu+1:	pop	bx
_retu+2:	push	bx
_retu+3:	push	ds
_retu+4:	push	ds
_retu+5:	pop	es
_retu+6:	mov	ds,bx
_retu+8:	push	si
_retu+9:	push	di
_retu+a:	mov	di,#0
_retu+d:	mov	bx,di
_retu+f:	sub	si,si
_retu+11:	mov	cx,#200
_retu+14:	cld
_retu+15:	repz
_retu+16:	movw
_retu+17:	pop	di
_retu+18:	pop	si
_retu+19:	pop	ds
_retu+1a:	j	_aretu+2

_outu:
_outu:		pop	ax
_outu+1:	pop	es
_outu+2:	push	es
_outu+3:	push	si
_outu+4:	push	di
_outu+5:	sub	di,di
_outu+7:	mov	si,#0
_outu+a:	mov	cx,#200
_outu+d:	cld
_outu+e:	repz
_outu+f:	movw
_outu+10:	pop	di
_outu+11:	pop	si
_outu+12:	jmp	ax

_sstack:
_sstack:	pop	ax
_sstack+1:	mov	sp,#1fc
_sstack+4:	mov	bp,sp
_sstack+6:	jmp	ax

_incupc:
_incupc:	push	bp
_incupc+1:	mov	bp,sp
_incupc+3:	mov	bx,*6(bp)
_incupc+6:	mov	ax,*4(bp)
_incupc+9:	sub	ax,*4(bx)
_incupc+c:	shr	ax
_incupc+e:
data address not found
_incupc+10:	push	es
_incupc+11:	mov	cx,#e
_incupc+14:	shr	dx
_incupc+16:	rcr	ax
_incupc+18:	loop	_incupc+14
_incupc+1a:	inc	ax
_incupc+1b:	and	ax,#fffe
_incupc+1e:	cmp	ax,*2(bx)
_incupc+21:	jnb	_incupc+34
_incupc+23:	add	ax,(bx)
_incupc+25:	mov	bx,ax
_incupc+27:	mov	ax,*8(bp)
_incupc+2a:	mov	es,6
_incupc+2e:	seg	es
_incupc+2f:	add	ax,(bx)
_incupc+31:	seg	es
_incupc+32:	mov	(bx),ax
_incupc+34:	pop	bp
_incupc+35:	ret

_setiva:
_setiva:	push	bp
_setiva+1:	mov	bp,sp
_setiva+3:	mov	bx,*4(bp)
_setiva+6:	mov	ax,*6(bp)
_setiva+9:	mov	bp,*8(bp)
_setiva+c:	movb	*0(bp),*50
_setiva+10:	movb	*1(bp),*b8
_setiva+14:	mov	*2(bp),ax
_setiva+17:	movb	*4(bp),*ea
_setiva+1b:	mov	*5(bp),#0
_setiva+20:	mov	*7(bp),cs
_setiva+23:	sub	ax,ax
_setiva+25:	mov	es,ax
_setiva+27:	seg	es
_setiva+28:	mov	(bx),bp
_setiva+2a:	seg	es
_setiva+2b:	mov	*2(bx),ds
_setiva+2e:	pop	bp
_setiva+2f:	ret

_lshift:
_lshift:	push	bp
_lshift+1:	mov	bp,sp
_lshift+3:	mov	cx,*6(bp)
_lshift+6:	mov	bp,*4(bp)
_lshift+9:	mov	bx,*0(bp)
_lshift+c:	mov	ax,*2(bp)
_lshift+f:
data address not found
_lshift+11:	or	cx,cx
_lshift+13:	je	_lshift+1b
_lshift+15:	shr	bx
_lshift+17:	rcr	ax
_lshift+19:	loop	_lshift+15
_lshift+1b:	pop	bp
_lshift+1c:	ret

_dpadd:
_dpadd:		push	bp
_dpadd+1:	mov	bp,sp
_dpadd+3:	mov	ax,*6(bp)
_dpadd+6:	mov	bx,*4(bp)
_dpadd+9:	add	*2(bx),ax
_dpadd+c:	adc	(bx),*0
_dpadd+f:	pop	bp
_dpadd+10:	ret

_dpcmp:
_dpcmp:		push	bp
_dpcmp+1:	mov	bp,sp
_dpcmp+3:	mov	ax,*6(bp)
_dpcmp+6:	mov	dx,*4(bp)
_dpcmp+9:	sub	ax,*10(bp)
_dpcmp+c:	sbb	dx,*8(bp)
_dpcmp+f:	jl	_dpcmp+1d
_dpcmp+11:	jnle	_dpcmp+18
_dpcmp+13:	cmp	ax,#200
_dpcmp+16:	jbe	_dpcmp+25
_dpcmp+18:	mov	ax,#200
_dpcmp+1b:	j	_dpcmp+25
_dpcmp+1d:	cmp	ax,#fe00
_dpcmp+20:	jnb	_dpcmp+25
_dpcmp+22:	mov	ax,#fe00
_dpcmp+25:	pop	bp
_dpcmp+26:	ret
