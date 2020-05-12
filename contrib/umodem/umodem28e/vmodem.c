/*
 * VMODEM -- Implements the "CP/M User's Group XMODEM FTP 3" protocol
 *	     for packetized file up/downloading for VAX/VMS.
 *
 * Modification History:
 *
 * August 23, 1985 by Robin Miller.  Version 2.9
 *   o	When receiving binary files, open the output file with
 *	fixed 128 byte records so they are readable by utilities.
 *   o	Change estimated transfer time calculation from 300 baud
 *	to 1200 baud since that speed is used more often.
 *   o	Change all references of UMODEM to VMODEM for VMS.
 *   o	Send CANcel byte when reporting errors and exiting.
 *   o	Document the code (too hard to maintain otherwise).
 *
 * April 19, 1984 by Robin Miller.  Version 2.8
 *	Remove all conditionals to generate code for VMS only.
 *	Don't send the last block twice when sending binary files.
 */

#include stdio

#include ssdef
#include tt2def
#include ttdef
#include "vmodem.h"

#define VERSION		29		/* Version Number. */
#define TRUE		1               
#define FALSE		0
#define SOH		001		/* Start of header. */
#define EOT		004		/* End of Transmission. */
#define ACK		006		/* Acknowlegment */
#define LF		012		/* Linefeed - Unix Newline. */
#define CR		015		/* Carriage return. */
#define NAK		025		/* Negative Acknowlegment. */
#define SYN		026		/* Synchronize. */
#define CAN		030		/* Cancel. */
#define CTRLZ		032		/* CP/M EOF for text (usually!). */
#define TIMEOUT		-1		/* Timeout occurs value. */
#define ERRORMAX	10		/* Maximum errors tolerated. */
#define RETRYMAX	10		/* Maximum retries per block. */
#define BBUFSIZ		128		/* Buffer size -- do not change! */
#define CREATMODE	0644		/* Mode for created files. */

/*  VMS structures  */
/*
 *	TT_INFO structures are used for passing information about
 *	the terminal.  Used in GTTY and STTY calls.
 */
struct  tt_info ttys, ttysnew, ttystemp;

FILE    *LOGFP, *fopen();
char    buff[BBUFSIZ + 1];

char    XMITTYPE;				/* Type of file transfer. */
int     BIT7, BITMASK, DELFLAG, LOGFLAG;
int     PMSG, RECVFLAG, SENDFLAG, STATDISP;
int     delay;

/*
 * main - Start of the VMODEM program.
 *
 * If the user doesn't specify a command line, display the help text
 * and then exit the program.
 *
 */
