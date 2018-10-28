|===============================================================+
|								|
|		`low' machine code for 8088/86/186		|
|		Version 86/2.0	     March 18, 1983		|
|			Edited:  6/13/84			|
|===============================================================+
|								|
|	(C)Copyright by VenturCom, Inc. 1982, 1983, 1984	|
|								|
|	All rights reserved: VENTURCOM INC. 1982,1983,1984	|
|								|
|	This source listing is supplied in accordance with	|
|	the Software Agreement you have with VenturCom.		|
|								|
|===============================================================+
LOWMEM = 0x60		| segment value for kernel (just above iva)
MGRAN  = 512		| memory granularity in bytes
USIZE  = 1024		| size of u. (must be in MGRAN clicks)
u_rsav = 0		| offsets from user.h
u_cs   = 4
u_ds   = 6
u_es   = 8
u_fpsaved = 16
u_fps	= 18

jmpcx = 0xE1FF		| some unimplemented assembler instructions
nop   = 0x90

|
| VENIX/86 start off (bootstrap starts execution at location 0 `start').
|
| Relocate complete kernel down to low memory.
	.text
start:	cli
	mov	dx,#LOWMEM	| base of relocated kernel
	mov	cx,cs
	cmp	cx,dx		| are we there (put there by bootstrap) ?
	beq	L0002		| Yes.
	mov	ds,cx
	mov	es,dx
	call	L0000		| get our relative address
L0000:	pop	si	
	add	si,#start-L0000
	xor	di,di
	mov	cx,#L0002+10	| relocate the first chunk (extra for
	repz			| pipelined processors)
	movsb
	.byte	0xEA		| go to relocated code "jmp intersegment"
	.word	L0001,LOWMEM
L0001:	mov	cx,di		| finish relocation (do a full 64kb)
	neg	cx
	repz
	movsb

L0002:	mov	dx,cs		| Get current code segment.
	mov	ax,*0
	cmp	ax,#Lfcw	| Is this a separate I/D kernel ?
	bne	L0010		| No. First data symbol is not 0.

| Move data into data segment (copy backwards to avoid bashing data).
	mov	ds,dx
	mov	ax,#_etext	| Calculate the data segment offset.
	mov	si,ax
	add	ax,#0x01FF	| Round up to 512 byte boundry.
	and	ax,#0xFE00
	movb	cl,*4
	shr	ax,cl
	mov	dsoff,ax	| Save the difference for trap
	mov	vid00,ax	| and BIOS video context switch.
	add	dx,ax		| Calculate the data segment.
	mov	es,dx
	mov	cx,#_edata
	add	si,cx
	mov	di,cx
	add	cx,*2		| Make sure to get all bytes.
	std
	repz
	movsb

| Set data, stack, and extra segments to kernel.
L0010:	mov	ss,dx
	mov	ds,dx
	mov	es,dx

| Set up stack pointer and zero BSS.
	mov	sp,#_u+USIZE
	mov	di,#_edata
	mov	cx,#_end
	sub	cx,di
	sub	ax,ax
	cld
	repz
	stosb

| Check if 8087 is present
	.byte	0xD9, 0xE8	| "fld1" (without wait)
	mov	cx,*30
	loop	.		| effective wait
	.byte	0xDF,0x1E	| "fstpi fpp" (without wait)
	.word	fpp
	mov	cx,*30
	loop	.		| effective wait
	cmp	fpp,*1		| Did it work correctly?
	bne	L0005
	mov	L0F4,#rti	| Yes.  8087 is present
	finit			| Initialize.
	fldcw	Lfcw		| No exception handling.
	incb	_u+u_fpsaved	| Indicate it is to be saved.
L0005:

| Relocate BIOS interrupt service routines for kernel interception.
	mov	es,ax
	seg	es
	mov	ax,0x10*4	| FD:	video call from 0x10
	seg	es
	mov	0xFD*4,ax
	seg	es
	mov	ax,0x10*4+2
	seg	es
	mov	0xFD*4+2,ax
	seg	es
	mov	ax,0x9*4	| FE:	keyboard interrupt from 0x9
	seg	es
	mov	0xFE*4,ax
	seg	es
	mov	ax,0x9*4+2
	seg	es
	mov	0xFE*4+2,ax
	seg	es
	mov	ax,0x8*4	| FF:	timer interrupt from 0x8
	seg	es
	mov	0xFF*4,ax
	seg	es
	mov	ax,0x8*4+2
	seg	es
	mov	0xFF*4+2,ax

