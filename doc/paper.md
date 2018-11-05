# The Venix/86 Emulator

Venix is a v7-based port of Unix to 8088, and others, in the early
1980's. It was the first v7-based port to market for 8088. It also ran
on the pdp-11. A number of changes have been made to accomodate the
8088's quirks. It adds a number of extensions over the original v7. In
later versions, it became a blended v6, v7 and System III hybrid.

## Background

### The Setup

In 1984 I bought a DEC Rainbow 100B. For many years I ran MS-DOS on
it. I went away to college and learned Unix on a VAX 11/750 running
4.2BSD (and later 4.3BSD). I'd heard that there was a Unix available
for my Rainbow, but I never was able to locate a copy. By the time I
became interested, it was no longer available. And new copies were
like $500. I saw people boasting about how cool the Venix setups were,
but I never had the chance to run it. In time, I put the Rainbow
aside, but never got rid of it as I had a sentamental attachment to
it.

Fast forward to 2017. Every year or two I'd search E-Bay and the
internet for a 'Rainbow Venix' and every year I'd see articles from
back in the day about it, but nothing further. In 2017, however, I
found a blog by a gentleman who said he'd rescued a Rainbow from an
attic in Connecticut that had belonged to one of the main Venix
developers. There were a number of disks that came along with the
Rainbow that had a full set of Venix. I wrote to him and after some
back and forth, he shipped them to me to copy.

At first, I thought it would be super easy, but these disks were
old. So, I bought a new RX-50 drive, thinking that would help. It did,
but only so far. I bought new controllers for my Rainbow, even a
second Rainbow 100A. I was only able to copy about 80% of the
diskettes. So I bought a couple of good TEAC drives and a
Kryoflux. With that I was finally able to image the drives completely,
only to discover that the bits I couldn't read didn't matter: they
were after the end of the tar balls that were the distributiosn. But
I'm getting a bit ahead of myself.

### Installing Venix

So, after making two copies of all the disks and returning the
originals with a copy, I installed Venix on my Rainbow. I had in the
interim purchased David Gesswein's MFM hard drive emulator (from
www.pdp8.net). In installed Venix onto the hard drive. But it only
recognized the first 10MB of the drive, so things were tight.

I then did some more research, and found that the mystery 'BSW' disks
were actually the ones I wanted to use. This was a special edition for
the Rainbow by Larry Campbell who had, among other things, written
LC-Term (which I used extensively in the 80's during the main active
use of my Rainbow). There were a number of kernel extensions to these
disks, as well as several bonus BSD and other free-ware programs.

Once I got the BSW disks installed on the Rainbow, I could use all of
the 45MB drive I'd setup. Except that I could really only use 32MB of
the drive: the other 13MB had to be devoted to DOS and CP/M. Even so,
32MB of space was a lot of space.

I was able to use the floppy images to puzzle out the logical
interleave the Venix used on its drives. Once I had that, I was able
to extract the binary from the disk images on my much newer 64-bit
Intel machines. I had to grab a copy of tar from V7 because latter-day
copies of tar to help in puzzling out the interleave. At first I
thought there was some kind of weird copy protection, but that turned
out not to be the case. The only copy protection was magic binary bits
lodged in the boot sector that were passed into the kernel to tell it
how many users were allowed.

### A Crazy Idea

I downloaded the V7 Unix sources from the TUHS archive. I thought it
would be interesting to see how hard it would be to build the raw V7
sources. I just tried the /usr/src/cmd directory, and just the simple
programs that were there. To my surprise many compiled, but a few
didn't. And some that did compile were useless because they operated
on pdp-11 binaries.

So, I thought, I'd love to be able to reconstruct the kernel. I have a
disassembler for the assembler bits I have the C compiler and sources
that were close. Why not try to "restore" the sources as best I could
for at least most of the features. It would be cool to have V7 sources
built on my Rainbow to celebrate the 40th anniversary of the V7
release, which was in 2 years (if you take the 1979 release date to be
accurate).

But I hit a snag. My Rainbow was not stable. At least not stable
enough to transfer the large amounts of data I'd need to. I bought a
couple of other Rainbows and did some part exchanges and came up with
one that was stable, but even then, the performance of the serial port
meant that I couldn't get more than about 2400 baud out of the link,
which was far too slow. And the Rainbow itself was also a fairly slow
machine, running at 4.7MHz on a CPU that took lots of clocks for most
of the instructions.