main(argc, argv)
int argc;
char **argv;
{
	char *logfile;
	int index;
	char flag;

	logfile = "vmodem.log";			/* Name of log File. */

	printf ("\nVMODEM Version %d.%d", VERSION/10, VERSION%10);
	printf (" -- VAX/VMS-Based Remote File Transfer Facility\n");

	if (argc < 3 || *argv[1] != '-')
	{   printf ("\nUsage:  \n\tvmodem ");
		printf ("-[rb!rt!sb!st][l][p][y][7] filename\n");
	    printf ("\n");
	    printf ("\trb <-- Receive Binary.\n");
	    printf ("\trt <-- Receive Text.\n");
	    printf ("\tsb <-- Send    Binary.\n");
	    printf ("\tst <-- Send    Text.\n");
	    printf ("\td  <-- Create vmodem.log else append to existing.\n");
	    printf ("\tl  <-- (ell) Turn OFF LOG File Entries.\n");
	    printf ("\ty  <-- Display file status (size) information only.\n");
	    printf ("\t7  <-- Enable 7-bit transfer mask.\n");
	    printf ("\n");
	    exit (SS$_NORMAL);		/* Exit to the system. */
	}
/*
 *      Initializations:
 */
	index		= 1;		/* Set index for loop. */
	delay		= 3;		/* Set FTP 3 delay. */
	RECVFLAG	= FALSE;	/* Init receive flag. */
	SENDFLAG	= FALSE;	/* Init send flag. */
	XMITTYPE	= 't';		/* Default to text mode. */
	LOGFLAG		= TRUE;		/* Enable log messages. */
	STATDISP	= FALSE;	/* Disable status display. */
	BIT7		= FALSE;	/* Assume 8-bit communication. */
	DELFLAG		= FALSE;	/* Append to existing log file. */
/*
 *	Parse the command line switches:
 */
	while ((flag = argv[1][index++]) != '\0')
	    switch (flag) {
		case '7' : BIT7 = TRUE;		/* Transfer only 7 bits. */
			   break;

		case 'd' : DELFLAG = TRUE;	/* Delete log file first. */
			   break;
		case 'L':
		case 'l' : LOGFLAG = FALSE;	/* Turn off log report. */
			   break;
		case 'R':
		case 'r' : RECVFLAG = TRUE;	/* Receive a file. */
			   XMITTYPE = gettype (argv[1][index++]); /* get t/b */
			   break;
		case 'S':
		case 's' : SENDFLAG = TRUE;	/* Send a file. */
			   XMITTYPE = gettype(argv[1][index++]);
			   break;

		case 'Y':
		case 'y' : STATDISP = TRUE;	/* Display file status. */
			   break;

		default  : error ("Invalid Flag", FALSE);
	    }

	if (BIT7 && (XMITTYPE == 'b'))
	{
	    printf ("\nVMODEM:  Fatal Error -- Both 7-Bit Transfer and ");
	    printf ("Binary Transfer Selected");
	    exit(SS$_NORMAL);
	}

	if (BIT7)				/* Set MASK value. */
	    BITMASK = 0177;			/* 7 significant bits. */
	else
	    BITMASK = 0377;			/* 8 significant bits. */

	if (LOGFLAG)
	{   if (!DELFLAG)
		LOGFP = fopen (logfile, "a");	/* Append to LOG file. */
	    else
		LOGFP = fopen (logfile, "w");	/* Open new LOG file. */
	    fprintf (LOGFP,"\n\n++++++++\n");
	    fprintf (LOGFP,"\nVMODEM Version %d.%d\n", VERSION/10, VERSION%10);
	    printf ("\nVMODEM:  LOG File '%s' is Open\n", logfile);
	}

	if (STATDISP) yfile (argv[2]);		/* Display status of a file. */

	if (RECVFLAG && SENDFLAG)
	    error ("Both Send and Receive Functions Specified", FALSE);
	if (!RECVFLAG && !SENDFLAG)
	    error ("Neither Send nor Receive Functions Specified", FALSE);
/*
 *	Receive or send a file.
 */
        if (RECVFLAG)
		rfile (argv[2]);		/* Receive a file. */
        else
		sfile (argv[2]);		/* Send a file. */
}

/*
 * gettype - Get the file transfer type.
 *
 * This function is used to get the file transfer type which can be
 * either 'b' for binary, or 't' for text.  If an invalid type is
 * specified, an error message is displayed and we exit the program.
 *
 * Inputs:
 *	ichar = The character to check.
 *
 */
gettype (ichar)
char ichar;
{
	switch (ichar)
	{
	    case 'b':       case 'B':		/* Binary file transfer. */
	    case 't':       case 'T':		/* Text file transfer. */
			return (ichar);		/* Return transfer type. */
	    default:
		error ("Invalid Send/Receive Parameter - not t or b", FALSE);
	}
	return;
}

