	.globl	_setjmp
	.globl	_longjmp

	.text

	|
	| jmp_buf is 6 slots.
	|
	| Compiler does weird things so si/di get saved sometimes
	| and longjmp seems to know about this.
	|
	| [0] BP of calling frame
	| [1] SP after BP pushed in _setjmp
	| [2] IP from far call...
	| [3] contents of location 2???? a pointer?
	|     This is weird because nothing is in location 2 for
	|     all binary types. For OMAGIC with -z, the stack is
	|     located here, so this will be the last bit of the
	|     environment. For OMAGIC w/o -z, this will be the
	|     instruction WAIT followed by FLDCW (well the first
	|     byte of this). And for NMAGIC it will be the floating
	|     point control word.
	| [4] *[3]
	| [5] *([3] + 1)
_setjmp:
	push	bp
	mov	bp,sp
	mov	dx,*4(bp)	| pointer to jmpbuf
	mov	cx,*2(bp)	| IP
	mov	bx,*0(bp)	| BP
	mov	bp,dx
	mov	*0(bp),bx	| BP
	mov	*2(bp),sp	| SP
	mov	*4(bp),cx	| IP
	mov	bx,0x0002
	mov	*6(bp),bx	| see above... only used for NMAGIC
	mov	cx,(bx)		| 
	mov	*8(bp),cx	| ???
	mov	cx,*2(bx)
	mov	*10(bp),cx	| ???
	mov	ax,#0
	pop	bp
	ret

_longjmp:
	push	bp
	mov	bp,sp
	mov	bx,*4(bp)	| pointer to jmpbuf
	mov	ax,*6(bp)	| retval for setjmp
	or	ax,ax		| Make sure it isn't 0
	jnz	L000
	inc	ax
L000:
	mov	cx,(bx)		| load saved BP...
	j	L002
L001:
	mov	bp,*0(bp)	| go to previous trap frame
	or	bp,bp
	jz	L003		| done if we get to the end.
L002:
	cmp	*0(bp),cx	| Get saved BP
	jnz	L001		| keep looping until we find the right one...
	mov	si,*-2(bp)	| Restore si/di from the trap frame, if need be.
	mov	di,*-4(bp)
L003:
	mov	cx,cs
	mov	dx,ds
	cmp	cx,dx
	jz	L004		| Simple case when DS == CS -> OMAGIC skip
	mov	bp,*6(bx)	| read in the saved 8087 control word
	or	bp,bp
	jz	L004		| Punt if there isn't one...
	mov	0x0002,bp	| Restore its value (for FP emulator?)
	mov	cx,*8(bx)
	mov	*0(bp),cx
	mov	cx,*10(bx)
	mov	*2(bp),cx	| Restore 2 more things I'm unclear on :(
L004:
	mov	bp,(bx)		| Restore saved frame pointer
	mov	sp,*2(bx)	| And stack
	pop	cx		| pop off saved BP and ignore (sp += 2)
	pop	cx		| pop off return location and ignore (sp += 2)
	mov	cx,*4(bx)	| restore saved return address
	push	cx		| to the top of the stack and
	ret			| return there...


