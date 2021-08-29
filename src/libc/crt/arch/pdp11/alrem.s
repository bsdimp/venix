/ Long remainder

.globl	alrem
.globl	csv, cret
alrem:
	jsr	r5,csv
	mov	8.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,6.(r5)
	bne	hardlrem
	mov	4.(r5),r0
	mov	2(r0),r2
	mov	(r0),r1
	mov	r1,r4
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	div	r3,r0
	mov	r1,r0
	mov	r2,r1
	div	r3,r0
	bvc	1f
	sub	r3,r0
	div	r3,r0
	tst	r1
	beq	9f
	add	r3,r1
1:
	tst	r4
	bpl	9f
	neg	r1
9:
	sxt	r0
	mov	4.(r5),r3
	mov	r0,(r3)+
	mov	r1,(r3)
	jmp	cret

/ The divisor is known to be >= 2^15.  Only 16 cycles are
/ needed to get a remainder.
hardlrem:
	mov	4.(r5),r0
	mov	2(r0),r2
	mov	(r0),r1
	bpl	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	mov	6.(r5),r3
	bge	1f
	neg	r3
	neg	8.(r5)
	sbc	r3
1:
	mov	$16.,r4
1:
	clc
	rol	r2
	rol	r1
	rol	r0
	cmp	r3,r0
	blt	2f
	bgt	3f
	cmp	8.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	8.(r5),r1
	sbc	r0
	sub	r3,r0
	sob	r4,1b
1:
	mov	4.(r5),r3
	tst	(r3)
	bge	1f
	neg	r0
	neg	r1
	sbc	r0
1:
	mov	r0,(r3)+
	mov	r1,(r3)
	jmp	cret