/* set tty modes for VMODEM transfers */
setmodes()
{

/*  Device characteristics for VMS  */

        int     *iptr, parameters;
/*
 *      Get current terminal parameters.
 */
        if (gtty(&ttys) != SS$_NORMAL)
                error ("SETMODES:  error return from GTTY (1)", FALSE);
        if (gtty(&ttysnew) != SS$_NORMAL)
                error ("SETMODES:  error return from GTTY (2)", FALSE);
/*
 *      Set new terminal parameters.
 *      Note that there are three bytes of terminal characteristics,
 *      so we should make sure the fourth byte of the integer is unchanged.
 */
        iptr    = &(ttysnew.dev_characteristics.bcharacteristics);
        parameters      = *iptr;

        parameters      &= ~TT$M_ESCAPE;                /*  ESCAPE   OFF  */
        parameters      &= ~TT$M_HOSTSYNC;              /*  HOSTSYNC OFF  */
        parameters      |=  TT$M_NOECHO;                /*  NOECHO   ON   */
        parameters      |=  TT$M_PASSALL;               /*  PASSALL  ON   */
        parameters      &= ~TT$M_READSYNC;              /*  READSYNC OFF  */
        parameters      &= ~TT$M_TTSYNC;                /*  TTSYNC   OFF  */
        parameters      &= ~TT$M_WRAP;                  /*  WRAP     OFF  */
        if (! BIT7)
                parameters      |= TT$M_EIGHTBIT;       /*  EIGHTBIT ON  */

        *iptr           = parameters;

        if (stty(&ttysnew) != SS$_NORMAL)
                error ("SETMODES:  error return from STTY", TRUE);
        return;
}

/*
 * restoremodes - Restore the normal terminal modes.
 *
 * Inputs:
 *	errcall = ?
 *
 */
restoremodes (errcall)
int errcall;
{
/*
 *	Device characteristic restoration for VMS.
 */
	if (stty (&ttys) != SS$_NORMAL)		/* Restore original modes. */
	{
	    if (!errcall)
		error ("Error restoring original terminal params.", FALSE);
	    else
		{
		    printf ("VMODEM/RESTOREMODES:  ");
		    printf ("Error restoring original terminal params.\n");
		}
	}
        return;
}

/*
 * error - Report error message.
 *
 * This function is used to print an error message and exit to the system.
 *
 * Inputs:
 *	mode = If TRUE, restore the original TTY modes.
 *
 */
error (msg, mode)
char *msg;
int mode;
{
	if (mode)				/* In file transmission. */
	{
	    sendbyte (CAN);			/* Cancel the HOST. */
	    restoremodes (TRUE);		/* Restore tty modes. */
	}
	printf ("VMODEM:  %s\n", msg);
	if (LOGFLAG)
	{   fprintf (LOGFP, "VMODEM Fatal Error:  %s\n", msg);
	    fclose (LOGFP);
	}
	exit (SS$_NORMAL);			/* Exit to the system. */
}

/*
 * rfile - Receive a file.
 *
 * Inputs:
 *	name = The name of the output file.
 *
 */
