_ioctl:		push	bp
_ioctl+1:	mov	bp,sp
_ioctl+3:	push	si
_ioctl+4:	push	di
_ioctl+5:	push	cx
_ioctl+6:	mov	bx,128
_ioctl+a:	push	(bx)
_ioctl+c:	call	0
_ioctl+f:	pop	cx
_ioctl+10:	mov	di,ax
_ioctl+12:	or	ax,ax
_ioctl+14:	je	_ioctl+54
_ioctl+16:	mov	si,*2(di)
_ioctl+19:	mov	ax,*6(si)
_ioctl+1c:	and	ax,#6000
_ioctl+1f:	cmp	ax,#2000
_ioctl+22:	je	_ioctl+2b
_ioctl+24:	movb	71,*19
_ioctl+29:	j	_ioctl+7d
_ioctl+2b:	mov	bx,128
_ioctl+2f:	mov	(bx),#0
_ioctl+33:	mov	ax,*14(si)
_ioctl+36:	mov	*-6(bp),ax
_ioctl+39:	mov	bx,128
_ioctl+3d:	cmp	*-12(bx),#6100
_ioctl+42:	jne	_ioctl+56
_ioctl+44:	mov	bx,128
_ioctl+48:	push	*-4(bx)
_ioctl+4b:	push	*-6(bp)
_ioctl+4e:	call	0
_ioctl+51:	add	sp,*4
_ioctl+54:	j	_ioctl+7d
_ioctl+56:	movb	al,(di)
_ioctl+58:	cbw
_ioctl+59:	push	ax
_ioctl+5a:	mov	bx,128
_ioctl+5e:	push	*-4(bx)
_ioctl+61:	mov	bx,128
_ioctl+65:	push	*-12(bx)
_ioctl+68:	push	*-6(bp)
_ioctl+6b:	movb	al,*-5(bp)
_ioctl+6e:	cbw
_ioctl+6f:	mov	dx,#a
_ioctl+72:
data address not found
_ioctl+74:	mov	bx,ax
_ioctl+76:	call	#8(bx)
_ioctl+7a:	add	sp,*8
_ioctl+7d:	pop	cx
_ioctl+7e:	pop	di
_ioctl+7f:	pop	si
_ioctl+80:	pop	bp
_ioctl+81:	ret
_ttioctl:
_ttioctl:	push	bp
_ttioctl+1:	mov	bp,sp
_ttioctl+3:	push	si
_ttioctl+4:	push	di
_ttioctl+5:	sub	sp,*6
_ttioctl+8:	mov	di,*6(bp)
_ttioctl+b:	mov	ax,*4(bp)
_ttioctl+e:	jmp	_ttioctl+ad
_ttioctl+11:	mov	ax,*6(di)
_ttioctl+14:	mov	cx,#8
_ttioctl+17:	shl	ax,cl
_ttioctl+19:	or	ax,(di)
_ttioctl+1b:	mov	*-10(bp),ax
_ttioctl+1e:	j	_ttioctl+83
_ttioctl+20:	or	*14(di),#80
_ttioctl+25:	j	_ttioctl+4d
_ttioctl+27:	and	*14(di),#ff7f
_ttioctl+2c:	j	_ttioctl+4d
_ttioctl+2e:	push	di
_ttioctl+2f:	call	_wflusht
_ttioctl+32:	pop	cx
_ttioctl+33:	mov	ax,#6
_ttioctl+36:	push	ax
_ttioctl+37:	lea	ax,*-10(bp)
_ttioctl+3a:	push	ax
_ttioctl+3b:	push	*8(bp)
_ttioctl+3e:	call	0
_ttioctl+41:	add	sp,*6
_ttioctl+44:	or	ax,ax
_ttioctl+46:	je	_ttioctl+4f
_ttioctl+48:	movb	71,*e
_ttioctl+4d:	j	_ttioctl+a4
_ttioctl+4f:	mov	ax,*-10(bp)
_ttioctl+52:	mov	*24(di),ax
_ttioctl+55:	movb	al,*-8(bp)
_ttioctl+58:	movb	*22(di),al
_ttioctl+5b:	movb	al,*-7(bp)
_ttioctl+5e:	movb	*23(di),al
_ttioctl+61:	mov	ax,*-6(bp)
_ttioctl+64:	mov	*12(di),ax
_ttioctl+67:	xor	ax,ax
_ttioctl+69:	j	_ttioctl+e6
_ttioctl+6b:	mov	ax,*24(di)
_ttioctl+6e:	mov	*-10(bp),ax
_ttioctl+71:	movb	al,*22(di)
_ttioctl+74:	movb	*-8(bp),al
_ttioctl+77:	movb	al,*23(di)
_ttioctl+7a:	movb	*-7(bp),al
_ttioctl+7d:	mov	ax,*12(di)
_ttioctl+80:	mov	*-6(bp),ax
_ttioctl+83:	mov	ax,#6
_ttioctl+86:	push	ax
_ttioctl+87:	push	*8(bp)
_ttioctl+8a:	lea	ax,*-10(bp)
_ttioctl+8d:	push	ax
_ttioctl+8e:	call	0
_ttioctl+91:	add	sp,*6
_ttioctl+94:	or	ax,ax
_ttioctl+96:	je	_ttioctl+e3
_ttioctl+98:	movb	71,*e
_ttioctl+9d:	j	_ttioctl+e3
_ttioctl+9f:	or	*14(di),#200
_ttioctl+a4:	j	_ttioctl+e3
_ttioctl+a6:	push	di
_ttioctl+a7:	call	_flushtt
_ttioctl+aa:	pop	cx
_ttioctl+ab:	j	_ttioctl+e3
_ttioctl+ad:	cmp	ax,#740a
_ttioctl+b0:	je	_ttioctl+33
_ttioctl+b2:	jnle	_ttioctl+c6
_ttioctl+b4:	cmp	ax,#7402
_ttioctl+b7:	je	_ttioctl+9f
_ttioctl+b9:	cmp	ax,#7408
_ttioctl+bc:	je	_ttioctl+6b
_ttioctl+be:	cmp	ax,#7409
_ttioctl+c1:	jne	_ttioctl+c6
_ttioctl+c3:	jmp	_ttioctl+2e
_ttioctl+c6:	cmp	ax,#740d
_ttioctl+c9:	jne	_ttioctl+ce
_ttioctl+cb:	jmp	_ttioctl+20
_ttioctl+ce:	cmp	ax,#740e
_ttioctl+d1:	jne	_ttioctl+d6
_ttioctl+d3:	jmp	_ttioctl+27
_ttioctl+d6:	cmp	ax,#7410
_ttioctl+d9:	je	_ttioctl+a6
_ttioctl+db:	cmp	ax,#741e
_ttioctl+de:	jne	_ttioctl+e3
_ttioctl+e0:	jmp	_ttioctl+11
_ttioctl+e3:	mov	ax,#1
_ttioctl+e6:	lea	sp,*-4(bp)
_ttioctl+e9:	pop	di
_ttioctl+ea:	pop	si
_ttioctl+eb:	pop	bp
_ttioctl+ec:	ret
_ttopen:
_ttopen:	push	bp
_ttopen+1:	mov	bp,sp
_ttopen+3:	push	si
_ttopen+4:	push	di
_ttopen+5:	mov	di,*4(bp)
_ttopen+8:
data address not found
_ttopen+a:	push	cs
_ttopen+b:	addb	(bx+si),*74
_ttopen+e:	orb	al,dh
_ttopen+10:	push	es
_ttopen+11:	jno	_ttopen+13
_ttopen+13:	push	es
_ttopen+14:	mov	ax,#ffff
_ttopen+17:	j	_ttopen+55
_ttopen+19:	mov	bx,76
_ttopen+1d:	cmp	*26(bx),*0
_ttopen+21:	jne	_ttopen+2a
_ttopen+23:	mov	bx,76
_ttopen+27:	mov	*26(bx),di
_ttopen+2a:
data address not found
_ttopen+2c:	push	cs
_ttopen+2d:	push	es
_ttopen+2e:	addb	*34(di),dh
_ttopen+31:	mov	*14(di),#4
_ttopen+36:	mov	*12(di),#80d8
_ttopen+3b:	movb	*22(di),*8
_ttopen+3f:	movb	*23(di),*15
_ttopen+43:	cmp	*24(di),*0
_ttopen+47:	jne	_ttopen+4e
_ttopen+49:	mov	*24(di),#d0d
_ttopen+4e:	mov	ax,#1
_ttopen+51:	j	_ttopen+55
_ttopen+53:	xor	ax,ax
_ttopen+55:	pop	di
_ttopen+56:	pop	si
_ttopen+57:	pop	bp
_ttopen+58:	ret
_cinit:
_cinit:		push	bp
_cinit+1:	mov	bp,sp
_cinit+3:	push	si
_cinit+4:	push	di
_cinit+5:	push	cx
_cinit+6:	mov	ax,#3f
_cinit+9:	and	ax,#ffc0
_cinit+c:	mov	di,ax
_cinit+e:	cmp	di,0
_cinit+12:	jnbe	_cinit+22
_cinit+14:	mov	ax,0
_cinit+17:	mov	(di),ax
_cinit+19:	mov	0,di
_cinit+1d:	add	di,*40
_cinit+20:	j	_cinit+e
_cinit+22:	mov	*-6(bp),#0
_cinit+27:	mov	si,#0
_cinit+2a:	cmp	(si),*0
_cinit+2d:	je	_cinit+37
_cinit+2f:	inc	*-6(bp)
_cinit+32:	add	si,*a
_cinit+35:	j	_cinit+2a
_cinit+37:	mov	ax,*-6(bp)
_cinit+3a:	mov	0,ax
_cinit+3d:	pop	cx
_cinit+3e:	pop	di
_cinit+3f:	pop	si
_cinit+40:	pop	bp
_cinit+41:	ret
_ttyinpu:
_ttyinpu:	push	bp
_ttyinpu+1:	mov	bp,sp
_ttyinpu+3:	push	si
_ttyinpu+4:	push	di
_ttyinpu+5:	sub	sp,*4
_ttyinpu+8:	mov	di,#6(bp)
_ttyinpu+c:	and	*4(bp),#ff
_ttyinpu+11:	mov	ax,*4(bp)
_ttyinpu+14:	mov	*-6(bp),ax
_ttyinpu+17:	cmp	(di),#100
_ttyinpu+1b:	jle	_ttyinpu+22
_ttyinpu+1d:	push	di
_ttyinpu+1e:	call	_flushtt
_ttyinpu+21:	pop	cx
_ttyinpu+22:	mov	si,*12(di)
_ttyinpu+25:
data address not found
_ttyinpu+27:	andb	(bx+si),al
_ttyinpu+29:	je	_ttyinpu+2e
_ttyinpu+2b:	jmp	462
_ttyinpu+2e:	movb	*21(di),*0
_ttyinpu+32:	mov	ax,*-6(bp)
_ttyinpu+35:	jmp	450
_ttyinpu+38:	cmp	*-6(bp),*3
_ttyinpu+3c:	jne	_ttyinpu+43
_ttyinpu+3e:	mov	ax,#2
_ttyinpu+41:	j	_ttyinpu+46
_ttyinpu+43:	mov	ax,#3
_ttyinpu+46:	push	ax
_ttyinpu+47:	push	di
_ttyinpu+48:	call	0
_ttyinpu+4b:	add	sp,*4
_ttyinpu+4e:	push	di
_ttyinpu+4f:	call	_flushtt
_ttyinpu+52:	pop	cx
_ttyinpu+53:	jmp	329
_ttyinpu+56:	push	di
_ttyinpu+57:	call	_getc
_ttyinpu+5a:	pop	cx
_ttyinpu+5b:	or	ax,ax
_ttyinpu+5d:	jnl	_ttyinpu+62
_ttyinpu+5f:	jmp	329
_ttyinpu+62:	j	_ttyinpu+56
_ttyinpu+64:	cmp	*6(di),*64
_ttyinpu+68:	jnl	_ttyinpu+d9
_ttyinpu+6a:
data address not found
_ttyinpu+6c:	orb	(bx+si),al
_ttyinpu+6e:	je	_ttyinpu+98
_ttyinpu+70:	push	di
_ttyinpu+71:	push	*-6(bp)
_ttyinpu+74:	call	_echo
_ttyinpu+77:	add	sp,*4
_ttyinpu+7a:	push	di
_ttyinpu+7b:	mov	ax,#a
_ttyinpu+7e:	push	ax
_ttyinpu+7f:	call	_echo
_ttyinpu+82:	add	sp,*4
_ttyinpu+85:	mov	ax,*2(di)
_ttyinpu+88:	mov	*-8(bp),ax
_ttyinpu+8b:	mov	ax,(di)
_ttyinpu+8d:	mov	*-6(bp),ax
_ttyinpu+90:	mov	ax,*-6(bp)
_ttyinpu+93:	dec	*-6(bp)
_ttyinpu+96:	or	ax,ax
_ttyinpu+98:	je	_ttyinpu+e0
_ttyinpu+9a:	push	di
_ttyinpu+9b:	mov	bx,*-8(bp)
_ttyinpu+9e:	movb	al,(bx)
_ttyinpu+a0:	cbw
_ttyinpu+a1:	push	ax
_ttyinpu+a2:	call	_echo
_ttyinpu+a5:	add	sp,*4
_ttyinpu+a8:	inc	*-8(bp)
_ttyinpu+ab:	mov	ax,*-8(bp)
_ttyinpu+ae:	test	ax,#3f
_ttyinpu+b1:	jne	_ttyinpu+90
_ttyinpu+b3:	mov	bx,*-8(bp)
_ttyinpu+b6:	mov	ax,*-64(bx)
_ttyinpu+b9:	inc	ax
_ttyinpu+ba:	inc	ax
_ttyinpu+bb:	mov	*-8(bp),ax
_ttyinpu+be:	j	_ttyinpu+90
_ttyinpu+c0:	and	*14(di),#feff
_ttyinpu+c5:
data address not found
_ttyinpu+c7:	push	cs
_ttyinpu+c8:	inc	ax
_ttyinpu+c9:	addb	*8(si),dh
_ttyinpu+cc:	lea	ax,*6(di)
_ttyinpu+cf:	push	ax
_ttyinpu+d0:	call	0
_ttyinpu+d3:	pop	cx
_ttyinpu+d4:	push	di
_ttyinpu+d5:	call	*16(di)
_ttyinpu+d8:	pop	cx
_ttyinpu+d9:	j	_ttyinpu+e0
_ttyinpu+db:	or	*14(di),#100
_ttyinpu+e0:	j	344
_ttyinpu+e2:
data address not found
_ttyinpu+e4:	adcb	(bx+si),al
_ttyinpu+e6:	je	_ttyinpu+f2
_ttyinpu+e8:	mov	*4(bp),#a
_ttyinpu+ed:	mov	*-6(bp),#a
_ttyinpu+f2:
data address not found
_ttyinpu+f4:	push	cs
_ttyinpu+f5:	addb	*116(bx+si),al
_ttyinpu+f8:	orb	dl,*-24(bx)
_ttyinpu+fb:	stc
_ttyinpu+fc:	cld
_ttyinpu+fd:	pop	cx
_ttyinpu+fe:	and	*14(di),#bfff
30d:		jmp	46a
310:
data address not found
312:		addb	al,(bx+si)
314:		jne	34d
316:		movb	al,*23(di)
319:		cbw
31a:		cmp	*-6(bp),ax
31d:		jne	346
31f:		push	di
320:		call	_unputc
323:		pop	cx
324:		cmp	ax,#ffff
327:		jne	31f
329:
data address not found
32b:		orb	(bx+si),al
32d:		je	363
32f:		push	di
330:		push	*-6(bp)
333:		call	_echo
336:		add	sp,*4
339:		push	di
33a:		mov	ax,#a
33d:		push	ax
33e:		call	_echo
341:		add	sp,*4
344:		j	3c2
346:		movb	al,*22(di)
349:		cbw
34a:		cmp	*-6(bp),ax
34d:		je	352
34f:		jmp	3d2
352:		push	di
353:		call	_unputc
356:		pop	cx
357:		mov	*-6(bp),ax
35a:		cmp	ax,#ffff
35d:		je	363
35f:
data address not found
361:		orb	(bx+si),al
363:		je	3c2
365:
data address not found
367:		addb	#5974(bx+si),al
36b:		cmp	*-6(bp),*1f
36f:		jnle	3a1
371:		cmp	*-6(bp),*1b
375:		je	3a1
377:		cmp	*-6(bp),*9
37b:		jne	380
37d:		jmp	_ttyinpu+7a
380:		push	di
381:		mov	ax,#8
384:		push	ax
385:		call	_echo
388:		add	sp,*4
38b:		push	di
38c:		mov	ax,#20
38f:		push	ax
390:		call	_echo
393:		add	sp,*4
396:		push	di
397:		mov	ax,#8
39a:		push	ax
39b:		call	_echo
39e:		add	sp,*4
3a1:		push	di
3a2:		mov	ax,#8
3a5:		push	ax
3a6:		call	_echo
3a9:		add	sp,*4
3ac:		push	di
3ad:		mov	ax,#20
3b0:		push	ax
3b1:		call	_echo
3b4:		add	sp,*4
3b7:		push	di
3b8:		mov	ax,#8
3bb:		push	ax
3bc:		call	_echo
3bf:		add	sp,*4
3c2:		j	3cf
3c4:		push	di
3c5:		mov	ax,#3c
3c8:		push	ax
3c9:		call	_echo
3cc:		add	sp,*4
3cf:		jmp	4b2
3d2:
data address not found
3d4:		addb	al,*0
3d6:		je	43a
3d8:		cmp	*-6(bp),*5a
3dc:		jnle	3e8
3de:		cmp	*-6(bp),*41
3e2:		jl	3e8
3e4:		sub	*-6(bp),*e0
3e8:
data address not found
3ea:		push	cs
3eb:		addb	(bx+si),dl
3ed:		je	43c
3ef:		cmp	*-6(bp),*7a
3f3:		jnle	406
3f5:		cmp	*-6(bp),*61
3f9:		jl	406
3fb:		push	di
3fc:		call	_unputc
3ff:		pop	cx
400:		add	*-6(bp),*e0
404:		j	435
406:		mov	*-8(bp),#ba8
40b:		mov	bx,*-8(bp)
40e:		inc	*-8(bp)
411:		cmpb	(bx),*0
414:		je	435
416:		mov	bx,*-8(bp)
419:		inc	*-8(bp)
41c:		movb	al,(bx)
41e:		cbw
41f:		cmp	*-6(bp),ax
422:		jne	40b
424:		push	di
425:		call	_unputc
428:		pop	cx
429:		mov	bx,*-8(bp)
42c:		movb	al,*-2(bx)
42f:		cbw
430:		mov	*-6(bp),ax
433:		j	40b
435:		and	*14(di),#efff
43a:		j	447
43c:		cmp	*-6(bp),*1b
440:		jne	447
442:		or	*14(di),#1000
447:
data address not found
449:		addb	al,(bx+si)
44b:		je	46a
44d:		jmp	_ttyinpu+f2
450:		sub	ax,#3
453:		cmp	ax,#17
456:		jbe	45b
458:		jmp	310
45b:		shl	ax
45d:		xchg	bx
45e:		jmp	#b6c(bx)
462:		dec	*18(di)
465:		jne	46a
467:		jmp	_ttyinpu+f2
46a:
data address not found
46c:		add	(bx+si),ax
46e:		je	498
470:		cmp	(di),#80
474:		jle	493
476:		lea	ax,*6(di)
479:		push	ax
47a:		mov	ax,#13
47d:		push	ax
47e:		call	_putc
481:		add	sp,*4
484:		and	*14(di),#feff
489:		or	*14(di),#400
48e:		push	di
48f:		call	*16(di)
492:		pop	cx
493:		push	di
494:		call	_ttybloc
497:		pop	cx
498:		push	di
499:		push	*-6(bp)
49c:		call	_putc
49f:		add	sp,*4
4a2:
data address not found
4a4:		orb	(bx+si),al
4a6:		je	4b2
4a8:		push	di
4a9:		push	*4(bp)
4ac:		call	_echo
4af:		add	sp,*4
4b2:		lea	sp,*-4(bp)
4b5:		pop	di
4b6:		pop	si
4b7:		pop	bp
4b8:		ret
_getc:
_getc:		push	bp
_getc+1:	mov	bp,sp
_getc+3:	push	si
_getc+4:	push	di
_getc+5:	sub	sp,*4
_getc+8:	mov	di,*4(bp)
_getc+b:	call	0
_getc+e:	mov	*-8(bp),ax
_getc+11:	cmp	(di),*0
_getc+14:	jnle	_getc+2b
_getc+16:	mov	*-6(bp),#ffff
_getc+1b:	mov	(di),#0
_getc+1f:	mov	*2(di),#0
_getc+24:	mov	*4(di),#0
_getc+29:	j	_getc+7b
_getc+2b:	mov	bx,*2(di)
_getc+2e:	movb	al,(bx)
_getc+30:	and	ax,#ff
_getc+33:	mov	*-6(bp),ax
_getc+36:	inc	*2(di)
_getc+39:	dec	(di)
_getc+3b:	jnle	_getc+5d
_getc+3d:	mov	ax,*2(di)
_getc+40:	dec	ax
_getc+41:	mov	si,ax
_getc+43:	and	ax,#ffc0
_getc+46:	mov	si,ax
_getc+48:	mov	*2(di),#0
_getc+4d:	mov	*4(di),#0
_getc+52:	mov	ax,0
_getc+55:	mov	(si),ax
_getc+57:	mov	0,si
_getc+5b:	j	_getc+7b
_getc+5d:	mov	ax,*2(di)
_getc+60:	test	ax,#3f
_getc+63:	jne	_getc+7b
_getc+65:	mov	si,*2(di)
_getc+68:	sub	si,*40
_getc+6b:	mov	ax,(si)
_getc+6d:	inc	ax
_getc+6e:	inc	ax
_getc+6f:	mov	*2(di),ax
_getc+72:	mov	ax,0
_getc+75:	mov	(si),ax
_getc+77:	mov	0,si
_getc+7b:	push	*-8(bp)
_getc+7e:	call	0
_getc+81:	pop	cx
_getc+82:	mov	ax,*-6(bp)
_getc+85:	lea	sp,*-4(bp)
_getc+88:	pop	di
_getc+89:	pop	si
_getc+8a:	pop	bp
_getc+8b:	ret
_putc:
_putc:		push	bp
_putc+1:	mov	bp,sp
_putc+3:	push	si
_putc+4:	push	di
_putc+5:	sub	sp,*4
_putc+8:	mov	di,*6(bp)
_putc+b:	call	0
_putc+e:	mov	*-8(bp),ax
_putc+11:	mov	ax,*4(di)
_putc+14:	mov	*-6(bp),ax
_putc+17:	or	ax,ax
_putc+19:	je	_putc+20
_putc+1b:	cmp	(di),*0
_putc+1e:	jnl	_putc+48
_putc+20:	mov	si,0
_putc+24:	or	si,si
_putc+26:	jne	_putc+34
_putc+28:	push	*-8(bp)
_putc+2b:	call	0
_putc+2e:	pop	cx
_putc+2f:	mov	ax,#ffff
_putc+32:	j	_putc+6b
_putc+34:	mov	ax,(si)
_putc+36:	mov	0,ax
_putc+39:	mov	(si),#0
_putc+3d:	lea	ax,*2(si)
_putc+40:	mov	*-6(bp),ax
_putc+43:	mov	*2(di),ax
_putc+46:	j	_putc+7e
_putc+48:	mov	ax,*-6(bp)
_putc+4b:	test	ax,#3f
_putc+4e:	jne	_putc+7e
_putc+50:	mov	ax,*-6(bp)
_putc+53:	sub	ax,#40
_putc+56:	mov	si,ax
_putc+58:	mov	ax,0
_putc+5b:	mov	(si),ax
_putc+5d:	or	ax,ax
_putc+5f:	jne	_putc+6d
_putc+61:	push	*-8(bp)
_putc+64:	call	0
_putc+67:	pop	cx
_putc+68:	mov	ax,#ffff
_putc+6b:	j	_putc+9c
_putc+6d:	mov	si,(si)
_putc+6f:	mov	ax,(si)
_putc+71:	mov	0,ax
_putc+74:	mov	(si),#0
_putc+78:	lea	ax,*2(si)
_putc+7b:	mov	*-6(bp),ax
_putc+7e:	mov	bx,*-6(bp)
_putc+81:	mov	ax,*4(bp)
_putc+84:	mov	cx,ax
_putc+86:	movb	(bx),cl
_putc+88:	inc	*-6(bp)
_putc+8b:	inc	(di)
_putc+8d:	mov	ax,*-6(bp)
_putc+90:	mov	*4(di),ax
_putc+93:	push	*-8(bp)
_putc+96:	call	0
_putc+99:	pop	cx
_putc+9a:	xor	ax,ax
_putc+9c:	lea	sp,*-4(bp)
_putc+9f:	pop	di
_putc+a0:	pop	si
_putc+a1:	pop	bp
_putc+a2:	ret
_unputc:
_unputc:	push	bp
_unputc+1:	mov	bp,sp
_unputc+3:	push	si
_unputc+4:	push	di
_unputc+5:	sub	sp,*4
_unputc+8:	mov	di,*4(bp)
_unputc+b:	mov	si,*4(di)
_unputc+e:	cmp	(di),*0
_unputc+11:	je	_unputc+21
_unputc+13:	dec	si
_unputc+14:	movb	al,(si)
_unputc+16:	cbw
_unputc+17:	mov	cx,ax
_unputc+19:	movb	*-7(bp),cl
_unputc+1c:	cmpb	cl,*a
_unputc+1f:	jne	_unputc+26
_unputc+21:	mov	ax,#ffff
_unputc+24:	j	_unputc+5d
_unputc+26:	cmp	(di),*1
_unputc+29:	jne	_unputc+32
_unputc+2b:	push	di
_unputc+2c:	call	_getc
_unputc+2f:	pop	cx
_unputc+30:	j	_unputc+5d
_unputc+32:	mov	ax,si
_unputc+34:	and	ax,#3f
_unputc+37:	cmp	ax,#2
_unputc+3a:	jne	_unputc+77
_unputc+3c:	mov	ax,*2(di)
_unputc+3f:	and	ax,#ffc0
_unputc+42:	mov	*-6(bp),ax
_unputc+45:	dec	si
_unputc+46:	dec	si
_unputc+47:	mov	bx,*-6(bp)
_unputc+4a:	cmp	(bx),si
_unputc+4c:	je	_unputc+5f
_unputc+4e:	mov	bx,*-6(bp)
_unputc+51:	mov	ax,(bx)
_unputc+53:	mov	*-6(bp),ax
_unputc+56:	or	ax,ax
_unputc+58:	jne	_unputc+47
_unputc+5a:	mov	ax,#ffff
_unputc+5d:	j	_unputc+82
_unputc+5f:	mov	ax,0
_unputc+62:	mov	(si),ax
_unputc+64:	mov	0,si
_unputc+68:	mov	bx,*-6(bp)
_unputc+6b:	mov	(bx),#0
_unputc+6f:	mov	ax,*-6(bp)
_unputc+72:	add	ax,#40
_unputc+75:	mov	si,ax
_unputc+77:	mov	*4(di),si
_unputc+7a:	dec	(di)
_unputc+7c:	movb	al,*-7(bp)
_unputc+7f:	and	ax,#ff
_unputc+82:	lea	sp,*-4(bp)
_unputc+85:	pop	di
_unputc+86:	pop	si
_unputc+87:	pop	bp
_unputc+88:	ret
_flushtt:
_flushtt:	push	bp
_flushtt+1:	mov	bp,sp
_flushtt+3:	push	si
_flushtt+4:	push	di
_flushtt+5:	mov	di,*4(bp)
_flushtt+8:	call	0
_flushtt+b:	mov	si,ax
_flushtt+d:	and	*14(di),#feff
_flushtt+12:	lea	ax,*6(di)
_flushtt+15:	push	ax
_flushtt+16:	call	_getc
_flushtt+19:	pop	cx
_flushtt+1a:	or	ax,ax
_flushtt+1c:	jnl	_flushtt+12
_flushtt+1e:	push	di
_flushtt+1f:	call	_getc
_flushtt+22:	pop	cx
_flushtt+23:	or	ax,ax
_flushtt+25:	jnl	_flushtt+1e
_flushtt+27:	push	si
_flushtt+28:	call	0
_flushtt+2b:	pop	cx
_flushtt+2c:	pop	di
_flushtt+2d:	pop	si
_flushtt+2e:	pop	bp
_flushtt+2f:	ret
_wflusht:
_wflusht:	push	bp
_wflusht+1:	mov	bp,sp
_wflusht+3:	push	si
_wflusht+4:	push	di
_wflusht+5:	mov	di,*4(bp)
_wflusht+8:	call	0
_wflusht+b:	cmp	*6(di),*0
_wflusht+f:	je	_wflusht+26
_wflusht+11:	or	*14(di),#40
_wflusht+16:	mov	ax,#14
_wflusht+19:	push	ax
_wflusht+1a:	lea	ax,*6(di)
_wflusht+1d:	push	ax
_wflusht+1e:	call	0
_wflusht+21:	add	sp,*4
_wflusht+24:	j	_wflusht+b
_wflusht+26:	push	di
_wflusht+27:	call	_flushtt
_wflusht+2a:	pop	cx
_wflusht+2b:	call	0
_wflusht+2e:	pop	di
_wflusht+2f:	pop	si
_wflusht+30:	pop	bp
_wflusht+31:	ret
_echo:
_echo:		push	bp
_echo+1:	mov	bp,sp
_echo+3:	push	si
_echo+4:	push	di
_echo+5:	mov	di,#4(bp)
_echo+9:	mov	si,#6(bp)
_echo+d:	and	*14(si),#feff
_echo+12:	cmp	di,*1b
_echo+15:	jne	_echo+1c
_echo+17:	mov	di,#24
_echo+1a:	j	_echo+43
_echo+1c:	cmp	di,*20
_echo+1f:	jnb	_echo+43
_echo+21:	cmp	di,*d
_echo+24:	je	_echo+43
_echo+26:	cmp	di,*a
_echo+29:	je	_echo+43
_echo+2b:	cmp	di,*9
_echo+2e:	je	_echo+43
_echo+30:	cmp	di,*8
_echo+33:	je	_echo+43
_echo+35:	push	si
_echo+36:	mov	ax,#5e
_echo+39:	push	ax
_echo+3a:	call	_ttyoutp
_echo+3d:	add	sp,*4
_echo+40:	add	di,*40
_echo+43:	push	si
_echo+44:	push	di
_echo+45:	call	_ttyoutp
_echo+48:	add	sp,*4
_echo+4b:	push	si
_echo+4c:	call	*16(si)
_echo+4f:	pop	cx
_echo+50:	pop	di
_echo+51:	pop	si
_echo+52:	pop	bp
_echo+53:	ret
_ttyoutp:
_ttyoutp:	push	bp
_ttyoutp+1:	mov	bp,sp
_ttyoutp+3:	push	si
_ttyoutp+4:	push	di
_ttyoutp+5:	mov	di,#6(bp)
_ttyoutp+9:
data address not found
_ttyoutp+b:	orb	al,*20
_ttyoutp+d:	addb	*55(di),dh
_ttyoutp+10:	lea	ax,*20(di)
_ttyoutp+13:	mov	si,ax
_ttyoutp+15:	movb	al,*4(bp)
_ttyoutp+18:	and	ax,#ff
_ttyoutp+1b:	jmp	_ttyoutp+c3
_ttyoutp+1e:	mov	ax,*12(di)
_ttyoutp+21:	and	ax,#c00
_ttyoutp+24:	cmp	ax,#c00
_ttyoutp+27:	jne	_ttyoutp+3f
_ttyoutp+29:	push	di
_ttyoutp+2a:	mov	ax,#20
_ttyoutp+2d:	push	ax
_ttyoutp+2e:	call	_ttyoutp
_ttyoutp+31:	add	sp,*4
_ttyoutp+34:	movb	al,(si)
_ttyoutp+36:	cbw
_ttyoutp+37:	test	ax,#7
_ttyoutp+3a:	jne	_ttyoutp+29
_ttyoutp+3c:	jmp	_ttyoutp+e3
_ttyoutp+3f:	orb	(si),*7
_ttyoutp+42:	j	_ttyoutp+6e
_ttyoutp+44:	movb	(si),*0
_ttyoutp+47:	j	_ttyoutp+6e
_ttyoutp+49:
data address not found
_ttyoutp+4b:	orb	al,*10
_ttyoutp+4d:	addb	*11(si),dh
_ttyoutp+50:	push	di
_ttyoutp+51:	mov	ax,#d
_ttyoutp+54:	push	ax
_ttyoutp+55:	call	_ttyoutp
_ttyoutp+58:	add	sp,*4
_ttyoutp+5b:
data address not found
_ttyoutp+5d:	orb	al,*0
_ttyoutp+5f:	inc	ax
_ttyoutp+60:	je	_ttyoutp+98
_ttyoutp+62:	incb	*21(di)
_ttyoutp+65:	j	_ttyoutp+6e
_ttyoutp+67:	cmpb	(si),*0
_ttyoutp+6a:	je	_ttyoutp+98
_ttyoutp+6c:	decb	(si)
_ttyoutp+6e:	j	_ttyoutp+c1
_ttyoutp+70:	cmpb	*4(bp),*1f
_ttyoutp+74:	jb	_ttyoutp+c1
_ttyoutp+76:	incb	(si)
_ttyoutp+78:
data address not found
_ttyoutp+7a:	orb	al,*4
_ttyoutp+7c:	addb	*66(si),dh
_ttyoutp+7f:	cmpb	*4(bp),*61
_ttyoutp+83:	jb	_ttyoutp+8f
_ttyoutp+85:	cmpb	*4(bp),*7a
_ttyoutp+89:	jnbe	_ttyoutp+8f
_ttyoutp+8b:	addb	*4(bp),*e0
_ttyoutp+8f:	mov	si,#bb3
_ttyoutp+92:	mov	bx,si
_ttyoutp+94:	inc	si
_ttyoutp+95:	cmpb	(bx),*0
_ttyoutp+98:	je	_ttyoutp+c1
_ttyoutp+9a:	movb	al,*4(bp)
_ttyoutp+9d:	and	ax,#ff
_ttyoutp+a0:	mov	bx,si
_ttyoutp+a2:	inc	si
_ttyoutp+a3:	movb	dl,(bx)
_ttyoutp+a5:	mov	cx,#8
_ttyoutp+a8:	shl	dx,cl
_ttyoutp+aa:	sar	dx,cl
_ttyoutp+ac:	cmp	ax,dx
_ttyoutp+ae:	jne	_ttyoutp+92
_ttyoutp+b0:	push	di
_ttyoutp+b1:	mov	ax,#24
_ttyoutp+b4:	push	ax
_ttyoutp+b5:	call	_ttyoutp
_ttyoutp+b8:	add	sp,*4
_ttyoutp+bb:	movb	al,*-2(si)
_ttyoutp+be:	movb	*4(bp),al
_ttyoutp+c1:	j	_ttyoutp+d2
_ttyoutp+c3:	sub	ax,#8
_ttyoutp+c6:	cmp	ax,#5
_ttyoutp+c9:	jnbe	_ttyoutp+70
_ttyoutp+cb:	shl	ax
_ttyoutp+cd:	xchg	bx
_ttyoutp+ce:	jmp	#b9c(bx)
_ttyoutp+d2:	lea	ax,*6(di)
_ttyoutp+d5:	push	ax
_ttyoutp+d6:	movb	al,*4(bp)
_ttyoutp+d9:	and	ax,#ff
_ttyoutp+dc:	push	ax
_ttyoutp+dd:	call	_putc
_ttyoutp+e0:	add	sp,*4
_ttyoutp+e3:	pop	di
_ttyoutp+e4:	pop	si
_ttyoutp+e5:	pop	bp
_ttyoutp+e6:	ret
_ttwrite:
_ttwrite:	push	bp
_ttwrite+1:	mov	bp,sp
_ttwrite+3:	push	si
_ttwrite+4:	push	di
_ttwrite+5:	mov	di,#4(bp)
_ttwrite+9:	call	0
_ttwrite+c:	cmp	7a,*0
_ttwrite+11:	je	_ttwrite+6f
_ttwrite+13:	dec	7a
_ttwrite+17:	cmpb	*21(di),*14
_ttwrite+1b:	jbe	_ttwrite+32
_ttwrite+1d:	or	*14(di),#40
_ttwrite+22:	mov	ax,#14
_ttwrite+25:	push	ax
_ttwrite+26:	lea	ax,*6(di)
_ttwrite+29:	push	ax
_ttwrite+2a:	call	0
_ttwrite+2d:	add	sp,*4
_ttwrite+30:	j	_ttwrite+17
_ttwrite+32:	cmp	*6(di),*64
_ttwrite+36:	jle	_ttwrite+58
_ttwrite+38:	push	di
_ttwrite+39:	call	*16(di)
_ttwrite+3c:	pop	cx
_ttwrite+3d:	cmp	*6(di),*0
_ttwrite+41:	je	_ttwrite+32
_ttwrite+43:	or	*14(di),#40
_ttwrite+48:	mov	ax,#14
_ttwrite+4b:	push	ax
_ttwrite+4c:	lea	ax,*6(di)
_ttwrite+4f:	push	ax
_ttwrite+50:	call	0
_ttwrite+53:	add	sp,*4
_ttwrite+56:	j	_ttwrite+32
_ttwrite+58:	mov	ax,78
_ttwrite+5b:	inc	78
_ttwrite+5f:	push	ax
_ttwrite+60:	call	0
_ttwrite+63:	pop	cx
_ttwrite+64:	mov	si,ax
_ttwrite+66:	or	ax,ax
_ttwrite+68:	jnl	_ttwrite+71
_ttwrite+6a:	movb	71,*e
_ttwrite+6f:	j	_ttwrite+d7
_ttwrite+71:
data address not found
_ttwrite+73:	orb	al,*20
_ttwrite+75:	addb	*85(si),dh
_ttwrite+78:	lea	ax,*6(di)
_ttwrite+7b:	push	ax
_ttwrite+7c:	push	si
_ttwrite+7d:	call	_putc
_ttwrite+80:	add	sp,*4
_ttwrite+83:	mov	ax,*10(di)
_ttwrite+86:	and	ax,#3f
_ttwrite+89:	mov	si,ax
_ttwrite+8b:	or	ax,ax
_ttwrite+8d:	jne	_ttwrite+92
_ttwrite+8f:	jmp	_ttwrite+c
_ttwrite+92:	cmp	7a,*0
_ttwrite+97:	je	_ttwrite+8d
_ttwrite+99:	mov	ax,#40
_ttwrite+9c:	sub	ax,si
_ttwrite+9e:	mov	si,ax
_ttwrite+a0:	cmp	si,7a
_ttwrite+a4:	jbe	_ttwrite+aa
_ttwrite+a6:	mov	si,7a
_ttwrite+aa:	push	si
_ttwrite+ab:	push	*10(di)
_ttwrite+ae:	push	78
_ttwrite+b2:	call	0
_ttwrite+b5:	add	sp,*6
_ttwrite+b8:	or	ax,ax
_ttwrite+ba:	jl	_ttwrite+6a
_ttwrite+bc:	sub	7a,si
_ttwrite+c0:	add	78,si
_ttwrite+c4:	add	*6(di),si
_ttwrite+c7:	add	*10(di),si
_ttwrite+ca:	jmp	_ttwrite+c
_ttwrite+cd:	push	di
_ttwrite+ce:	push	si
_ttwrite+cf:	call	_ttyoutp
_ttwrite+d2:	add	sp,*4
_ttwrite+d5:	j	_ttwrite+ca
_ttwrite+d7:	push	di
_ttwrite+d8:	call	*16(di)
_ttwrite+db:	pop	cx
_ttwrite+dc:	call	0
_ttwrite+df:	pop	di
_ttwrite+e0:	pop	si
_ttwrite+e1:	pop	bp
_ttwrite+e2:	ret
_ttread:
_ttread:	push	bp
_ttread+1:	mov	bp,sp
_ttread+3:	push	si
_ttread+4:	push	di
_ttread+5:	sub	sp,*6
_ttread+8:	mov	di,#4(bp)
_ttread+c:	call	0
_ttread+f:	push	di
_ttread+10:	call	_ttybloc
_ttread+13:	pop	cx
_ttread+14:	mov	ax,*12(di)
_ttread+17:	and	ax,#22
_ttread+1a:	jmp	a3b
_ttread+1d:	push	di
_ttread+1e:	call	_getc
_ttread+21:	pop	cx
_ttread+22:	mov	*-6(bp),ax
_ttread+25:	or	ax,ax
_ttread+27:	jl	_ttread+52
_ttread+29:	push	*-6(bp)
_ttread+2c:	mov	ax,78
_ttread+2f:	inc	78
_ttread+33:	push	ax
_ttread+34:	call	0
_ttread+37:	add	sp,*4
_ttread+3a:	or	ax,ax
_ttread+3c:	jnl	_ttread+43
_ttread+3e:	movb	71,*e
_ttread+43:	dec	7a
_ttread+47:	push	di
_ttread+48:	call	_ttybloc
_ttread+4b:	pop	cx
_ttread+4c:	call	0
_ttread+4f:	jmp	a67
_ttread+52:	mov	*18(di),#1
_ttread+57:	jmp	a39
_ttread+5a:	mov	ax,(di)
_ttread+5c:	mov	*-6(bp),ax
_ttread+5f:	mov	*-8(bp),ax
_ttread+62:	mov	si,*2(di)
_ttread+65:	mov	ax,*-6(bp)
_ttread+68:	dec	*-6(bp)
_ttread+6b:	or	ax,ax
_ttread+6d:	jne	_ttread+72
_ttread+6f:	jmp	a39
_ttread+72:	cmpb	(si),*a
_ttread+75:	je	_ttread+7c
_ttread+77:	cmpb	(si),*4
_ttread+7a:	jne	_ttread+93
_ttread+7c:	mov	ax,*-6(bp)
_ttread+7f:	sub	*-8(bp),ax
_ttread+82:	mov	ax,*-8(bp)
_ttread+85:	cmp	ax,7a
_ttread+89:	jbe	_ttread+aa
_ttread+8b:	mov	ax,7a
_ttread+8e:	mov	*-8(bp),ax
_ttread+91:	j	_ttread+aa
_ttread+93:	inc	si
_ttread+94:	mov	ax,si
_ttread+96:	test	ax,#3f
_ttread+99:	jne	_ttread+65
_ttread+9b:	mov	ax,*-64(si)
_ttread+9e:	inc	ax
_ttread+9f:	inc	ax
_ttread+a0:	mov	si,ax
_ttread+a2:	j	_ttread+65
_ttread+a4:	mov	ax,7a
_ttread+a7:	mov	*-8(bp),ax
_ttread+aa:	cmp	(di),*0
_ttread+ad:	jne	_ttread+b2
_ttread+af:	jmp	a27
_ttread+b2:	mov	ax,*2(di)
_ttread+b5:	and	ax,#3f
_ttread+b8:	mov	dx,#40
_ttread+bb:	sub	dx,ax
_ttread+bd:	mov	si,dx
_ttread+bf:	mov	ax,si
_ttread+c1:	cmp	ax,(di)
_ttread+c3:	jle	_ttread+c7
_ttread+c5:	mov	si,(di)
_ttread+c7:	mov	ax,si
_ttread+c9:	cmp	ax,*-8(bp)
_ttread+cc:	jle	_ttread+d1
_ttread+ce:	mov	si,*-8(bp)
_ttread+d1:	push	si
_ttread+d2:	push	78
_ttread+d6:	push	*2(di)
_ttread+d9:	call	0
_ttread+dc:	add	sp,*6
_ttread+df:	or	ax,ax
_ttread+e1:	jnl	_ttread+e6
_ttread+e3:	jmp	_ttread+3e
_ttread+e6:	mov	ax,si
_ttread+e8:	dec	ax
_ttread+e9:	mov	*-6(bp),ax
_ttread+ec:	or	ax,ax
_ttread+ee:	jle	_ttread+fb
_ttread+f0:	mov	ax,*-6(bp)
_ttread+f3:	add	*2(di),ax
_ttread+f6:	mov	ax,*-6(bp)
_ttread+f9:	sub	(di),ax
_ttread+fb:	push	di
_ttread+fc:	call	_getc
9f0:		pop	cx
9f1:		mov	*-10(bp),ax
9f4:		mov	ax,si
9f6:		sub	7a,ax
9fa:		mov	ax,si
9fc:		sub	*-8(bp),ax
9ff:		jne	a18
a01:		cmp	*-10(bp),*4
a05:		je	a0a
a07:		jmp	_ttread+47
a0a:
data address not found
a0c:		orb	al,*20
a0e:		addb	*-12(di),dh
a11:		inc	7a
a15:		jmp	_ttread+47
a18:		mov	ax,si
a1a:		add	78,ax
a1e:		call	0
a21:		call	0
a24:		jmp	_ttread+aa
a27:		cmp	7a,*28
a2c:		jnb	a33
a2e:		mov	ax,7a
a31:		j	a36
a33:		mov	ax,#28
a36:		mov	*18(di),ax
a39:		j	a4d
a3b:		or	ax,ax
a3d:		jne	a42
a3f:		jmp	_ttread+5a
a42:		cmp	ax,#20
a45:		jne	a4a
a47:		jmp	_ttread+a4
a4a:		jmp	_ttread+1d
a4d:
data address not found
a4f:		orb	al,*0
a51:		add	*-79(di),si
a54:		or	*14(di),#4000
a59:		mov	ax,#a
a5c:		push	ax
a5d:		push	di
a5e:		call	0
a61:		add	sp,*4
a64:		jmp	_ttread+f
a67:		lea	sp,*-4(bp)
a6a:		pop	di
a6b:		pop	si
a6c:		pop	bp
a6d:		ret
_ttybloc:
_ttybloc:	push	bp
_ttybloc+1:	mov	bp,sp
_ttybloc+3:	push	si
_ttybloc+4:	push	di
_ttybloc+5:	mov	di,#4(bp)
_ttybloc+9:
data address not found
_ttybloc+b:	push	cs
_ttybloc+c:	addb	(si),al
_ttybloc+e:	je	_ttybloc+2d
_ttybloc+10:	cmp	(di),*20
_ttybloc+13:	jnl	_ttybloc+2d
_ttybloc+15:	and	*14(di),#faff
_ttybloc+1a:	lea	ax,*6(di)
_ttybloc+1d:	push	ax
_ttybloc+1e:	mov	ax,#11
_ttybloc+21:	push	ax
_ttybloc+22:	call	_putc
_ttybloc+25:	add	sp,*4
_ttybloc+28:	push	di
_ttybloc+29:	call	*16(di)
_ttybloc+2c:	pop	cx
_ttybloc+2d:	pop	di
_ttybloc+2e:	pop	si
_ttybloc+2f:	pop	bp
_ttybloc+30:	ret
_ttyopen:
_ttyopen:	push	bp
_ttyopen+1:	mov	bp,sp
_ttyopen+3:	push	si
_ttyopen+4:	push	di
_ttyopen+5:	push	cx
_ttyopen+6:	mov	bx,76
_ttyopen+a:	cmp	*26(bx),*0
_ttyopen+e:	jne	_ttyopen+17
_ttyopen+10:	movb	71,*6
_ttyopen+15:	j	_ttyopen+3c
_ttyopen+17:	mov	bx,76
_ttyopen+1b:	mov	bx,*26(bx)
_ttyopen+1e:	mov	ax,*26(bx)
_ttyopen+21:	mov	*-6(bp),ax
_ttyopen+24:	push	*6(bp)
_ttyopen+27:	push	*-6(bp)
_ttyopen+2a:	movb	al,*-5(bp)
_ttyopen+2d:	cbw
_ttyopen+2e:	mov	dx,#a
_ttyopen+31:
data address not found
_ttyopen+33:	mov	bx,ax
_ttyopen+35:	call	#0(bx)
_ttyopen+39:	add	sp,*4
_ttyopen+3c:	pop	cx
_ttyopen+3d:	pop	di
_ttyopen+3e:	pop	si
_ttyopen+3f:	pop	bp
_ttyopen+40:	ret
_ttyread:
_ttyread:	push	bp
_ttyread+1:	mov	bp,sp
_ttyread+3:	push	si
_ttyread+4:	push	di
_ttyread+5:	push	cx
_ttyread+6:	mov	bx,76
_ttyread+a:	mov	bx,*26(bx)
_ttyread+d:	mov	ax,*26(bx)
_ttyread+10:	mov	*-6(bp),ax
_ttyread+13:	push	*-6(bp)
_ttyread+16:	movb	al,*-5(bp)
_ttyread+19:	cbw
_ttyread+1a:	mov	dx,#a
_ttyread+1d:
data address not found
_ttyread+1f:	mov	bx,ax
_ttyread+21:	call	#4(bx)
_ttyread+25:	pop	cx
_ttyread+26:	pop	cx
_ttyread+27:	pop	di
_ttyread+28:	pop	si
_ttyread+29:	pop	bp
_ttyread+2a:	ret
_ttywrit:
_ttywrit:	push	bp
_ttywrit+1:	mov	bp,sp
_ttywrit+3:	push	si
_ttywrit+4:	push	di
_ttywrit+5:	push	cx
_ttywrit+6:	mov	bx,76
_ttywrit+a:	mov	bx,*26(bx)
_ttywrit+d:	mov	ax,*26(bx)
_ttywrit+10:	mov	*-6(bp),ax
_ttywrit+13:	push	*-6(bp)
_ttywrit+16:	movb	al,*-5(bp)
_ttywrit+19:	cbw
_ttywrit+1a:	mov	dx,#a
_ttywrit+1d:
data address not found
_ttywrit+1f:	mov	bx,ax
_ttywrit+21:	call	#6(bx)
_ttywrit+25:	pop	cx
_ttywrit+26:	pop	cx
_ttywrit+27:	pop	di
_ttywrit+28:	pop	si
_ttywrit+29:	pop	bp
_ttywrit+2a:	ret
_ttyioct:
_ttyioct:	push	bp
_ttyioct+1:	mov	bp,sp
_ttyioct+3:	push	si
_ttyioct+4:	push	di
_ttyioct+5:	push	cx
_ttyioct+6:	mov	bx,76
_ttyioct+a:	mov	bx,*26(bx)
_ttyioct+d:	mov	ax,*26(bx)
_ttyioct+10:	mov	*-6(bp),ax
_ttyioct+13:	push	*10(bp)
_ttyioct+16:	push	*8(bp)
_ttyioct+19:	push	*6(bp)
_ttyioct+1c:	push	*-6(bp)
_ttyioct+1f:	movb	al,*-5(bp)
_ttyioct+22:	cbw
_ttyioct+23:	mov	dx,#a
_ttyioct+26:
data address not found
_ttyioct+28:	mov	bx,ax
_ttyioct+2a:	call	#8(bx)
_ttyioct+2e:	add	sp,*8
_ttyioct+31:	pop	cx
_ttyioct+32:	pop	di
_ttyioct+33:	pop	si
_ttyioct+34:	pop	bp
_ttyioct+35:	ret