# Venix Source Code Restoration Project

This is a project whose aim is to recreate the Venix86 operation
system source code using the Research Unix 7th Edition release as its
basis. To the largest extent possible, we will endevlor to create .c
sources that, when compiled with the compiler in Venix86, produce
identical binaries.

For some programs, this will be easy as it's clear that the program
was compiled unmodified from the v7 sources. For other programs, that
will be significantly harder because the originals were written in
pdp-11 assembler, or they are inherently machine dependent (like
compilers and assemblers).

In addition, efforts are under way to adopt one of the 8088/8086
simulators to run Venix binaries, though it's unclear the exact scope
of the project. Ideally, we'd be able to run the compilers and
toolchain that way to help in the reconstruction efforts as the native
machines that these run on are now quite slow (it takes about 30
minutes to build the kernel sources natively, and < 3s to build them
with alternative compilers).

Venix differs from other V7 unixes in a number of ways. PC/IX, for
example, does floating point emulation differently. There may be
differences in object headers and such as well.

