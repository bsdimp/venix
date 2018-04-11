# A case study: sync(1)

## sync(1)

In any Unix distribution, /bin/sync is about the simplest program in
the system. It is little more than a call to sync(2) and maybe exit(3).
V7 unix is no exception:
```C
main()
{

	sync();
}
```
though the Venix cersion is a either a little more complicated, or the
compiler is generating special code for main.

## Disassembled sync

A fully disssembled sync is below

```assembler

| @(#)	Disassembly of /dune/imp/venix/extracted/bin/sync
| OMAGIC format
| Stack size for -z binary: 0x2000 paragraphs
	.text
start:
	int	$f4
	wait
	.byte	0xd9		| FLDCW f8087cw
	.byte	0x2e
	.byte	0x8c
	.byte	0x20
	mov	bp,sp		| Reconstruct args from the kernel
	mov	si,bp
1:	add	si,*2
	cmp	(si),*0
	jnz	1b
	add	si,*2
	push	si		| push _environ
	mov	_environ,si
	mov	ax,bp
	add	ax,#2
	push	ax		| push argv
	mov	ax,*0(bp)
	push	ax		| push argc
	call	_main
	add	sp,*6
	push	ax
	call	_exit
	pop	cx
	mov	bx,#1
	int	$f1
	.byte	0
_main:				| main() {
	push	bp
	mov	bp,sp
	push	si
	push	di
	cmp	sp,*127
	ja	1f
	int	$f2
1:	call	_sync		| sync();
	xor	ax,ax
	push	ax
	call	_exit		| exit(0);
	pop	cx
	pop	di
	pop	si
	pop	bp
	ret			| }

_sync:
	push	bp
	mov	bp,sp
	mov	bx,#36
	int	$f1
	jcxz	1
	mov	_errno,cx
1:	pop	bp
	ret

_exit:
	push	bp
	mov	bp,sp
	push	si
	push	di
	call	__cleanup
	push	*4(bp)
	call	__exit
	pop	cx
	pop	di
	pop	si
	pop	bp
	ret
	.byte	0
__cleanup:
	ret
	.byte	0
__exit:
	push	bp
	mov	bp,sp
	mov	bx,#1
	mov	ax,*4(bp)
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:	pop	bp
	ret

	.data
junk:
	.word	0
f8087cw:
	.word	$0fbf

	.bss
_environ: . = .+2
_errno: . = .+2
```

### start

The start routine above looks like:
```assembler
	.text
start:
	int	$f4
	wait
	.byte	0xd9		| FLDCW f8087cw
	.byte	0x2e
	.byte	0x8c
	.byte	0x20
	mov	bp,sp		| Reconstruct args from the kernel
	mov	si,bp
1:	add	si,*2
	cmp	(si),*0
	jnz	1b
	add	si,*2
	push	si		| push _environ
	mov	_environ,si
	mov	ax,bp
	add	ax,#2
	push	ax		| push argv
	mov	ax,*0(bp)
	push	ax		| push argc
	call	_main
	add	sp,*6
	push	ax
	call	_exit
	pop	cx
	mov	bx,#1
	int	$f1

	.data
_environ:	.word 0
	.bss
_errno:		.zerow
```

The assembler is pretty stright forward: setup the FPU (8087),
reconstruct the args for main (and save _environ) from the stack given
a known protocol between the kernel and userland, call main, call
exit.

Also, given the default instructions, we know here that ds = ss, and
it's reasonable to assume that es = ss as well. Since no assumptions
are made about cs == ds or cs != ds, this assembler can be used for
both NMAGIC and OMAGIC code.

addr | stack
---- | -----
stack top -> |
 | NUL terminated strings for argv and env
 | 0
 | argv[0]
 | ...
 | argv[n]
 | 0
 | argv
 | 0
 | env[0]
... | 
      | env[n]
      | 0
sp -> | argc

### main

Main looks pretty straight forward. It calls sync, then exit. It's
unclear since we don't have the C compiler running outside of the
system yet if the exit(0) call was generated, or was added to the v7
sources. We know from TUHS that sync(1) in System III has evolved to
look like:
```C
main()
{
	sync();
	exit(0);
}
```

and we know from historical records that Venix was described variously
as being based on v7 and on System III. v1 for the pdp11/pro was based
on v7, while v2 for the pdp11/pro was based on System III. However,
Venix 86 2.0 was described as being based on v7. So we have some
evidence that even Venix 86 was based, in part, on System III. It's
quite like, as was common practice of the day, that VenturCom had an
enhanced v7 kernel and with parts from System III for userland.

### exit

