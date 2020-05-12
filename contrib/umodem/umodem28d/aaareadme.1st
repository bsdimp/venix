       			       Free software BY
		     Project Software & Development, Inc.

     This  software  is  furnished for free and may be used and  copied as
     desired. This software or  any  other copies  thereof may be provided
     or  otherwise  made  available  to any other person.  No title to and
     ownership of  the  software  is  hereby transferred or allowed. 

     The information in this software is subject to change  without notice
     and  should  not  be  construed  as  a commitment by PROJECT SOFTWARE
     AND DEVELOPMENT, INC.

     PROJECT SOFTWARE assumes no responsibility for the use or reliability  
     of this software on any equipment whatsoever.

	Project Software & Development, Inc.
	14 Story St.
	Cambridge, Ma. 02138
	617-661-1444

	Program:	VMODEM
	Author:		Robin Miller
	Date:		Fall 1984

     Description:

	VMODEM is  the VMS version of the UMODEM program which uses the
     XMODEM  protocol  used on many micros.  I've included this program
     because  the VAXNET program now supports this protocol and you may
     wish  to  use  this program instead of the SNDRCV program for file
     transfers.  The XMODEM protocol has better error checking than the
     SNDRCV protocol but only allows sending or receiving a single file
     at a time.  It also has the advantage of running on UNIX where the
     SNDRCV program does not.

     WARNING:

	Receiving  files from the VMODEM program works just fine (doing
     a  VAXNET  GET  command).  But sending files to VMODEM has lots of
     overhead since it does single character reads.  At slow baud rates
     this isn't too bad but at higher baud rates it eats up VMS.