| Bash the interrupt vectors.
	mov	ax,cs
	mov	bp,#sstack	| prototype addresses
	mov	bx,es		| start at physical zero
	mov	cx,#0x100	| number of vectors to be bashed
L0006:	mov	dx,(bp)
	cmp	dx,*0
	beq	L0007		| don't bash if prototype is 0
	seg	es
	mov	(bx),dx
	seg	es
	mov	*2(bx),ax
L0007:	add	bx,*4
	add	bp,*2
	loop	L0006

| Execute kernel "once only" code.
	int	0x12		| get memory size from BIOS
	push	ax
	push	ds		| data segment
	call	_main

| If possible, get the TOD from hardware and stash it in `_time'
| in long format as seconds since Jan 1, 1970.
				| No such hardware on PC.

| Return to the user code copied out in main.c.
	cli
	cmpb	_u+u_fpsaved,*2	| Need to restore 8087 ?
	bne	L0008
	frstor	_u+u_fps	| Yes.
	fwait
L0008:	movb	_u+u_fpsaved,*0
	mov	ss,_u+u_ds	| set up return to user
	mov	sp,#0xfffe
	mov	ax,#0x0200
	push	ax		| new psw; interrupts enabled
	push	_u+u_cs		| code segment
	xor	ax,ax		| start PC at 0
	push	ax
	incb	Luser		| indicate at user level
	mov	ds,_u+u_ds	| data segment
	iret

|
| A user has changed the TOD via a `time' system call.
| Examine `_time' and put into the hardware.
|
	.globl	_setdate
_setdate:
	ret			| Nothing to do on PC.

|
| Various trap transfers.
|

video:	push	di		| Some users and BIOS call BIOS directly.
	push	si		| Intercept and make sure in the kernel.
	mov	si,ss		| Save current stack segment.
	mov	di,cs		| Calculate kernel's data/stack segment.
	.byte	0x81, 0xC7	| "add	di,#..." instruction
vid00:	.word	0
	mov	ss,di		| Use the kernel stack segment.
	seg	ss
	cmpb	Luser,*0	| Check if from user.
	beq	Lvid00
	mov	di,sp		| Yes. Change to kernel stack.
	mov	sp,#_u+USIZE-2
	seg	ss
	movb	Luser,*0	| Indicate in kernel mode.
	int	0xFD		| Pass off to BIOS.
	seg	ss
	incb	Luser		| Go back to user.
	mov	ss,si
	mov	sp,di
	pop	si
	pop	di
	iret
Lvid00:	mov	ss,si		| Already in kernel.
	pop	si
	pop	di
	int	0xFD		| Pass off to BIOS.
				| Fall through and return ...

rti:	iret			| No special actions.

fpsim:	push	ax		| FP simulator
	mov	ax,#_fpsim
	jmp	trap

uintr:	push	ax		| unknown interrupt
	mov	ax,#_uintr
	jmp	trap

cslintr:push	ax		| console interrupt
	mov	ax,#Lcintr
	jmp	trap
Lcintr:	int	0xFE		| pass to BIOS first
	jmp	_cslintr

clock:	push	ax		| timer interrupt
	mov	ax,#Lclock
	jmp	trap
Lclock:	int	0xFF		| pass to BIOS first
	xor	ax,ax		| Map the actual 18.2 Hz clock into an
	mov	dx,ax		| effective 60 Hz clock by adding 3 or
	movb	ax,_time	| 4 ticks per physical tick.  This value
	mov	cx,*5		| is set into `_clkdiff'.  The algorithm
	div	cx		| used here is off by less than 1 minute
	or	dx,dx		| in 24 hours.  The equivalent C code is:
	jne	Lclk00		| if(lbolt < (((unsigned char)time)%5==0?48:36))
	mov	ax,*48		|	diff = 3;
	j	Lclk01		| else
Lclk00:	mov	ax,*36		|	diff = 4;
Lclk01:	cmp	_lbolt,ax
	jge	Lclk02
	mov	_clkdiff,*3
	j	Lclk03
Lclk02:	mov	_clkdiff,*4
Lclk03:	jmp	_clock

ttrap:	push	ax		| trace or bpt trap
	mov	ax,#_ttrap
	jmp	trap

nmi:	push	ax		| non-maskable interrupt
	mov	ax,#_nmi
	jmp	trap

emt:	push	ax		| emt
	mov	ax,#_emt
	jmp	trap

abort:	push	ax		| abort
	mov	ax,#_abort
	jmp	trap

