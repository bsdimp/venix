|
|     88/86/186 Micro-boot Program.   3/83, 7/83, 8/84
|
|-------------------------------------------------------+
|       All rights reserved: VenturCom Inc. 1983,1984   |
|                                                       |
|       (c) Copyright VenturCom, Inc. 1983, 1984        |
|-------------------------------------------------------+
|
| Disk boot program to load and transfer to a VENIX file
| system entry.   Must be assembled with appropriate disk
| driver (ibmflp.s, ibmxt.s, etc.) and terminal driver
| (ibmterm.s, etc.).
|
| NOTE: The 32 byte "a.out" header must be removed before
| this code is transferred to the block 0 bootstrap.  Also
| a magic number may need to be patched in at some location
| (for example, a 0xAA55 at end of block for the ibm winnie).
|
| Authors: Branscomb, Hashimoto, Zimmerman
|
| Operation:
|	It prints an `&' on the CRT and accepts a venix file
|	name in the root directory.  If a carriage return is
|	typed to the `&', a default file name defined in a
|	string in this code will be used instead.  It will be
|	echoed to the CRT.  If nothing is typed within default
|	one second (controlled by TIME) then all will proceed
|	as if only a carriage return was entered.  If any char
|	is typed within 1 second, the machine will wait
|	forever for carriage return to terminate input.  If
|	backspace is entered, one char is removed from input
|	buffer, and screen is updated accordingly.  If the
|	specified file name is not found, an error message
|	and `&' prompt will be displayed.
|
| Memory layout:
|	000:
|		bootstrap
|	200:
|		some variables
|		stack
|	400:
|		512 byte buffer
|	600:
|		Beginning of loaded file
|

TIME	= 0x8000		| Timer for autoboot operation, ~1 sec on 8088

mema	= 0x200			| Several variables.
seg	= mema+2
inod	= seg+2
mode	= inod+0
addr	= inod+8
bno	= inod+32
name0	= bno+2
buf	= 0x400			| Base of stack and a buffer.
file	= 0x600			| Beginning of loaded file.
exec	= 0x620			| Execution address in loaded file.

	.text
begin:
	call	L000		| Calculate PC
L000:	pop	ax
	add	ax,#begin-L000
	shr	ax,*1		| Make into segment value
	shr	ax,*1
	shr	ax,*1
	shr	ax,*1
	push	cs
	pop	dx
	add	ax,dx
	push	ax		| New CS
	mov	ax,#L001
	push	ax		| and PC
	reti
L001:	push	cs		| Set up segment registers
	pop	ds
	push	cs
	pop	es
	push	cs
	pop	ss
	mov	sp,#buf-2
	cld
	br	start

putsub:				| Output a string (SI=address, CX=count)
	lodsb
	call	putchar
	loop	putsub
	ret

getnam:				| Get chars into name0 buffer.
	mov	(di),*0
	call	getchar		| Get a character in ax
	cmpb	al,*13
	bne	L308
	cmp	di,bp
	beq	search
	call	putchar
	movb	al,*10
	call	putchar
	br	search		| return
L308:	cmpb	al,*8
	beq	L302		| back space (delete a character)
	cmpb	al,*' 
	jb	L303		| illegal character
	cmp	di,#name0+14
	jae	L303		| line full
	stosb
L301:	call	putchar
	br	getnam
L302:	cmp	di,bp		| backspace operation
	beq	L303		| beginning of line
	dec	di
	push	ax
	call	putchar		| print <BS>
	movb	al,*' 
	call	putchar
	pop	ax
	br	L301
L303:	movb	al,*7
	br	L301

retry:				| if retry, print no file message
	mov	cx,*errend-error
	mov	si,#error
	call	putsub

start:
	movb    al,*'&		| print the & prompt
        call    putchar
	mov	di,#name0
	mov	bp,di
	cmp	si,#errend	| check for retry
	beq	getnam		| no timer if retry
	mov	cx,#TIME	| set up timer
L4:	call	getchk		| check for input character
	bne	getnam		| if char typed, stop timing and get char
	loop	L4		| keep on timing

search:
	cmp	di,bp		| no chars, then insert default path name
	bne	L7
	mov	bp,#default
	mov	cx,#error-default	| display default name
	mov	si,bp
	call	putsub
	mov	cx,#default1-default	| copy default name
	mov	si,bp
	repz
	movsb
L7:	mov	cx,*14
	xor	ax,ax
	repz
	stosb			| Zero rest of name0 buffer
	inc	ax		| Target inode 1 for the root
	call	iget

|
| Read directory looking for filename.
|
	mov	mema,#buf	| while directory searching, use buf
L9:	call    rmblk		| reads in block specified by indexed inode
	bne	retry		| if no file, restart
        mov     bx,#buf		| index into directory block
L10:	mov     di,bx		| temporary directory entry pointer
	mov	ax,(di)		| inode number
        add     bx,*16		| index directory increment by 16
	mov	si,#name0
        cmp     ax,*0		| check for null entry
	beq	L14		| try next entry
        inc     di
        inc     di		| index past inode pointer
        mov     cx,*14		| compare 14 bytes (name = entry ?)
	repz
	cmpsb
	or	cx,cx
        beq	L15		| both names matched
L14:	cmp     bx,#buf+511	| check for end of directory
        blo     L10		| if not, try next entry
        br      L9		| else, read another block
L15:
	call	iget
	test	mode,#060000	| regular file ?
	bne	retry		| no.
|
| Load in the file and execute.
|
	mov	mema,#file	| set disk handler memory address file
	mov	seg,ds
L16:
	mov	es,seg
	call    rmblk		| read in disk block
	bne	L17
	add	seg,#32		| Increment memory pointer by 512 bytes
	br	L16		| next block
L17:
	jmp	exec		| jmp to starting address

|
| Routine to read the inode specified by inode number in AX.
|
iget:
	mov     bno,*0		| clear logical block no.
	add     ax,*31
        mov     si,ax
        mov     cl,*4
        shr     ax,cl		| divide by 16
	mov	bx,#buf
        call    rblk		| read block
        and     si,*0xF
        mov     cl,*5
        shl     si,cl		| multiply by 32
        add     si,#buf		| offset from buf
        mov     di,#inod	| destination index to inod
        mov     cx,*12		| copy 24 bytes from disk buf to inod
	repz
	movsw
	ret

|
| Routine to read in block number specified by bno
| after applying file system mapping algorithm in inode.
| bno is incremented, success return is with the Z bit
| set, eof or error the Z bit is cleared.
|
rmblk:
	mov     bx,bno		| Get the relative block number.
	shl	bx,*1
        inc     bno		| add 1 for next time
	test	mode,#010000	| check if large file
        bne     L18		| branch to large file algorithm
        mov     ax,#addr(bx)	| hard block number from indexed inode
        or	ax,ax		| if none there, set eof return
        beq	Leof		| else read it in
	mov	bx,mema
	br	rblk
				| Large algorithm.
L18:
	or	bx,bx		| Branch around if indirect block already in
	bne	L20
	mov	bx,#buf		| Memory pointer to buf for indirect block
	mov	ax,addr		| *** Will fail for files > 128kb ***
	call    rblk		| and read it in.
	bne	L21
	xor	bx,bx
L20:
        mov     ax,#buf(bx)	| get block address from indirect block
	mov	bx,mema
        or	ax,ax		| if no block, eof return
        bne	rblk
Leof:	cmpb	al,*1		| clear the Z bit for eof
L21:	ret

	.data
default:
	.byte	'v,'e,'n,'i,'x
default1:
	.byte	13,10
error:
	.byte	'N,'o,' ,'f,'i,'l,'e,13,10
errend:
