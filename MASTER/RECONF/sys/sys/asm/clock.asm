_clock:		push	bp
_clock+1:	mov	bp,sp
_clock+3:	push	si
_clock+4:	push	di
_clock+5:	sub	sp,*6
_clock+8:	cmp	4,*0
_clock+d:	je	_clock+2c
_clock+f:	mov	di,#0
_clock+12:	cmp	(di),*0
_clock+15:	jnle	_clock+22
_clock+17:	cmp	*4(di),*0
_clock+1b:	je	_clock+22
_clock+1d:	add	di,*6
_clock+20:	j	_clock+12
_clock+22:	mov	ax,_clkdiff
_clock+25:	sub	(di),ax
_clock+27:	test	*24(bp)
_clock+2a:	addb	#3d74(bp+si),al
_clock+2e:	call	0
_clock+31:	cmp	0,*0
_clock+36:	jnle	_clock+6b
_clock+38:	mov	ax,4
_clock+3b:	mov	*-6(bp),ax
_clock+3e:	mov	si,2
_clock+42:	mov	di,#0
_clock+45:	mov	ax,*10(di)
_clock+48:	mov	*4(di),ax
_clock+4b:	or	ax,ax
_clock+4d:	je	_clock+5f
_clock+4f:	mov	ax,*6(di)
_clock+52:	mov	(di),ax
_clock+54:	mov	ax,*8(di)
_clock+57:	mov	*2(di),ax
_clock+5a:	add	di,*6
_clock+5d:	j	_clock+45
_clock+5f:	push	si
_clock+60:	call	*-6(bp)
_clock+63:	pop	cx
_clock+64:	cmp	4,*0
_clock+69:	jne	_clock+31
_clock+6b:	cmp	*10(bp),*0
_clock+6f:	je	_clock+a9
_clock+71:	mov	ax,_clkdiff
_clock+74:	cwd
_clock+75:	mov	*-10(bp),ax
_clock+78:	mov	*-8(bp),dx
_clock+7b:	mov	ax,118
_clock+7e:	mov	dx,11a
_clock+82:	add	ax,*-10(bp)
_clock+85:	adc	dx,*-8(bp)
_clock+88:	mov	118,ax
_clock+8b:	mov	11a,dx
_clock+8f:	cmp	130,*0
_clock+94:	je	_clock+cf
_clock+96:	push	_clkdiff
_clock+9a:	mov	ax,#12a
_clock+9d:	push	ax
_clock+9e:	push	*20(bp)
_clock+a1:	call	0
_clock+a4:	add	sp,*6
_clock+a7:	j	_clock+cf
_clock+a9:	cmp	0,#80
_clock+af:	jnl	_clock+cf
_clock+b1:	mov	ax,_clkdiff
_clock+b4:	cwd
_clock+b5:	mov	*-10(bp),ax
_clock+b8:	mov	*-8(bp),dx
_clock+bb:	mov	ax,11c
_clock+be:	mov	dx,11e
_clock+c2:	add	ax,*-10(bp)
_clock+c5:	adc	dx,*-8(bp)
_clock+c8:	mov	11c,ax
_clock+cb:	mov	11e,dx
_clock+cf:	mov	si,76
_clock+d3:	cmpb	*6(si),*50
_clock+d7:	jnb	_clock+e8
_clock+d9:	movb	al,*6(si)
_clock+dc:	and	ax,#ff
_clock+df:	addb	al,_clkdiff
_clock+e3:	mov	cx,ax
_clock+e5:	movb	*6(si),cl
_clock+e8:	mov	ax,_clkdiff
_clock+eb:	add	0,ax
_clock+ef:	mov	ax,0
_clock+f2:	cmp	ax,0
_clock+f6:	jnl	_clock+fb
_clock+f8:	jmp	1ab
_clock+fb:	test	*24(bp)
_clock+fe:	addb	#_clkdiff+d3(bp+si),al
102:		jmp	1ae
105:		mov	ax,0
108:		sub	0,ax
10c:		add	0,*1
111:		adc	2,*0
116:		call	0
119:		incb	0
11d:		mov	ax,#0
120:		push	ax
121:		call	0
124:		pop	cx
125:		mov	si,#0
128:		cmp	si,0
12c:		jnb	172
12e:		cmpb	*2(si),*0
132:		je	16d
134:		cmpb	*5(si),*7c
138:		jnb	149
13a:		movb	al,*5(si)
13d:		and	ax,#ff
140:		addb	al,_clkdiff
144:		mov	cx,ax
146:		movb	*5(si),cl
149:		shrb	*6(si)
14c:		cmpb	*3(si),*64
150:		jl	157
152:		push	si
153:		call	0
156:		pop	cx
157:		cmp	*32(si),*0
15b:		je	16d
15d:		dec	*32(si)
160:		jne	16d
162:		mov	ax,#e
165:		push	ax
166:		push	si
167:		call	0
16a:		add	sp,*4
16d:		add	si,*22
170:		j	128
172:		cmpb	0,*0
177:		je	186
179:		movb	0,*0
17e:		mov	ax,#0
181:		push	ax
182:		call	0
185:		pop	cx
186:		cmp	*10(bp),*0
18a:		je	1ab
18c:		lea	ax,*18(bp)
18f:		mov	128,ax
192:		push	*8(bp)
195:		call	0
198:		pop	cx
199:		call	0
19c:		or	ax,ax
19e:		je	1a3
1a0:		call	0
1a3:		push	76
1a7:		call	0
1aa:		pop	cx
1ab:		call	0
1ae:		lea	sp,*-4(bp)
1b1:		pop	di
1b2:		pop	si
1b3:		pop	bp
1b4:		ret
_timeout:
_timeout:	push	bp
_timeout+1:	mov	bp,sp
_timeout+3:	push	si
_timeout+4:	push	di
_timeout+5:	push	cx
_timeout+6:	mov	di,#0
_timeout+9:	call	0
_timeout+c:	mov	*-6(bp),ax
_timeout+f:	cmp	*4(di),*0
_timeout+13:	je	_timeout+26
_timeout+15:	mov	ax,(di)
_timeout+17:	cmp	ax,*8(bp)
_timeout+1a:	jnle	_timeout+26
_timeout+1c:	mov	ax,(di)
_timeout+1e:	sub	*8(bp),ax
_timeout+21:	add	di,*6
_timeout+24:	j	_timeout+f
_timeout+26:	mov	si,di
_timeout+28:	cmp	*4(si),*0
_timeout+2c:	je	_timeout+33
_timeout+2e:	add	si,*6
_timeout+31:	j	_timeout+28
_timeout+33:	cmp	si,0
_timeout+37:	jb	_timeout+4a
_timeout+39:	push	*-6(bp)
_timeout+3c:	call	0
_timeout+3f:	pop	cx
_timeout+40:	mov	ax,#0
_timeout+43:	push	ax
_timeout+44:	call	0
_timeout+47:	pop	cx
_timeout+48:	j	_timeout+83
_timeout+4a:	mov	ax,*8(bp)
_timeout+4d:	sub	(di),ax
_timeout+4f:	mov	ax,si
_timeout+51:	sub	si,*6
_timeout+54:	cmp	ax,di
_timeout+56:	jbe	_timeout+6b
_timeout+58:	mov	ax,(si)
_timeout+5a:	mov	*6(si),ax
_timeout+5d:	mov	ax,*4(si)
_timeout+60:	mov	*10(si),ax
_timeout+63:	mov	ax,*2(si)
_timeout+66:	mov	*8(si),ax
_timeout+69:	j	_timeout+4f
_timeout+6b:	mov	ax,*8(bp)
_timeout+6e:	mov	(di),ax
_timeout+70:	mov	ax,*4(bp)
_timeout+73:	mov	*4(di),ax
_timeout+76:	mov	ax,*6(bp)
_timeout+79:	mov	*2(di),ax
_timeout+7c:	push	*-6(bp)
_timeout+7f:	call	0
_timeout+82:	pop	cx
_timeout+83:	pop	cx
_timeout+84:	pop	di
_timeout+85:	pop	si
_timeout+86:	pop	bp
_timeout+87:	ret
_timecan:
_timecan:	push	bp
_timecan+1:	mov	bp,sp
_timecan+3:	push	si
_timecan+4:	push	di
_timecan+5:	sub	sp,*4
_timecan+8:	mov	*-6(bp),#0
_timecan+d:	mov	di,#0
_timecan+10:	call	0
_timecan+13:	mov	*-8(bp),ax
_timecan+16:	cmp	*4(di),*0
_timecan+1a:	je	_timecan+57
_timecan+1c:	lea	ax,*6(di)
_timecan+1f:	mov	si,ax
_timecan+21:	cmp	*-6(bp),*0
_timecan+25:	je	_timecan+39
_timecan+27:	mov	ax,*4(si)
_timecan+2a:	mov	*4(di),ax
_timecan+2d:	mov	ax,*2(si)
_timecan+30:	mov	*2(di),ax
_timecan+33:	mov	ax,(si)
_timecan+35:	mov	(di),ax
_timecan+37:	j	_timecan+52
_timecan+39:	mov	ax,*4(di)
_timecan+3c:	cmp	ax,*4(bp)
_timecan+3f:	jne	_timecan+52
_timecan+41:	mov	ax,*2(di)
_timecan+44:	cmp	ax,*6(bp)
_timecan+47:	jne	_timecan+52
_timecan+49:	inc	*-6(bp)
_timecan+4c:	mov	ax,(di)
_timecan+4e:	add	(si),ax
_timecan+50:	j	_timecan+16
_timecan+52:	add	di,*6
_timecan+55:	j	_timecan+16
_timecan+57:	push	*-8(bp)
_timecan+5a:	call	0
_timecan+5d:	pop	cx
_timecan+5e:	lea	sp,*-4(bp)
_timecan+61:	pop	di
_timecan+62:	pop	si
_timecan+63:	pop	bp
_timecan+64:	ret