sys:	push	ax		| sys
	mov	ax,#_sys
	jmp	trap

| spl-- set and return priority levels
|	this machine only has 2, so there isn't much to do;
|	but the pdp11 names are retained.

	.globl	_spl0,_spl1,_spl2,_spl3,_spl4,_spl5,_spl6,_spl7,_splx
_spl0:
	pushf
	sti
	pop	ax	| return old flags
	ret
_spl1:
_spl2:
_spl3:
_spl4:
_spl5:
_spl6:
_spl7:
	pushf
	cli
	pop	ax
	ret
_splx:
	pop	cx
	popf
	pushf
	.word	jmpcx

|
| idle()
|
.globl _idle
_idle: 	pushf
	sti
	.byte	nop,nop,nop
	popf
	ret

|
| disp#( ... )
|	Used by the kernel driver to call the BIOS display routine.
|
	.globl	_disp2, _disp5, _disp6, _disp7, _disp9, _disp14

_disp2:				| set cursor position
	push	bp
	mov	bp,sp
	movb	ah,*2
	movb	bh,*4(bp)
	movb	dh,*6(bp)
	movb	dl,*8(bp)
disp00:	int	0xFD		| call BIOS display routine
	pop	bp
	ret
_disp5:				| select active display page
	push	bp
	mov	bp,sp
	movb	ah,*5
	movb	al,*4(bp)
	br	disp00
_disp6:				| scroll active page up
	push	bp
	mov	bp,sp
	movb	ah,*6
	br	disp01
_disp7:				| scroll active page down
	push	bp
	mov	bp,sp
	movb	ah,*7
disp01:	movb	al,*4(bp)
	movb	bh,*6(bp)
	mov	cx,*8(bp)
	mov	dx,*10(bp)
	br	disp00
_disp9:				| write a character
	push	bp
	mov	bp,sp
	movb	ah,*9
	movb	al,*4(bp)
	movb	bh,*6(bp)
	movb	bl,*8(bp)
	mov	cx,*10(bp)
	br	disp00
_disp14:			| write a char in tty mode
	push	bp
	mov	bp,sp
	movb	ah,*14
	movb	al,*4(bp)
	movb	bh,*6(bp)
	subb	bl,bl
	br	disp00

|
| getchar()
|	Called by the driver to get the next character
|	from the console terminal.
|
	.globl _getchar
_getchar:
	movb	ah,*1		| see if characters available
	int	0x16
	jnz	L0040		| yes
	mov	ax,#-1		| no, return (int)(-1)
	ret
L0040:	subb	ah,ah		| fetch char
	int	0x16
	ret

|
| flpio(ah, al, ch, cl, dh, dl, bx, es)
|	Called by the floppy driver.
|
	.globl	_flpio
_flpio:
	push	bp
	mov	bp,sp
	movb	ah,*4(bp)
	movb	al,*6(bp)
	movb	ch,*8(bp)
	movb	cl,*10(bp)
	movb	dh,*12(bp)
	movb	dl,*14(bp)
	mov	bx,*16(bp)
	mov	es,*18(bp)
	int	0x13		| call BIOS floppy driver
	mov	al,ah
	pop	bp
	ret	

|
| Routines to do byte i/o.
|
	.globl	_io_inb, _io_outb
_io_inb:
	mov	bx,sp
	mov	dx,*2(bx)
	in
	xorb	ah,ah
	ret

_io_outb:
	mov	bx,sp
	mov	dx,*2(bx)
	mov	ax,*4(bx)
	out
	ret

|
| getds()
|	Get the current DS value.
|
	.globl	_getds
_getds:
	mov	ax,ds
	ret

flp_base:			| Faster & 9 sector diskette parameters.
	.byte	0xDF, 2, 0x25, 2, 9, 0x2A, 0xFF, 0x50, 0xF6, 0, 4

	.data
Lfcw:	.word	0xFBF		| 8087 control word (must be first in data)
	.comm	Luser,2

	.globl	sstack