rfile(name)
char *name;
{
	char mode;
	int fd, j, firstchar, sectnum, sectcurr, tmode;
	int sectcomp, errors, errorflag, recfin;
	register int bufctr, checksum;
	register int c;
	int errorchar, fatalerror, inchecksum;
	long recvsectcnt;
	int	lowlim, nlflag;
	char	inbuf [BBUFSIZ + 7];		/* Allocate input buffer */

	mode = XMITTYPE;			/* Copy transmit type. */
	if (mode == 't' || mode == 'T')
	    tmode = TRUE;			/* Text mode enabled. */
	else
	    tmode = FALSE;			/* Binary mode enabled. */
/*
 *	For text files, use a normal create which creates output file
 *	with stream_LF attributes.  For binary files, create the file
 *	with fixed length 128 byte records so other VMS utilities and
 *	especially so the MISH program can access this file.
 *
 */
	if (tmode)
	{
	    if ((fd = creat (name, CREATMODE)) < 0)
		error ("Can't create file for receive text", FALSE);
	}
	else
	{
	    if ((fd = creat (name, CREATMODE,
			"bls=128", "mrs=128", "rfm=fix")) < 0)
		error ("Can't create file for receive binary", FALSE);
	}
	prstats (name);				/* Print start statistics. */
	setmodes();				/* Setup the tty modes. */
	recfin = FALSE;				/* Init record end flag. */
	sectnum = errors = 0;			/* Init sector/error count. */
	fatalerror = FALSE;			/* Init fatal error flag. */
	recvsectcnt = 0;			/* Init received sectors. */
/*
 *	We're ready to receive the file, send the sync byte.
 */
	sendbyte (NAK);				/* FTP 3 Sync character. */
	nlflag = FALSE;				/* Init the newline flag. */

	do
	{
	  errorflag = FALSE;			/* Reset error seen flag. */
	  do
	  {
	    firstchar = readbyte (6);		/* Read first character. */
	  } while ((firstchar != SOH) && (firstchar != EOT)
			&& (firstchar != CAN) && (firstchar != TIMEOUT));

	  if (firstchar == CAN)
		do_cancel (fd);			/* Do cancel processing. */

	  if (firstchar == TIMEOUT)
	  {
	    if (LOGFLAG)
	      fprintf (LOGFP, "Timeout on Sector %d\n", sectnum);
	    errorflag = TRUE;			/* Show error was seen. */
          }

	  if (firstchar == SOH)
	  {
/*
 *      Under VMS, read the whole block at once.
 */
	    raw_read (BBUFSIZ + 3, inbuf, delay); /* Read next block. */
	    sectcurr = inbuf[0] & BITMASK;	/* Copy sector number. */
	    sectcomp = inbuf[1] & BITMASK;	/* Copy complemented #. */

	    if ((sectcurr + sectcomp) == BITMASK)
	    {
	      if (sectcurr == ((sectnum + 1) % 256) & BITMASK)
	      {
		checksum = 0;			/* Init the checksum. */
		lowlim = 2;			/* Set the buffer index. */

		bufctr = 0;			/* Init the buffer counter. */

		for (j = lowlim; j < lowlim + BBUFSIZ; j++)
		{
		  buff[bufctr] = c = inbuf[j] & BITMASK;
		  checksum = (checksum+c)&BITMASK;
		  if (!tmode)			/* Binary mode. */
		  {
		    bufctr++;			/* Just copy the byte. */
		    continue;			/* And process the next. */
		  }
/*
 *	Translate CP/M's CR/LF into UNIX's LF, but don't do it by
 *	simply ignoring a CR from CP/M--it may indicate an
 *	overstruck line!
 */
		  if (nlflag)
		  {
/*
 *	Last packet ended with a CR.  If the first character of this
 *	packet ISN'T a LF, then be sure to insert the CR.
 */
		    nlflag  = FALSE;		/* Init newline flag. */
		    if (c != LF)
		    {
		      buff[bufctr++] = CR;	/* Insert the newline. */
		      buff[bufctr] = c;		/* Copy this character. */
		    }
		  }
		  if (j == lowlim + BBUFSIZ - 1 && c == CR)
		  {
/*
 *	Last character in the packet is a CR.  Don't send it, but
 *	turn on NLFLAG to make sure we check the first character in
 *	the next packet to see if it's a LF.
 */
		    nlflag = TRUE;		/* Show newline seen. */
		    continue;			/* Do the next character. */
		  }
		  if (c == LF && bufctr && buff[bufctr - 1] == CR)
		  {
/*
 *	We have a CR/LF pair.  Discard the CR.
 */
		    buff[bufctr - 1] = LF;	/* Overwrite CR with LF. */
		    continue;			/* Do the next character. */
		  }
		  if (c == CTRLZ)		/* If EQ, CP/M EOF char. */
		  {
		    recfin = TRUE;		/* Set the EOF flag. */
		    continue;			/* And continue ... */
		  }
		  if (!recfin)			/* If EOF not detected, */
		    bufctr++;			/* Adjust output index. */
		}
/*
 *	Process the checksum.
 */
		inchecksum = inbuf[lowlim + BBUFSIZ] & BITMASK;
                        
		if (checksum == inchecksum)	/* Good checksum. */
		{
		  errors = 0;			/* Reset error counter. */
		  recvsectcnt++;		/* Count received sector. */
		  sectnum = sectcurr;		/* Update sector counter. */
		  if (write (fd, buff, bufctr) < 0) /* Write this block. */
		    error ("File Write Error", TRUE);
		  else
		    sendbyte (ACK);		/* Acknowlege last block. */
		}
		else
		{
		  if (LOGFLAG)
		    fprintf (LOGFP, "Checksum Error on Sector %d\n",
								sectnum);
		  errorflag = TRUE;		/* Show error seen. */
		}
	      }
	      else				/* Not expected sector. */
	      {
		if (sectcurr == (sectnum % 256) & BITMASK)
		{				/* Previous sector. */
		  while (readbyte(3) != TIMEOUT); /* Read any garbage. */
		    sendbyte (ACK);		/* ACK the last block. */
		}
		else				/* Unexpected sector. */
		{
		  if (LOGFLAG)
		  {
		    fprintf (LOGFP, "Phase Error--received sector is %d ",
								  sectcurr);
		    fprintf (LOGFP, "while expected sector is %d (%d)\n",
				((sectnum + 1) % 256) & BITMASK, sectnum);
		  }
		  errorflag = TRUE;		/* Show error seen. */
		  fatalerror = TRUE;		/* Set fatal error flag. */
		  sendbyte (CAN);		/* Send CANcel to HOST. */
		}
	      }
	    }
	    else				/* Missing start of header. */
	    {
	      if (LOGFLAG)
		fprintf (LOGFP, "Header Sector Number Error on Sector %d\n",
								    sectnum);
	      errorflag = TRUE;			/* Show error was seen. */
	    }
	  }
	  if (errorflag == TRUE)		/* If error was seen, */
	  {
	    errors++;				/* Adjust error count. */
	    while (readbyte(3) != TIMEOUT);	/* Read any garbage. */
	    sendbyte (NAK);			/* NAK the last block. */
	  }
	} while ((firstchar != EOT) && (errors != ERRORMAX) && !fatalerror);

	if ((firstchar == EOT) && (errors < ERRORMAX))
	{
	  sendbyte (ACK);			/* ACK the EOT byte. */
	  close (fd);				/* Close output file. */
	  restoremodes (FALSE);			/* Restore tty modes. */
	  sleep (3);	/* Give other side time to return to terminal mode */
	  if (LOGFLAG)
	  {
	    fprintf (LOGFP, "\nReceive Complete\n");
	    fprintf (LOGFP,"Number of Received CP/M Records is %ld\n",
								recvsectcnt);
	    fclose (LOGFP);			/* Close the log file. */
	  }
	  printf ("\n");
	  exit (SS$_NORMAL);			/* Exit to the system. */
	}
	else
	{
	  if (errors == ERRORMAX)		/* Exceeded maximum. */
	    error ("Too many errors", TRUE);
	  if (fatalerror)			/* Fatal error detected. */
	    error ("Fatal error", TRUE);
	}
}

