| Floppy disk driver via IBM-BIOS.           1/83, 7/84
|	AX = logical block number (0 to ?)
|	ES = memory segment
|	BX = memory address
|	on return Z flag 0->error, 1->no error

NSECT=  8
NTRAC=	40

	.text
	.globl	rblk
rblk:
	mov     cx,*NSECT
        xor     dx,dx
        idiv    cx
	movb	cl,dl
	incb	cl			| sector (1-8) in cl
	xor	dx,dx			| clear head select
	cmpb	al,*NTRAC		| check for double sided
	blt	L00
	incb	dh			| set head to 1 for double sided
	negb	al			| new track is 2*NTRAC - 1 - track
	addb	al,*2*NTRAC-1
L00:	movb	ch,al			| track no. (0-39)
        mov     ax,#0x0201              | read one sector
        int     0x13
        cmpb    ah,*0			| adjust Z flag
        ret
