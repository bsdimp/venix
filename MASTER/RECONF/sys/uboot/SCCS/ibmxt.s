| XT winchester driver via IBM-BIOS.           6/83, 7/84
|	AX = logical block number (0 to ?)
|	ES = memory segment
|	BX = memory address
|	on return Z flag 0->error, 1->no error

#ifdef	ATASI

NSECT=  17
NHEAD=	7

#else	ATASI

NSECT=  17
NHEAD=	4

#endif	ATASI

	.text
	.globl	rblk
rblk:
	mov     cx,*NSECT*NHEAD
        xor     dx,dx
	inc	ax			| bump block past partition table
        idiv    cx
	movb	ch,al			| low cylin no.
	xorb	al,al
	shr	ax,*1
	shr	ax,*1
	movb	cl,al			| high cylinder bits
	mov	ax,dx
	xor	dx,dx
	push	bx			| save bx
	mov	bx,*NSECT
	idiv	bx
	movb	dh,al			| head number
	orb	cl,dl
	incb	cl			| sector (1-NHEAD) in cl
	movb	dl,*0x80		| winchester drive 0
	pop	bx
        mov     ax,#0x0201              | read one sector
        int     0x13
        cmpb    ah,*0			| adjust Z flag
	ret