/*
 * sfile - Send a file.
 *
 * Inputs:
 *	name = The name of the file to send.
 *
 */
sfile(name)
char *name;
{
	char mode;
	int fd, attempts, firstchar, charval;
	int nlflag, sendfin, tmode;
	register int bufctr, checksum, optr, sectnum;
	char c;
	int	sendresp;			/* Response to sent block. */
	char	outbuf [BBUFSIZ + 7];		/* Allocate output buffer. */

	mode = XMITTYPE;			/* Copy the transmit mode. */
	if (mode == 't' || mode == 'T')
	    tmode   = TRUE;			/* Text mode enabled. */
	else
	    tmode   = FALSE;			/* Binary mode enabled. */

        if ((fd = open (name, 0)) < 0)		/* Open the file to send. */
        {  if (LOGFLAG) fprintf (LOGFP, "Can't Open File\n");
           error ("Can't open file for send", FALSE);
        }
	prstats (name);				/* Print start statistics. */
	setmodes();				/* Setup the tty modes. */

	sendfin = nlflag = FALSE;		/* Init finish/nline flags. */
	attempts = 0;				/* Init the rettry count. */
/*
 *	Wait for the sync byte from the HOST.
 */
	do
	{
	    firstchar = readbyte (30);		/* Read first character. */
	} while ((firstchar != NAK) && (firstchar != CAN)
			&& (++attempts < RETRYMAX));

	if (firstchar == CAN)
	    do_cancel (fd);			/* Do cancel processing. */

	if (attempts == RETRYMAX)		/* Retry limit exceeded. */
           error ("Remote System Not Responding", TRUE);

	sectnum = 1;				/* Set 1st sector number. */
	attempts = 0;				/* Reinit retry counter. */
/*
 *	Start sending the file.
 */
	do 
	{
	  for (bufctr=0; bufctr < BBUFSIZ;)	/* Fill the buffer. */
	  {
	    if (nlflag)
	    {
	      buff[bufctr++] = LF;		/* Leftover newline. */
	      nlflag = FALSE;			/* Reset newline flag. */
	    }
	    if ((charval = read (fd, &c, 1)) < 0) /* Error reading file. */
		error ("File Read Error", TRUE);

	    if (charval == 0)			/* EOF for read. */   
	    {
	      sendfin = TRUE;			/* This is the last sector. */
	      if (tmode)
		buff[bufctr++] = CTRLZ;		/* Control-Z for CP/M EOF. */
	      else
	      {
		if (bufctr == 0)		/* End of input buffer. */
		  break;			/* Nothing more to send. */
		else
		  bufctr++;          		/* Partial binary file. */
	      }
	      continue;
	    }
	    if (tmode && c == LF)		/* Text mode & newline ? */
	    {
	      if (c == LF)			/* Unix newline ? */
	      {
		buff[bufctr++] = CR;		/* Insert carriage return. */
		if (bufctr < BBUFSIZ)		/* If room, add newline. */
		  buff[bufctr++] = LF;		/* Insert Unix newline. */
		else
		  nlflag = TRUE;		/* Insert newline next sector */
	      }
	      continue;				/* Do the next character. */
	    }
	    buff[bufctr++] = c;			/* Copy char without change. */
	  }
/*
 *	The buffer is full, build the transmit buffer.
 */
	  attempts = 0;				/* Reset the retry count. */
	  if (bufctr == 0) break;		/* Nothing to send. */
/*
 *	Prepare the buffer to transmit.
 *
 *	Format:  <SOH><block #><comp block #>< 128 data bytes ><checksum>
 *
 */
	  optr = 0;				/* Init output pointer. */
	  outbuf[optr++] = SOH;			/* Start with SOH byte. */
	  outbuf[optr++] = sectnum;		/* Fill in sector number. */
	  outbuf[optr++] = -sectnum-1;		/* And its complement. */
/*
 *	Calculate the checksum.
 */
	  checksum = 0;			/* Init the checksum. */
	  for (bufctr=0; bufctr < BBUFSIZ; bufctr++)
	  {
	    outbuf[optr++] = buff[bufctr];	/* Copy the next byte. */
	    checksum = (checksum+buff[bufctr])&BITMASK;
	  }
/*	  while (readbyte(3) != TIMEOUT);	/ flush chars from line */
	  outbuf[optr++] = checksum;		/* Copy the checksum. */
/*
 *	Ready to transmit this buffer.
 */
	  do
	  {
	    raw_write (outbuf, BBUFSIZ+4);	/* Write transmit buffer. */
	    attempts++;				/* Adjust retry counter. */
	    sendresp = readbyte(10);		/* Get the response. */
	    if ((sendresp != ACK) && LOGFLAG)	/* Did not receive ACK */
	    {
	      fprintf (LOGFP, "Non-ACK Received on Sector %d\n", sectnum);
	      if (sendresp == TIMEOUT)
                        fprintf (LOGFP, "This non-ACK was a TIMEOUT\n");
	      else
		if (sendresp == NAK)
		  fprintf (LOGFP, "This non-ACK was a NAK\n");
	    }
	    if (sendresp == CAN)		/* Did we receive CANcel? */
	    {
	      if (LOGFLAG)
		fprintf (LOGFP, "This non-ACK was a CAN\n");
		do_cancel (fd);			/* Do CANcel processing. */
	    }
	  } while ((sendresp != ACK) && (attempts != RETRYMAX));
	  sectnum++;				/* Adjust sector number. */
	} while (!sendfin && (attempts != RETRYMAX));
/*
 *	Done sending file or retry limit exceeded.
 */
	if (attempts == RETRYMAX)		/* Retry limit exceeded. */
	    error ("Remote System Not Responding", TRUE);

	attempts = 0;				/* Reinit retry counter. */
	sendbyte (EOT);				/* Send 1st EOT byte. */
	while ((readbyte(15) != ACK) && (attempts++ < RETRYMAX))
	    sendbyte(EOT);			/* Send another EOT. */

	if (attempts >= RETRYMAX)		/* Retry limit exceeded. */
	    error ("Remote System Not Responding on Completion", TRUE);

	close (fd);				/* Close output file. */
	restoremodes (FALSE);			/* Restore tty modes. */
	sleep (3);	/* Give other side time to return to terminal mode */
	if (LOGFLAG)
	{
	  fprintf (LOGFP, "\nSend Complete\n");
	  fclose (LOGFP);			/* Close the log file. */
	}
	printf ("\n");				/* Write last newline. */
	exit (SS$_NORMAL);			/* Exit to the system */
}