So, the crazy idea: Run it in emulation. I tried out QEMU, but found
that there was no support for my old hardware. I tried out out in
Bhyve, but found the same thing. I tried MESS from the MAME project,
but it wasn't competely working (things have changed a little
there). But the MESS images were hard to export data from since I
didn't have a good network connection to the virtual machine, and the
machine itself was slow (in part to faithfully emulate the timing and
speed of the old hardware). I soon gave up on this idea.

I spent some time enhancing FreeBSD's vm86 mode to run these old a.out
binaries, but hit several snags in doing that (the FreeBSD kernel
kinda assumes it's running 32-bit clients). It would be possible, but
it would be a lot more work than I was up for.

I looked at all the run DOS programs on your Linux, FreeBSD or other
Unix programs. They too suffered form a number of issues that made
them hard to adopt. Also, 16-bit code doesn't run on my amd64 box, so
I have to run it in a 32-bit OS running in emulation on my 64-bit
Intel server. So I walked away from those ideas.

I finally found a 8088 emulator online. I started crafting that to
suit my needs. It turns out that someone had a partial MS-DOS emulator
using it. I was able use that as a base.

# Reverse Engineering VENIX ABI

So, I started to reverse engineer the VENIX ABI. The first thing I
learned was that it used a.out format, which all the old Unixes had
before COFF and later ELF displaced it. That was good. There's a
number of tools that understand a.out, so I was able to peer into the
binaries I had.

I had no manuals at this point (though I've since discovered that
Bitkeepers had the PRO version of Venix manuals scanned in online, and
those manual documented both PRO/PDP-11 details as well as the 8088
Rainbow details, which was perfect, but it came late to the game). I
did have a disassembler and time to look at the various ar archive on
the system.

I disassembled all the system call .o's. By long standing tradition on
Unix to keep program sizes down, most .o's in libc.a have only one
function in them, sometimes with a small number of auxiliary functions
needed to impelement them. I didn't think I had much hope of
understanding stdio, but the system call .o's in newer versions of
Unix were all rock simple. So too were the Venix ones. I was able to
guess and discover most of the important system calls via this
method. Some of the details were missing, but I had enough to create a
table of system calls, along with arguments used. Some I had no clue
about as I just had a function name (sdata, what's that?). But I had
enough to go on.

In addition to system call tables, I was able to start to put together
an ABI specification for Venix. I saw which registers were used in
what ways looking at this, and other disassembly. I was able to
document the setjmp/longjmp stuff as well, which lead me to know that
ax, bd, cx, and dx were caller save, while si and di were callee
save. The standard Intel frame pointers were used with bp. I learned
the size of the different operands. Months later, I'd discover that
the Venturacom folks had started with a MIT 8088 portable C compiler
port for their compiler and fixed a lot of bugs and write an
optimimzer for it (though the code wasn't all that optimized by
today's standards).

It was about this time that I also produced an annotated copy of sync,
one of the simplest programs in the system and the shortest real
binary I could find. This let me walk through the startup code and
figure out what all the bits of it did. I also worked out more of the
system call interface (int 0xf1) as well as other soft interrupt
usage. I discovered that there's only 2 system calls used in
sync: sync(2) and _exit(2).

Once I had this information in hand, I was able to start to write a
loader. I discovered there were two types of binaries. OMAGIC and
NMAGIC. OMAGIC were a tiny model that had I and D space in the same
64k segment. This is somewhat similar to DOS .COM files (except those
had no header, while OMAGIC did have a header and then nothing else
after the header). NMAGIC had I and D space separated out. With more
disassembly, I was able to determine that DS == ES == SS in this
model, but CS != DS. I've seen this referred to as a 'small' model.

From the disassembled startup code, I was able to workout the
conventions for passing in the arguments to the program and the
environment. Using these, I was able to setup the environment like the
programs expected. During this time, I also ported FreeBSD's ddb to
this environment, at least enough so I could disassemble files. That
helped me greatly because ddb has a much better disassembler than the
one I was using from Minix. It would alter turn out a very good thing
that I had done so.

Once I got all that, I was able to go about naively implementing
system calls. I got sync working easily enough. The system calls
passed in no arguments that mattered, and it was easy enough to have
something work, even if the code was fairly linear.

That's when I hit the first of many snags. While I could get programs
like 'sync' to work, programs like 'echo' didn't work. echo is not
much harder, except it used stdio to puts the args. I then put the
project aside, disheartened I couldn't get 'echo' to work.

found manuals, got docs, added a debugger, implemented more system
calls. etc

# Overview of the Venix/86 emulator

