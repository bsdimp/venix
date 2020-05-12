Submissions from:
Department of Chemistry
Harvard University
12 Oxford Street
Cambridge, MA 02138

UMODEM
This is a file transfer program which uses the CP/M User's Group
MODEM protocol.  It is helpful in performing transfers between a
VAX/VMS machine and CP/M-based microcomputers, and is a lot
cheaper than the DEC offering!
     The program was originally implemented under Version 7 UNIX,
and is written entirely in C.  All of the modifications made to
the program for VMS (VAX-11 C) are in #ifdef statements, so the
program still compiles properly under V7 UNIX.
     The VMS support routines required by the program to emulate
UNIX "raw" I/O and the gtty(), stty(), and stat() system services
are in a separate module, VMODEM.C.  This module alone should be
interesting to those who are interested in learning how to call
system services and perform QIOs using VAX-11 C.
     UMODEM consists of five files:  UMODEM.C, the main program
(the only file needed to compile the program under V7 UNIX);
VMODEM.C, VMS support routines (not needed under UNIX); VMODEM.H,
a header file including structure definitions needed for the VMS
routines; UMODEM.HLP, a help file in VMS HELP format; and
UMODEM.EXE.  All that's needed to run the program is to define a
symbol for a foreign command:  "$ umodem :== $umodem.exe".
     The VMS version of UMODEM is by Walter Reiher; the UNIX
version is by Lauren Weinstein, Richard Conn, and Bennett Marks.
Thanks to the C Line computer bulletin board system for making
the UNIX version available and to Max Benson and Robert
Bruccoleri for coding examples which made VMODEM.C easier to
implement.
     [The CP/M program MODEM, which is also needed to perform
file transfers, is available on a number of public computer
bulletin board systems and user groups.  WR has a version of
MODEM configured for the Heath/Zenith-89 microcomputer system
which is available for the asking.]


DISPLAY
This command file makes the VMS Version 3 MONITOR program look
like the Version 2 DISPLAY program.  For those of us who prefer
the old syntax, this beats having to type those long MONITOR
commands or defining a slew of symbols.
     DISPLAY.COM is by Andrew Cherenson.


Submitted by:  Walter Reiher, 617-495-1768