/*
 * do_cancel - Do CANcel received processing.
 *
 * This function is used to do final processing after a CANcel byte
 * has been read from the HOST system.
 *
 * Inputs:
 *	fd = File descriptor of file to close.
 *
 */
do_cancel (fd)
int fd;
{
	if (LOGFLAG)
	    fprintf (LOGFP, "Exiting:  Received a CANcel from the HOST.\n");

	close (fd);				/* Close the open file. */
	restoremodes (TRUE);			/* Restore terminal modes. */
	sleep (5);				/* Allow HOST to recover. */
	printf ("Exiting:  Received a CANcel from the HOST.\n");
	exit (SS$_NORMAL);			/* Exit to the system. */
}

/*
 * prfilestat - Print file size statistics.
 *
 * This function is used to calculate and display the estimated file
 * transfer time.  It presently calculates the time for 1200 baud.
 *
 * Inputs:
 *	name = Name of file being sent.
 *
 */
prfilestat (name)
char *name;
{
	long bytes, Kbytes, records;

	bytes = filestat (name);		/*  Get the file size. */
	if (bytes < 0)
	{
	    printf ("PRFILESTAT:  error return from FILESTAT\n");
	    return;
	}
	Kbytes          = (bytes / 1024) + 1;	/* Calculate K bytes. */
	records         = (bytes /  128) + 1;	/* Calculate record count. */

	printf ("\nVMODEM:  Estimated File Size %ldK, %ld Records, %ld Bytes",
					Kbytes, records, bytes);
	printf ("\n         Estimated transfer time at 1200 baud:  ");
	printf ("%ld min, %ld sec.",
			bytes / (120*60), ((bytes % (120*60)) / 120) + 1);

	if (LOGFLAG)
	    fprintf (LOGFP,
			"Estimated File Size %ldK, %ld Records, %ld Bytes\n",
			Kbytes, records, bytes);
	return;
}

