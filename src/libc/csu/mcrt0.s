	| C runtime startoff including monitoring
cbufs	= 300

.globl	_monitor
.globl	_sbrk
.globl	_main
.globl	_exit
.globl	_environ
.globl	_etext
.globl	__cleanu
.globl	cntbase
.globl	start
	
	.text

start:
	int	0xf4
	fldcw	L001
	mov	bp,sp
	mov	si,bp
L000:
	inc	si
	inc	si
	cmp	(si),*0
	jnz	L000
	inc	si
	inc	si
	push	si
	mov	_environ,si
	mov	ax,bp
	add	ax,#2
	push	ax
	mov	ax,*0(bp)
	push	ax
	mov	ax,#cbufs
	push	ax
	mov	ax,#_etext
	sub	ax,#eprol
	add	ax,#7
	mov	cx,#3
	ror	ax,cl
	and	ax,#0x1fff
	add	ax,#0x0388
	push	ax
	rol	ax,*1
	push	ax
	call	_sbrk
	pop	cx
	cmp	ax,#-1
	jz	L004
	push	ax
	add	ax,#6
	mov	cntbase,ax
	mov	ax,#_etext
	push	ax
	mov	ax,#eprol
	push	ax
	call	_monitor
	add	sp,*10
	call	_main
	push	ax
	call	_exit
L004:
	mov	ax,*L003-L002
	push	ax
	mov	ax,#L002
	push	ax
	mov	ax,#2
	push	ax
	call	_write
_exit:
	call	__cleanu
	mov	ax,#0
	push	ax
	call	_monitor
	pop	ax
	pop	ax
	pop	ax
	mov	bx,#1
	int	0xf1
eprol:
	.data
	.byte	0,0		| v7 has these two bytes here for a NULL pointer...
L001:
	.word	0xfbf		| 8087 control word
L002:
	.byte	'N, 'o, ' , 's, 'p, 'a, 'c, 'e, ' , 'f, 'o, 'r, ' 
	.byte	'm, 'o, 'n, 'i, 't, 'o, 'r, ' , 'b, 'u, 'f, 'f, 'e, 'r, 10, 0
L003:

	.bss
	.comm	_environ, 2
	.comm	cntbase, 2