```assembler
_exit:
	push	bp
	mov	bp,sp
	push	si
	push	di
	call	__cleanup
	push	*4(bp)
	call	__exit
	pop	cx
	pop	di
	pop	si
	pop	bp
	ret

__cleanup:
	ret

__exit:
	push	bp
	mov	bp,sp
	mov	bx,#1
	mov	ax,*4(bp)
	int	$f1
	jcxz	1f
	mov	_errno,cx
1:	pop	bp
	ret
```

In v7 unix we have the following code for exit in pdp-11 assembler:
```assembler
/ C library -- exit

/ exit(code)
/ code is return in r0 to system

.globl  _exit
.globl  __cleanup
exit = 1

_exit:
        mov     r5,-(sp)
        mov     sp,r5
        jsr     pc,__cleanup
        mov     4(r5),r0
        sys     exit
```
which is a little different since it inlines things. It's quite likely
that the Venix:
```C
int exit(val)
{
	_cleanup();
	return (__exit(val));
}
```
though they may have implemented it in assembler, since the System III
code was for pdp11 and vax and both of those had this code in assembler
similar to what we find in V7.

Finally, we see that __exit looks a lot like exit.o disassembled from
libc.a on the Venix system, and matches the typical system call
pattern we've documented in ABI.md. We store the system call number in
bx, and the argument in ax and hook into the kernel via int $f1.

### main return code
```assembler
	call	_main
	add	sp,*6
	push	ax
	call	_exit
	pop	cx
	mov	bx,#1
	int	$f1
```
Here we see some of the calling conventions:
* caller pushes arguments (see above, I omitted that for main because it was complex) and then pops them when it's done.
* Return value is in ax
* startup code automatically calls exit (not _exit) for the return value of main
* It falls back to calling the exit system call if that fails.

Also curious is that we pop into cx, not ax, the argument we pushed
for exit(). I think this is a bug, we should have popped ax since
that's where the first arg to the system call is passed, and ax isn't
guaranteed to be preserved. So we _exit with the return code of the
failed exit() call, not the return code of main.

## Other observations
The above sync.s has been hand massaged by me. Looking at the raw assembler we see things like:
```assembler
0x53	mov	bx,#36
0x56	int	$f1
0x58	jcxz	.+6		| loc 0005e
0x5a	mov	$2090,cx
0x5e	pop	bp
...
	.bss			| loc = 0208e, size = 00004
	.zerow	2
```
Now, the loc here is 208e. In the original disassembler, it reported
out as only 8e. I was able to deduce that the stack came right after
the code segment but before the data and bss segments by seeing code
like the above. Th only way this works is that if the stack is between
the code segment and the data segment. Other more complicated binaries
show the same pattern.

I've reconstructed the bss for sync.s as:
```assembler
_environ: .=.+2
_errno= .=.+2
```
since that was the style of the pdp-11 sources. The disassembler
produces a .zerow directive that seems to do the same thing, but
I see that nowhere in the pdp-11 v7 sources I've looked at.

We also see a few places like this:
```assembler
	int	$f1
	.byte	0
_main:				| main() {
	push	bp
```
or
```assembler
	pop	bp
	ret
	.byte	0
__cleanup:
	ret
	.byte	0
__exit:
	push	bp
```
where there's weird 0 bytes in the instruction stream. Now, of course,
we're only seeing this because I hacked the disassembler to recognize
this pattern. Before I hacked it, we had all kinds of crazy constructs
because the 0 was being disassembled as an instruction, not as padding.

## Conclusions
Even though it's nearly the simplest program in the tree, we've been able
to glean a lot from careful study sync(1), even without a system
to run the compiler on to test hypothesises.
* We have found the system call interface
  * int $f1 for all calls
  * bx is system call number
  * ax has first arg
  * ax returns value from system call
  * cx returns errno from system call when carry set
* We have discovered the int $f4 convention for calling FP instructions
* We've reconstructed crt0.s, though a more direct route also does that.
* We've found hints that some System III sources were used, though we'll need more evidence once we get the compiler running
* We've discovered that the linker pads things to an even boundary sometimes, producing a 0 byte in the output to start on an even boundary.
* We've reconstructed sync(), exit() and _exit(), and confirmed them by disassembling the .o's in libc.a
* We've learned the layout of the in-memory program for OMAGIC progams
  * text segment first, loaded at 0
  * stack next
  * data
  * bss
  * cs = ds = es = ss (though tentative)
* We've learned that the entry field of struct exec is not an address to where to start execution of the program, so what is it?
* We can see some of the calling conventions for this platform
  * All pointers are 2 bytes
  * bp is used as a frame pointer, and is callee saved
  * ax is the return value from functions (and system calls)
  * caller pushes args in reverse order onto the stack
  * caller pops the args when done
* We may have found a bug in the startup code  
