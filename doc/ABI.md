Venix/86 ABI

This document tries to describe the Venix86 ABI. It is reverse
engineered from a copy of Venix86R 2.0 and Venix86 2.1.

# Venix Origins

Venix is a v7-based port to the 8088, including the Dec Rainbow and
IBM-PC, XT and AT. A number of changes have been made to the original
v7 sources to accomodate the 8088's quirks.

## Interupt Usage

### $F1 -- System Calls

System Call number is in the `bx` register. The system calls appear to
be the same as in pdp-11 v7 unix, but that's not been completely
confirmed.

### $F2 -- EMT

This is 'emt' in the low.s. The comment says its a hold over from
pdp11. Looks like it generates SIGEMT. In the generated code, we often
see sequences like:
```assembler
    cmp     sp,*127
    ja      1f
    int     $f2
1f:
```
where various sanity checks are done on sp. Some programs just have
this in main, while others have it more extensively. See the EMT
section in the kernel reference.

### $F3 -- Abort

Seems unused, but low.s lists it as 'abort' which trap.o translates to
tsignal like emt.

### $F4 -- Floating Point

Floating point emulation. All 8087 opcodes are preceeded by this sequence:
    int     $f4
    wait
as in the following:
    int     $f4
    wait
    fld     (si)

All programs start with:
    int     $f4
    wait
    .byte   0xd9            | esc   $36f4
    .byte   0x2e
    .byte   0xf4
    .byte   0x36
Note that x87 0xd9 0x2e is FLDCW to load the next two bytes into code
word, but the mask of valid bits for the CW is $0f3f, so the above
number is nonsense. It's also different for every progam. It's not yet
clear what the extra bits mean, or what it's encoding. The 'base'
sequence in crt0.o is
    int     $f4
    wait
   .byte   0xd9            | esc   '+56
   .byte   0x2e
   .byte   0x38
   .byte   0x00
Here $0038 does make some sense. It decodes as
* Round to nearest even
* 24-bits of precision
* ignore the Precision, Underflow and Overflow exceptions.
which seems like a reasonable default. It may be some kind of signal
or hint to the floating point emulation library inside the Venix
kernel to do things or not do things.

### $Fx -- Others reserved for redirect

For the xt version $fd is reseved for video redirect, $fe is reserved
for keyboard redirect, and $ff is reserved for timer interrupt.

The Rainbow version has not been examined in detail yet.

low.s stores these values in a table at the end of low.s, and then it
gets overwritten with the stack for the kernel after the kenrel is
relocated, but more about that in the kernel document.