sstack:				| will later be used as a system stack area
	.word	rti		|  0:	divide by zero
	.word	ttrap		|  1:	trace trap
	.word	nmi		|  2:	NMI (memory parity error)
	.word	ttrap		|  3:	BPT instruction
	.word	rti		|  4:	overflow
	.word	0		|  5:	BIOS print screen	****
	.word	0		|  6:	BIOS reserved		****
	.word	0		|  7:	BIOS reserved		****
	.word	clock		|  8:	BIOS clock
	.word	cslintr		|  9:	keyboard
	.word	uintr		|  A:		unknown
	.word	uintr		|  B:		unknown
	.word	uintr		|  C:		unknown (async comm)
	.word	uintr		|  D:		unknown (winchester)
	.word	0		|  E:		floppy		****
	.word	uintr		|  F:		unknown (printer)

	.word	video		| 10:	BIOS video
	.word	0		| 11:	BIOS equipment		****
	.word	0		| 12:	BIOS memory size	****
	.word	0		| 13:	BIOS diskette		****
	.word	uintr		| 14:	BIOS RS232
	.word	0		| 15:	BIOS cassette		****
	.word	0		| 16:	BIOS keyboard		****
	.word	0		| 17:	BIOS printer		****
	.word	uintr		| 18:	BIOS basic entry
	.word	0		| 19:	BIOS bootstrap		****
	.word	0		| 1A:	BIOS time of day	****
	.word	rti		| 1B:	keyboard break
	.word	rti		| 1C:	time interrupt
	.word	0		| 1D:	BIOS video table	****
	.word	flp_base	| 1E:	BIOS diskette table
	.word	0		| 1F:	BIOS graphic char table	****

	.word	uintr		| 20:
	.word	uintr		| 21:
	.word	uintr		| 22:
	.word	uintr		| 23:
	.word	uintr		| 24:
	.word	uintr		| 25:
	.word	uintr		| 26:
	.word	uintr		| 27:
	.word	uintr		| 28:
	.word	uintr		| 29:
	.word	uintr		| 2A:
	.word	uintr		| 2B:
	.word	uintr		| 2C:
	.word	uintr		| 2D:
	.word	uintr		| 2E:
	.word	uintr		| 2F:

	.word	uintr		| 30:
	.word	uintr		| 31:
	.word	uintr		| 32:
	.word	uintr		| 33:
	.word	uintr		| 34:
	.word	uintr		| 35:
	.word	uintr		| 36:
	.word	uintr		| 37:
	.word	uintr		| 38:
	.word	uintr		| 39:
	.word	uintr		| 3A:
	.word	uintr		| 3B:
	.word	uintr		| 3C:
	.word	uintr		| 3D:
	.word	uintr		| 3E:
	.word	uintr		| 3F:

	.word	0		| 40:	BIOS new diskette interrupt	*****
	.word	0		| 41:	BIOS fixed disk param		*****
	.word	uintr		| 42:
	.word	uintr		| 43:
	.word	uintr		| 44:
	.word	uintr		| 45:
	.word	uintr		| 46:
	.word	uintr		| 47:
	.word	uintr		| 48:
	.word	uintr		| 49:
	.word	uintr		| 4A:
	.word	uintr		| 4B:
	.word	uintr		| 4C:
	.word	uintr		| 4D:
	.word	uintr		| 4E:
	.word	uintr		| 4F:

	.word	uintr		| 50:
	.word	uintr		| 51:
	.word	uintr		| 52:
	.word	uintr		| 53:
	.word	uintr		| 54:
	.word	uintr		| 55:
	.word	uintr		| 56:
	.word	uintr		| 57:
	.word	uintr		| 58:
	.word	uintr		| 59:
	.word	uintr		| 5A:
	.word	uintr		| 5B:
	.word	uintr		| 5C:
	.word	uintr		| 5D:
	.word	uintr		| 5E:
	.word	uintr		| 5F:

	.word	uintr		| 60:
	.word	uintr		| 61:
	.word	uintr		| 62:
	.word	uintr		| 63:
	.word	uintr		| 64:
	.word	uintr		| 65:
	.word	uintr		| 66:
	.word	uintr		| 67:
	.word	uintr		| 68:
	.word	uintr		| 69:
	.word	uintr		| 6A:
	.word	uintr		| 6B:
	.word	uintr		| 6C:
	.word	uintr		| 6D:
	.word	uintr		| 6E:
	.word	uintr		| 6F:

	.word	uintr		| 70:
	.word	uintr		| 71:
	.word	uintr		| 72:
	.word	uintr		| 73:
	.word	uintr		| 74:
	.word	uintr		| 75:
	.word	uintr		| 76:
	.word	uintr		| 77:
	.word	uintr		| 78:
	.word	uintr		| 79:
	.word	uintr		| 7A:
	.word	uintr		| 7B:
	.word	uintr		| 7C:
	.word	uintr		| 7D:
	.word	uintr		| 7E:
	.word	uintr		| 7F:

	.word	uintr		| 80:
	.word	uintr		| 81:
	.word	uintr		| 82:
	.word	uintr		| 83:
	.word	uintr		| 84:
	.word	uintr		| 85:
	.word	uintr		| 86:
	.word	uintr		| 87:
	.word	uintr		| 88:
	.word	uintr		| 89:
	.word	uintr		| 8A:
	.word	uintr		| 8B:
	.word	uintr		| 8C:
	.word	uintr		| 8D:
	.word	uintr		| 8E:
	.word	uintr		| 8F:

	.word	uintr		| 90:
	.word	uintr		| 91:
	.word	uintr		| 92:
	.word	uintr		| 93:
	.word	uintr		| 94:
	.word	uintr		| 95:
	.word	uintr		| 96:
	.word	uintr		| 97:
	.word	uintr		| 98:
	.word	uintr		| 99:
	.word	uintr		| 9A:
	.word	uintr		| 9B:
	.word	uintr		| 9C:
	.word	uintr		| 9D:
	.word	uintr		| 9E:
	.word	uintr		| 9F:

	.word	uintr		| A0:
	.word	uintr		| A1:
	.word	uintr		| A2:
	.word	uintr		| A3:
	.word	uintr		| A4:
	.word	uintr		| A5:
	.word	uintr		| A6:
	.word	uintr		| A7:
	.word	uintr		| A8:
	.word	uintr		| A9:
	.word	uintr		| AA:
	.word	uintr		| AB:
	.word	uintr		| AC:
	.word	uintr		| AD:
	.word	uintr		| AE:
	.word	uintr		| AF:

	.word	uintr		| B0:
	.word	uintr		| B1:
	.word	uintr		| B2:
	.word	uintr		| B3:
	.word	uintr		| B4:
	.word	uintr		| B5:
	.word	uintr		| B6:
	.word	uintr		| B7:
	.word	uintr		| B8:
	.word	uintr		| B9:
	.word	uintr		| BA:
	.word	uintr		| BB:
	.word	uintr		| BC:
	.word	uintr		| BD:
	.word	uintr		| BE:
	.word	uintr		| BF:

	.word	uintr		| C0:
	.word	uintr		| C1:
	.word	uintr		| C2:
	.word	uintr		| C3:
	.word	uintr		| C4:
	.word	uintr		| C5:
	.word	uintr		| C6:
	.word	uintr		| C7:
	.word	uintr		| C8:
	.word	uintr		| C9:
	.word	uintr		| CA:
	.word	uintr		| CB:
	.word	uintr		| CC:
	.word	uintr		| CD:
	.word	uintr		| CE:
	.word	uintr		| CF:

	.word	uintr		| D0:
	.word	uintr		| D1:
	.word	uintr		| D2:
	.word	uintr		| D3:
	.word	uintr		| D4:
	.word	uintr		| D5:
	.word	uintr		| D6:
	.word	uintr		| D7:
	.word	uintr		| D8:
	.word	uintr		| D9:
	.word	uintr		| DA:
	.word	uintr		| DB:
	.word	uintr		| DC:
	.word	uintr		| DD:
	.word	uintr		| DE:
	.word	uintr		| DF:

	.word	uintr		| E0:
	.word	uintr		| E1:
	.word	uintr		| E2:
	.word	uintr		| E3:
	.word	uintr		| E4:
	.word	uintr		| E5:
	.word	uintr		| E6:
	.word	uintr		| E7:
	.word	uintr		| E8:
	.word	uintr		| E9:
	.word	uintr		| EA:
	.word	uintr		| EB:
	.word	uintr		| EC:
	.word	uintr		| ED:
	.word	uintr		| EE:
	.word	uintr		| EF:

	.word	uintr		| F0:
	.word	sys		| F1:	system calls
	.word	emt		| F2:	emt (pdp11 carry over)
	.word	abort		| F3:	abort
L0F4:	.word	fpsim		| F4:	(8087 lead in)
	.word	ip_call		| F5:	interpage call
	.word	ip_ret		| F6:	interpage return
	.word	uintr		| F7:
	.word	uintr		| F8:
	.word	uintr		| F9:
	.word	uintr		| FA:
	.word	uintr		| FB:
	.word	uintr		| FC:
	.word	0		| FD:	(relocated BIOS video call)
	.word	0		| FE:	(relocated BIOS keyboard interrupt)
	.word	0		| FF:	(relocated BIOS timer interrupt)