/*
 * prstats - Print the startup statistics.
 *
 * This function is used to print the startup statistics to the terminal
 * and to the log file if one is open.
 *
 * Inputs:
 *	name = The file name to send or receive.
 *
 */
prstats (name)
char *name;
{
	char mode;
	int tmode;				/* Text mode flag. */

	mode = XMITTYPE;			/* Copy the transmit type. */
	if (mode == 't' || mode == 'T')
	    tmode = TRUE;			/* Text mode enabled. */
	else
	    tmode = FALSE;			/* Binary mode enabled. */

	if (LOGFLAG)
	{   if (RECVFLAG)
		fprintf (LOGFP, "\n----\nVMODEM Receive Function\n");
	    else
		fprintf (LOGFP, "\n----\nVMODEM Send Function\n");

	    fprintf (LOGFP, "File Name: %s\n", name);
	    if (SENDFLAG)			/* If sending a file, */
		prfilestat(name);		/* print file size stats. */

	    fprintf (LOGFP,
		"TERM II File Transfer Protocol 3 (CP/M UG) Selected\n");

	    if (tmode)
		fprintf (LOGFP, "Text Mode Selected\n");
	    else
		fprintf (LOGFP, "Binary Mode Selected\n");

	    if (BIT7)
		fprintf (LOGFP, "7-Bit Transmission Enabled\n");
	    else
		fprintf (LOGFP, "8-Bit Transmission Enabled\n");
	}
	printf ("\r\nVMODEM:  File Name: %s", name);
	printf ("\r\nVMODEM:  ");
	if (tmode)
	    printf ("Text Mode Selected");
	else
	    printf ("Binary Mode Selected");

	printf ("\r\nVMODEM:  ");
	if (BIT7)
	    printf ("7-Bit");			/* 7 bits data enabled. */
	else
	    printf ("8-Bit");			/* 8 bits data enabled. */
	printf (" Transmission Enabled");

	if (RECVFLAG)
	    printf ("\r\nVMODEM:  Ready to RECEIVE File\r\n");
	else
	    printf ("\r\nVMODEM:  Ready to SEND File\r\n");

	return;
}

/*
 * readbyte - Read a single byte with timeout.
 *
 * Inputs:
 *	seconds = The timeout count in seconds.
 *
 * Outputs:
 *	Function value = TIMEOUT if a timeout occured,
 *			 else, returns the character read.
 *	(Note:  The return value is an int not a char.)
 *
 */
readbyte (seconds)
unsigned seconds;
{
	int c;				/* Buffer for character read. */

	c = raw_read (1, &c, seconds);	/* Read a single character. */
	if (c == SS$_TIMEOUT)		/* If a timeout occured, */
		return (TIMEOUT);	/*  return the timeout value. */
	return (c & BITMASK);		/* Else, return the char. */
}

/*
 * sendbyte - Send a single byte.
 *
 * Inputs:
 *	data = The character to send.
 *
 */
sendbyte (data)
char data;
{
	char dataout;				/* The character to write. */

	dataout = data & BITMASK;               /* Mask for 7 or 8 bits. */
	raw_write (&dataout, 1);		/* Write the character. */
	return;
}

/*
 * yfile - Print statistics for a file.
 *
 * This function is used to calculate the approximate file transfer of
 * the specified file.
 *
 * Inputs:
 *	name = Name of file to print statistics for.
 *
 */
yfile (name)
char *name;
{
	printf ("VMODEM:  File Status Display for %s\n", name);
	if (LOGFLAG)
	    fprintf (LOGFP,"VMODEM File Status Display for %s\n", name);

	if (open (name,0) < 0)			/* Try to open the file. */
	{
	    printf ("File %s does not exist\n", name);
	    if (LOGFLAG) fprintf (LOGFP,"File %s does not exist\n", name);
	    exit (SS$_NORMAL);
        }

	prfilestat (name);			/* Print the status. */
	printf ("\n");				/* Add a final newline. */
	if (LOGFLAG)
	{
	    fprintf (LOGFP,"\n");		/* Add a final newline. */
	    fclose (LOGFP);			/* Close the log file. */
        }
	exit (SS$_NORMAL);			/* Exit to the system. */
}
