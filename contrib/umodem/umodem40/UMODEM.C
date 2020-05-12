/*
 *  UMODEM Version 4.0
 *
 *  UMODEM -- Implements the "CP/M User's Group XMODEM" protocol, 
 *            the TERM II File Transfer Protocol (FTP) Number 1,
 *            and the TERM II File Transfer Protocol Number 4 for
 *            packetized file up/downloading.    
 *
 *    Note: UNIX System-Dependent values are indicated by the string [SD]
 *          in a comment field on the same line as the values.
 *
 *
 *         -- Lauren Weinstein, 6/81
 *         -- (Version 2.0) Modified for JHU/UNIX by Richard Conn, 8/1/81
 *         -- Version 2.1 Mods by Richard Conn, 8/2/81
 *              . File Size Included on Send Option
 *         -- Version 2.2 Mods by Richard Conn, 8/2/81
 *              . Log File Generation and Option Incorporated
 *         -- Version 2.3 Mods by Richard Conn, 8/3/81
 *              . TERM II FTP 1 Supported
 *              . Error Log Reports Enhanced
 *              . CAN Function Added to FTP 3
 *              . 'd' Option Added to Delete umodem.log File before starting
 *         -- Version 2.4 Mods by Richard Conn, 8/4/81
 *              . 16K-extent sector number check error corrected
 *              . Count of number of received sectors added
 *         -- Version 2.5 Mods by Richard Conn, 8/5/81
 *              . ARPA Net Flag added
 *              . ARPA Net parameter ('a') added to command line
 *              . ARPA Net BIS, BIE, BOS, BOE added
 *              . ARPA Net FFH escape added
 *         -- Version 2.6 Mods by Bennett Marks, 8/21/81 (Bucky @ CCA-UNIX)
 *              . mods for UNIX V7 (Note: for JHU compilation define
 *                the variable JHU  during 'cc'
 *              . added 'mungmode' flag to protect from inadvertant
 *                overwrite on file receive
 *              . changed timeout handling prior to issuing checksum
 *         -- Version 2.7 Mods by Richard Conn, 8/25/81 (rconn @ BRL)
 *              . correct minor "ifndef" error in which ifndef had no arg
 *              . restructured "ifdef" references so that other versions
 *                of UNIX than Version 7 and JHU can be easily incorporated;
 *                previous ifdef references were for JHU/not JHU;
 *                to compile under Version 7 or JHU UNIX, the following
 *                command lines are recommended:
 *                      "cc umodem.c -o umodem -DVER7" for Version 7
 *                      "cc -7 umodem.c -o umodem -DJHU" for JHU
 *              . added 'y' file status display option; this option gives
 *                the user an estimate of the size of the target file to
 *                send from the UNIX system in terms of CP/M records (128
 *                bytes) and Kbytes (1024 byte units)
 *              . added '7' option which modifies the transmission protocols
 *                for 7 significant bits rather than 8; modifies both FTP 1
 *                and FTP 3
 *         -- Version 2.8 Mods by Richard Conn, 8/28/81
 *              . corrected system-dependent reference to TIOCSSCR (for
 *                disabling page mode) and created the PAGEMODE flag which
 *                is to be set to TRUE to enable this
 *              . added -4 option which engages TERM II, FTP 4 (new release)
 *         -- Version 2.9 Mods by Richard Conn, 9/1/81
 *              . internal documentation on ARPA Net protocols expanded
 *              . possible operator precedence problem with BITMASK corrected
 *                by redundant parentheses
 *         -- Version 3.0 Mods by Lauren Weinstein, 9/14/81
 *              . fixed bug in PAGEMODE defines (removed PAGEMODE define
 *                line; now should be compiled with "-DPAGEMODE" if
 *                Page Mode is desired)
 *              . included forward declaration of ttyname() to avoid problems
 *                with newer V7 C compilers
 *         -- Version 3.1 Mods by Lauren Weinstein, 4/17/82
 *              . avoids sending extraneous last sector when file EOF
 *                occurs on an exact sector boundary
 *         -- Version 3.2 Mods by Michael M Rubenstein, 5/26/83
 *              . fixed bug in readbyte.  assumed that int's are ordered
 *                from low significance to high
 *              . added LOGDEFAULT define to allow default logging to be
 *                off.  compile with -DLOGDEFAULT=0 to set default to no
 *                logging.
 *         -- Version 3.3 Mod by Ben Goldfarb, 07/02/83
 *              . Corrected problem with above implementation of "LOGDEFAULT".
 *                A bitwise, instead of a logical negation operator was
 *                used to complement LOGFLAG when the '-l' command line
 *                flag was specified.  This can cause LOGFLAG to be true
 *                when it should be false.
 *         -- Version 3.4 Mods by David F. Hinnant, NCECS, 7/15/83
 *              . placed log file in HOME directory in case user doesn't
 *                have write permission in working directory.
 *              . added DELDEFAULT define to allow default purge/no purge
 *                of logfile before starting.  Compile with -DDELDEFAULT=0
 *                to set default to NOT delete the log file before starting.
 *              . check log file for sucessful fopen().
 *              . buffer disk read for sfile().
 *              . turn messages off (standard v7) before starting.
 *         -- Version 3.5 Mods by Richard Conn, 08/27/83
 *              . added equates for compilation under UNIX SYSTEM III
 *                      to compile for SYSTEM III, use -DSYS3 instead of
 *                      -DJHU or -DVER7
 *              . added command mode (-c option) for continuous entry
 *                      of commands
 *          --  Version 4.0 Mods by Mycroft Holmes, 04/17/84
 *                . released constraint on using caps for commands in
 *                        the 'command' mode. 
 *                . added descriptors for local mode, allowing use of
 *                        environmental variable 'MODEM' to specify
 *                        i/o port. now, no more limitation to remote
 *                        usage only.
 *                . added rudamentary terminal program for local mode
 *                        and (ala modem7 series, thankyou
 *                        WC) ^E exits terminal mode.  
 *                . reassigned SIGINT to command() when in local mode, thus
 *                        no die from program, just return to command mode.
 *                . put the conditional (on local mode) display of sector
 *                        counts (so you could see it work).
 *                . made case conversions to lower case conditional on
 *                        being upper case, instead of always.  thus using
 *                        lower case in commands works.  (defined 'mklow')
 *                . put in flag to search for in order to get past all this
 *                        garbage at the start of the file.
 *                . forced exit(0) on 'x' from command mode.
 *EOS
 */
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#define mklow(c) (isupper(c) ? tolower(c) : c)
 
#include <sgtty.h>
 
/* log default define */
#ifndef LOGDEFAULT
#define LOGDEFAULT      1
#endif
 
/* Delete logfile define.  Useful on small systems with limited
 * filesystem space and careless users.
 */
#ifndef DELDEFAULT
#define DELDEFAULT      1
#endif
 
#include <signal.h>
 
#define      VERSION    40      /* Version Number */
#define      FALSE      0
#define      TRUE       ~FALSE
 
/*  Compile with "-DPAGEMODE" if Page Mode (TIOCSSCR) is supported on your
 *  UNIX system.  If it is supported, make sure that TIOCSSCR is the
 *  correct name for Page Mode engagement.
 */
 
/*  ASCII Constants  */
#define      SOH        001 
#define      STX        002
#define      ETX        003
#define      EOT        004
#define      ENQ        005
#define      ACK        006
#define      LF         012   /* Unix LF/NL */
#define      CR         015  
#define      NAK        025
#define      SYN        026
#define      CAN        030
#define      ESC        033
#define      CTRLZ      032   /* CP/M EOF for text (usually!) */
 
/*  UMODEM Constants  */
#define      TIMEOUT    -1
#define      ERRORMAX   10    /* maximum errors tolerated */
#define      RETRYMAX   10    /* maximum retries to be made */
#define      BBUFSIZ    128   /* buffer size -- do not change! */
 
/*  [SD] Mode for Created Files  */
#define      CREATMODE  0644  /* mode for created files */
 
/*  ARPA Net Constants  */
/*      The following constants are used to communicate with the ARPA
 *      Net SERVER TELNET and TIP programs.  These constants are defined
 *      as follows:
 *              IAC                     <-- Is A Command; indicates that
 *                                              a command follows
 *              WILL/WONT               <-- Command issued to SERVER TELNET
 *                                              (Host); WILL issues command
 *                                              and WONT issues negative of
 *                                              the command
 *              DO/DONT                 <-- Command issued to TIP; DO issues
 *                                              command and DONT issues
 *                                              negative of the command
 *              TRBIN                   <-- Transmit Binary Command
 *      Examples:
 *              IAC WILL TRBIN  <-- Host is configured to transmit Binary
 *              IAC WONT TRBIN  <-- Host is configured NOT to transmit binary
 *              IAC DO TRBIN    <-- TIP is configured to transmit Binary
 *              IAC DONT TRBIN  <-- TIP is configured NOT to transmit binary
 */
#define      IAC        0377    /* Is A Command */
#define      DO         0375    /* Command to TIP */
#define      DONT       0376    /* Negative of Command to TIP */
#define      WILL       0373    /* Command to SERVER TELNET (Host) */
#define      WONT       0374    /* Negative of Command to SERVER TELNET */
#define      TRBIN      0       /* Transmit Binary Command */
 
struct sgttyb  ttys, ttysnew, ttystemp;    /* for stty terminal mode calls */
struct sgttyb  ttycon, ttycom;
 
struct stat statbuf;    /* for terminal message on/off control */
char *strcat();
FILE *LOGFP, *fopen();
char buff[BBUFSIZ];
int nbchr;  /* number of chars read so far for buffered read */
 
int wason;
int i_port, o_port;
int catch;
 
int pagelen;
char *ttyname();  /* forward declaration for C */
 
char *tty;
char XMITTYPE;
int ARPA, CMNDFLAG, RECVFLAG, SENDFLAG, FTP1, PMSG, DELFLAG, LOGFLAG, MUNGMODE;
int STATDISP, BIT7, BITMASK, NOMUNGMODE;
int delay;
char filename[256];
 
alarmfunc();
 
main(argc, argv)
int argc;
char **argv;
{
        char *getenv();
        char *getlogin();
        char *fname = filename;
        char *logfile;
        int index;
        char flag;
 
        logfile = "umodem.log";  /* Name of LOG File */
        i_port = 0;        /* default input port */
        o_port = 1;        /* default output port */
 
        printf("\nUMODEM Version %d.%d", VERSION/10, VERSION%10);
        printf(" -- UNIX-Based Remote File Transfer Facility\n");
 
        if (argc < 2 || *argv[1] != '-')
        {
                help(FALSE);
                exit(-1);
        }
 
        index = 1;  /* set index for loop */
        delay = 3;  /* assume FTP 3 delay */
        PMSG = FALSE;  /* turn off flags */
        FTP1 = FALSE;  /* assume FTP 3 (CP/M UG XMODEM2) */
        RECVFLAG = FALSE;  /* not receive */
        SENDFLAG = FALSE;  /* not send either */
        CMNDFLAG = FALSE;  /* not command either */
        XMITTYPE = 't';  /* assume text */
        DELFLAG = DELDEFAULT;
        LOGFLAG = LOGDEFAULT;
        if (LOGFLAG) LOGFLAG = TRUE;
           else LOGFLAG = FALSE;  /* set LOGFLAG to REALLY true or false */
        ARPA = FALSE;  /* assume not on ARPA Net */
        MUNGMODE = FALSE; /* protect files from overwriting */
        NOMUNGMODE = TRUE; /* keep user from using MUNGMODE */
        if ((strcmp(getlogin(),"zeus"))==0) NOMUNGMODE = FALSE;
        STATDISP = FALSE;  /* assume not a status display */
        BIT7 = FALSE;  /* assume 8-bit communication */
        while ((flag = argv[1][index++]) != '\0')
            switch (flag) {
                case '1' : FTP1 = TRUE;  /* select FTP 1 */
                           delay = 5;  /* FTP 1 delay constant */
                           printf("\nUMODEM:  TERM II FTP 1 Selected\n");
                           break;
                case '4' : FTP1 = TRUE;  /* select FTP 1 (varient) */
                           BIT7 = TRUE;  /* transfer only 7 bits */
                           delay = 5;  /* FTP 1 delay constant */
                           printf("\nUMODEM:  TERM II FTP 4 Selected\n");
                           break;
                case '7' : BIT7 = TRUE;  /* transfer only 7 bits */
                           break;
                case 'a' : ARPA = TRUE;  /* set ARPA Net */
                           break;
                case 'c' : CMNDFLAG = TRUE;  /* command mode */
                           break;
                case 'd' : DELFLAG = !DELDEFAULT;  /* delete log file ? */
                           break;
                case 'l' : LOGFLAG = !LOGDEFAULT;  /* turn off log ? */
                           break;
                case 'm' : 
                           if(NOMUNGMODE) break;
                           MUNGMODE = TRUE; /* allow overwriting of files */
                           break;
                case 'o' : if ((i_port = open(getenv("MODEM"), 2)) > 0) 
                                o_port = i_port;
                           else
                                i_port = 0;
                           if (i_port) {
                               printf("\nCommunications to %s\n", 
                                   ttyname(i_port));
                               CMNDFLAG = TRUE; /* turn on command mode */
                               printf("Entering local mode\n");
                           };
				break;

                case 'p' : PMSG = TRUE;  /* print all messages */
                           break;
                case 'r' : RECVFLAG = TRUE;  /* receive file */
                           XMITTYPE = gettype(argv[1][index++]);  /* get t/b */
                           break;
                case 's' : SENDFLAG = TRUE;  /* send file */
                           XMITTYPE = gettype(argv[1][index++]);
                           break;
                case 'y' : STATDISP = TRUE;  /* display file status */
                           break;
                default  : error("Invalid Flag", FALSE);
                }
 
        if (BIT7 && (XMITTYPE == 'b'))
        {  printf("\nUMODEM:  Fatal Error -- Both 7-Bit Transfer and ");
           printf("Binary Transfer Selected");
           exit(-1);  /* error exit to UNIX */
        }
 
        if (BIT7)  /* set MASK value */
           BITMASK = 0177;  /* 7 significant bits */
        else
           BITMASK = 0377;  /* 8 significant bits */
 
        if (PMSG)
           { printf("\nSupported File Transfer Protocols:");
             printf("\n\tTERM II FTP 1");
             printf("\n\tCP/M UG XMODEM2 (TERM II FTP 3)");
             printf("\n\tTERM II FTP 4");
             printf("\n\n");
           }
 
        if (CMNDFLAG) LOGFLAG = TRUE;  /* if command mode, always log */
        if (LOGFLAG)
           { 
             if ((fname = getenv("HOME")) == 0) /* Get HOME variable */
                error("Can't get Environment!", FALSE);
             fname = strcat(fname, "/");
             fname = strcat(fname, logfile);
             if (!DELFLAG)
                LOGFP = fopen(fname, "a");  /* append to LOG file */
             else
                LOGFP = fopen(fname, "w");  /* new LOG file */
             if (!LOGFP)
                error("Can't Open Log File", FALSE);
             fprintf(LOGFP,"\n\n++++++++\n");
             fprintf(LOGFP,"\nUMODEM Version %d.%d\n", VERSION/10, VERSION%10);
             printf("\nUMODEM:  LOG File '%s' is Open\n", fname);
           }
 
        if (STATDISP) {
                yfile(argv[2]);  /* status of a file */
                exit(0);  /* exit to UNIX */
                }
 
        if (RECVFLAG && SENDFLAG)
                error("Both Send and Receive Functions Specified", FALSE);
        if (!RECVFLAG && !SENDFLAG && !CMNDFLAG)
                error("Send, Receive, or Command Functions NOT Given", FALSE);
 
        if (RECVFLAG)
        {  if(open(argv[2], 0) != -1)  /* possible abort if file exists */
           {    printf("\nUMODEM:  Warning -- Target File Exists\n");
                if( MUNGMODE == FALSE )
                        error("Fatal - Can't overwrite file\n",FALSE);
                printf("UMODEM:  Overwriting Target File\n");
           }
           rfile(argv[2]);  /* receive file */
        }
        else {
                if (SENDFLAG) sfile(argv[2]);  /* send file */
                else command();  /* command mode */
                }
        if (CMNDFLAG) LOGFLAG = TRUE;  /* for closing log file */
        if (LOGFLAG) fclose(LOGFP);
        exit(0);
}
 
/*  Major Command Mode  */
command()
{
        char cmd, *fname, *infile();
        int command(), fpid, c;
 
        if (i_port) /* if local, then i_port is non-zero */
                signal(SIGINT, command);
        printf("\nUMODEM Command Mode -- Type ? for Help");
        do {
           printf("\n");
           printf(FTP1 ? "1" : "3");  /* FTP 1 or 3? */
           printf(BIT7 ? "7" : " ");  /* BIT 7 or not? */
           printf(ARPA ? "A" : " ");  /* ARPA Net or not? */
           printf(LOGFLAG ? "L" : " ");  /* Log Entries or not? */
           printf(MUNGMODE ? "M" : " ");  /* Mung Files or not? */
           printf(" UMODEM> ");
           scanf("%s", filename);
           switch (cmd = mklow(filename[0])) {
                case 't' : if (!i_port) { /* only do it if local */
                                printf("Sorry, must be in local mode.\n");
                                break;
                           }
                           setmodes(1);
                           /* setup raw and noecho for console */
                           gtty(0, &ttycon);
                           gtty(0, &ttycom);
                           ttycom.sg_flags |= RAW;
                           ttycom.sg_flags &= ~ECHO;
                           stty(0, &ttycom);
                           printf("\n\nEntering terminal mode\r\n\n");
                           if (!(fpid = fork())) { 
                                /* child to read from modem */
                                signal(SIGINT, SIG_DFL);
                                do {
                                    if ((c=readbyte(0)) > 0)
                                        putchar(c);
                                } while(1);
                           } else {
                                /* parent, reading from console */
                                while ((c=getchar()) != 'Q'-0x40)
                                    sendbyte(c);
                                kill(fpid, 2);
                           }
                           /* put it back the way you got it */
			   catch=chmod(tty,00666);
                           stty(0, &ttycon);
                           printf("\n\nExiting terminal mode\n\n");
                           break;
                case '1' : FTP1 = TRUE;  /* select FTP 1 */
                           delay = 5;  /* FTP 1 delay constant */
                           printf("\nTERM FTP 1 Selected");
                           break;
                case '3' : FTP1 = FALSE; /* select FTP 3 */
                           delay = 3;  /* FTP 3 delay constant */
                           printf("\nTERM FTP 3 Selected");
                           break;
                case '7' : BIT7 = ~BIT7;  /* toggle 7 bit transfer */
                           printf("\n7-Bit Transfer %s Selected",
                                BIT7 ? "" : "NOT");
                           break;
                case 'a' : ARPA = ~ARPA;  /* toggle ARPA Net */
                           printf("\nDDN Interface %s Selected",
                                ARPA ? "" : "NOT");
                           break;
                case 'l' : LOGFLAG = ~LOGFLAG;  /* toggle log flag */
                           printf("\nEntry Logging %s Enabled",
                                LOGFLAG ? "" : "NOT");
                           break;
                case 'm' : 
                           if(NOMUNGMODE)
                           {
                           printf("\nOverwrite Permission Denied");
                           break;
                           }
                           MUNGMODE = ~MUNGMODE; /* toggle file overwrite */
                           printf("\nFile Overwriting %s Enabled",
                                MUNGMODE ? "" : "NOT");
                           break;
                case 'r' : RECVFLAG = TRUE;  /* receive file */
                           XMITTYPE = mklow(filename[1]);
                           fname = infile();  /* get file name */
                           if (*fname != '\0') 
                              {  if(open(fname, 0) != -1)
                                 {
                          printf("\nUMODEM:  Warning -- Target File Exists\n");
                                    if( MUNGMODE == FALSE )
                                      {
                                      printf("Fatal - Can't overwrite file\n");
                                      break;
                                      }
                                  printf("UMODEM:  Overwriting Target File\n");
                                  }
                                 rfile(fname);
                              }
                           break;
                case 's' : SENDFLAG = TRUE;  /* send file */
                           XMITTYPE = mklow(filename[1]);
                           fname = infile();  /* get file name */
                           if (*fname != '\0') sfile(fname);
                           break;
                case 'x' : exit(0); /* hard core exit */
                case 'y' : fname = infile();  /* get file name */
                           if (*fname != '\0') yfile(fname);
                           break;
                default  : help(TRUE);
           }
        } while (cmd != 'x');
}
 
/*  Get file name from user  */
char *infile()
{
        char *startptr = filename;
 
        scanf("%s",startptr);
        if (*startptr == '.') *startptr = '\0';  /* set NULL */
        return(startptr);
}
 
/*  Print Help Message  */
help(caller)
int caller;
{
        if (!caller) { printf("\nUsage:  \n\tumodem ");
                printf("-[o!c!rb!rt!sb!st][options]");
                printf(" filename\n");
                }
        else {
                printf("\nUsage: r or s or option");
                }
        printf("\nMajor Commands --");
        if (!caller) printf("\n\tc  <-- Enter Command Mode");
        printf("\n\to  <-- Operate in local mode");
        printf("\n\trb <-- Receive Binary");
        printf("\n\trt <-- Receive Text");
        printf("\n\tsb <-- Send Binary");
        printf("\n\tst <-- Send Text");
        printf("\nOptions --");
        printf("\n\t1  <-- (one) Employ TERM II FTP 1");
        if (caller) printf("\n\t3  <-- Enable TERM FTP 3 (CP/M UG)");
        if (!caller) printf("\n\t4  <-- Enable TERM FTP 4");
        printf("\n\t7  <-- Enable 7-bit transfer mask");
        printf("\n\ta  <-- Turn ON ARPA Net Flag");
        if (!caller)
#if DELDEFAULT == 1
        printf("\n\td  <-- Do not delete umodem.log file before starting");
#else
        printf("\n\td  <-- Delete umodem.log file before starting");
#endif
 
        if (!caller)
#if LOGDEFAULT == 1
        printf("\n\tl  <-- (ell) Turn OFF LOG File Entries");
#else
        printf("\n\tl  <-- (ell) Turn ON LOG File Entries");
#endif
        else printf("\n\tl  <-- Toggle LOG File Entries");
 
        printf("\n\tm  <-- Allow file overwiting on receive");
        if (!caller) printf("\n\tp  <-- Turn ON Parameter Display");
        if (caller) printf("\n\tx  <-- Exit");
        printf("\n\ty  <-- Display file status (size) information only");
        printf("\n");
}
 
gettype(ichar)
char ichar;
{
        if (ichar == 't') return(ichar);
        if (ichar == 'b') return(ichar);
        error("Invalid Send/Receive Parameter - not t or b", FALSE);
        return;
}
 
/* set tty modes for UMODEM transfers */
setmodes()
{
 
 
        /* changed to get tty params from working console, so that
           other mode changes would not interfere. */
        if (gtty(0, &ttys) < 0)  /* get current tty params */
                error("Can't get TTY Parameters", TRUE);
        tty = ttyname(i_port);  /* identify current tty */
        /* transfer current modes to new structure */
        if (!i_port) {
                ttysnew.sg_ispeed = ttys.sg_ispeed;     /* copy input speed */
                ttysnew.sg_ospeed = ttys.sg_ospeed;     /* copy output speed */
        } else {
                ttysnew.sg_ispeed = B1200;
                ttysnew.sg_ospeed = B1200;
        }
        ttysnew.sg_erase  = ttys.sg_erase;      /* copy erase flags */
        ttysnew.sg_flags  = ttys.sg_flags;      /* copy flags */
        ttysnew.sg_kill   = ttys.sg_kill;       /* copy std terminal flags */
        
 
        ttysnew.sg_flags |= RAW;    /* set for RAW Mode */
                        /* This ORs in the RAW mode value, thereby
                           setting RAW mode and leaving the other
                           mode settings unchanged */
        ttysnew.sg_flags &= ~ECHO;  /* set for no echoing */
                        /* This ANDs in the complement of the ECHO
                           setting (for NO echo), thereby leaving all
                           current parameters unchanged and turning
                           OFF ECHO only */
        ttysnew.sg_flags &= ~XTABS;  /* set for no tab expansion */
        ttysnew.sg_flags &= ~LCASE;  /* set for no upper-to-lower case xlate */
        ttysnew.sg_flags |= ANYP;  /* set for ANY Parity */
        ttysnew.sg_flags &= ~NL3;  /* turn off ALL delays - new line */
        ttysnew.sg_flags &= ~TAB0; /* turn off tab delays */
        ttysnew.sg_flags &= ~TAB1;
        ttysnew.sg_flags &= ~CR3;  /* turn off CR delays */
        ttysnew.sg_flags &= ~FF1;  /* turn off FF delays */
        ttysnew.sg_flags &= ~BS1;  /* turn off BS delays */
 
        if (stty(i_port, &ttysnew) < 0)  /* set new params */
                error("Can't set new TTY Parameters", TRUE);
 
        if (stat(tty, &statbuf) < 0)  /* get tty status */ 
                error("Can't get your TTY Status", TRUE);
        if (statbuf.st_mode & 022)      /* Need to turn messages off */
                if (chmod(tty, statbuf.st_mode & ~022) < 0)
                        error("Can't change  TTY mode", TRUE);
                else wason = TRUE;
        else wason = FALSE;
 
        if (PMSG)
                { printf("\nUMODEM:  TTY Device Parameters Altered");
                  ttyparams();  /* print tty params */
                }
 
        if (ARPA)  /* set 8-bit on ARPA Net */
                setarpa();
 
        return;
}
 
/*  set ARPA Net for 8-bit transfers  */
setarpa()
{
        sendbyte(IAC);  /* Is A Command */
        sendbyte(WILL); /* Command to SERVER TELNET (Host) */
        sendbyte(TRBIN);        /* Command is:  Transmit Binary */
 
        sendbyte(IAC);  /* Is A Command */
        sendbyte(DO);   /* Command to TIP */
        sendbyte(TRBIN);        /* Command is:  Transmit Binary */
 
        sleep(3);  /* wait for TIP to configure */
 
        return;
}
 
/* restore normal tty modes */
restoremodes(errcall)
int errcall;
{
        if (ARPA)  /* if ARPA Net, reconfigure */
                resetarpa();
 
        if (wason)
                if (chmod(tty, statbuf.st_mode | 022) < 0)
                        error("Can't change TTY mode", FALSE);
 
        if (stty(i_port, &ttys) < 0)  /* restore original tty modes */
                { if (!errcall)
                   error("RESET - Can't restore normal TTY Params", FALSE);
                else
                   { printf("UMODEM:  ");
                     printf("RESET - Can't restore normal TTY Params\n");
                   }
                }
 
        if (PMSG)
                { printf("\nUMODEM:  TTY Device Parameters Restored");
                  ttyparams();  /* print tty params */
                }
 
        return;
}
 
/* reset the ARPA Net */
resetarpa()
{
        sendbyte(IAC);  /* Is A Command */
        sendbyte(WONT); /* Negative Command to SERVER TELNET (Host) */
        sendbyte(TRBIN);        /* Command is:  Don't Transmit Binary */
 
        sendbyte(IAC);  /* Is A Command */
        sendbyte(DONT); /* Negative Command to TIP */
        sendbyte(TRBIN);        /* Command is:  Don't Transmit Binary */
 
        return;
}
 
/* print error message and exit; if mode == TRUE, restore normal tty modes */
error(msg, mode)
char *msg;
int mode;
{
        if (mode)
                restoremodes(TRUE);  /* put back normal tty modes */
        printf("UMODEM:  %s\n", msg);
        if (LOGFLAG & (int)LOGFP)
        {   fprintf(LOGFP, "UMODEM Fatal Error:  %s\n", msg);
            fclose(LOGFP);
        }
        exit(-1);
}
 
/**  print status (size) of a file  **/
yfile(name)
char *name;
{
        printf("\nUMODEM File Status Display for %s\n", name);
 
        if (open(name,0) < 0) {
                printf("File %s does not exist\n", name);
                return;
                }
 
        prfilestat(name);  /* print status */
        printf("\n");
}
 
getbyte(fildes, ch)                             /* Buffered disk read */
int fildes;
char *ch;
/*
 *
 *      Get a byte from the specified file.  Buffer the read so we don't
 *      have to use a system call for each character.
 *
 */
 
{
        static char buf[BUFSIZ];        /* Remember buffer */
        static char *bufp = buf;        /* Remember where we are in buffer */
        
        if (nbchr == 0)                 /* Buffer exausted; read some more */
        {
                if ((nbchr = read(fildes, buf, BUFSIZ)) < 0)
                        error("File Read Error", TRUE);
                bufp = buf;             /* Set pointer to start of array */
        }
        if (--nbchr >= 0)
        {
                *ch = *bufp++;
                return(0);
        }
        else
                return(EOF);
}
 
/**  receive a file  **/
rfile(name)
char *name;
{
        char mode;
        int fd, j, firstchar, sectnum, sectcurr, tmode;
        int sectcomp, errors, errorflag, recfin;
        register int bufctr, checksum;
        register int c;
        int errorchar, fatalerror, startstx, inchecksum, endetx, endenq;
        long recvsectcnt;
 
        mode = XMITTYPE;  /* set t/b mode */
        if ((fd = creat(name, CREATMODE)) < 0)
                error("Can't create file for receive", FALSE);
        setmodes();  /* setup tty modes for xfer */
        printf("\r\nUMODEM:  File Name: %s", name);
        if (LOGFLAG)
        {    fprintf(LOGFP, "\n----\nUMODEM Receive Function\n");
             fprintf(LOGFP, "File Name: %s\n", name);
             if (FTP1)
                if (!BIT7)
                 fprintf(LOGFP, "TERM II File Transfer Protocol 1 Selected\n");
                else
                 fprintf(LOGFP, "TERM II File Transfer Protocol 4 Selected\n");
             else
                fprintf(LOGFP,
                  "TERM II File Transfer Protocol 3 (CP/M UG) Selected\n");
             if (BIT7)
                fprintf(LOGFP, "7-Bit Transmission Enabled\n");
             else
                fprintf(LOGFP, "8-Bit Transmission Enabled\n");
        }
        printf("\r\nUMODEM:  ");
        if (BIT7)
                printf("7-Bit");
        else
                printf("8-Bit");
        printf(" Transmission Enabled");
        printf("\r\nUMODEM:  Ready to RECEIVE File\r\n");
 
        recfin = FALSE;
        sectnum = errors = 0;
        fatalerror = FALSE;  /* NO fatal errors */
        recvsectcnt = 0;  /* number of received sectors */
 
        if (mode == 't')
                tmode = TRUE;
        else
                tmode = FALSE;
 
        if (tmode && i_port)
                printf("\n\rText mode conversions activated (cp/m files)\n\r");
 
        if (i_port)
                printf("\nSync...\n\n");
        if (FTP1)
        {
          while (readbyte(4) != SYN);
          sendbyte(ACK);  /* FTP 1 Sync */
        }
        else sendbyte(NAK);  /* FTP 3 Sync */
 
        do
        {   errorflag = FALSE;
            do {
                  firstchar = readbyte(6);
            } while ((firstchar != SOH) && (firstchar != EOT) && (firstchar 
                     != TIMEOUT));
            if (firstchar == TIMEOUT)
            {  if (LOGFLAG)
                fprintf(LOGFP, "Timeout on Sector %d\n", sectnum);
               errorflag = TRUE;
            }
 
            if (firstchar == SOH)
            {  if (FTP1) readbyte(5);  /* discard leading zero */
               sectcurr = readbyte(delay);
               sectcomp = readbyte(delay);
               if (FTP1) startstx = readbyte(delay);  /* get leading STX */
               if ((sectcurr + sectcomp) == BITMASK)
               {  if (sectcurr == ((sectnum+1)&BITMASK))
                  {  checksum = 0;
                     for (j = bufctr = 0; j < BBUFSIZ; j++)
                     {  buff[bufctr] = c = readbyte(delay);
                        checksum = ((checksum+c)&BITMASK);
                        if (!tmode)  /* binary mode */
                        {  bufctr++;
                           continue;
                        }
                        if (c == CR)
                           continue;  /* skip CR's */
                        if (c == CTRLZ)  /* skip CP/M EOF char */
                        {  recfin = TRUE;  /* flag EOF */
                           continue;
                        }
                        if (!recfin)
                           bufctr++;
                     }
                     if (FTP1) endetx = readbyte(delay);  /* get ending ETX */
                     inchecksum = readbyte(delay);  /* get checksum */
                     if (FTP1) endenq = readbyte(delay); /* get ENQ */
                     if (checksum == inchecksum)  /* good checksum */
                     {  errors = 0;
                        recvsectcnt++;
                        sectnum = sectcurr;  /* update sector counter */
                        if (write(fd, buff, bufctr) < 0)
                           error("File Write Error", TRUE);
                        else
                        {  
                           if (i_port)
                                printf("Received Sector %d\r", sectcurr);
                            if (FTP1) sendbyte(ESC);  /* FTP 1 requires <ESC> */
                           sendbyte(ACK);
                        }
                     }
                     else
                     {  if (LOGFLAG)
                                fprintf(LOGFP, "Checksum Error on Sector %d\n",
                                sectnum);
                        errorflag = TRUE;
                     }
                  }
                  else
                  { if (sectcurr == sectnum)
                    {  while(readbyte(3) != TIMEOUT);
                       if (FTP1) sendbyte(ESC);  /* FTP 1 requires <ESC> */
                       sendbyte(ACK);
                    }
                    else
                    {  if (LOGFLAG)
                        { fprintf(LOGFP, "Phase Error - Received Sector is ");
                          fprintf(LOGFP, "%d while Expected Sector is %d\n",
                           sectcurr, ((sectnum+1)&BITMASK));
                        }
                        errorflag = TRUE;
                        fatalerror = TRUE;
                        if (FTP1) sendbyte(ESC);  /* FTP 1 requires <ESC> */
                        sendbyte(CAN);
                    }
                  }
           }
           else
           {  if (LOGFLAG)
                fprintf(LOGFP, "Header Sector Number Error on Sector %d\n",
                   sectnum);
               errorflag = TRUE;
           }
        }
        if (FTP1 && !errorflag)
        {  if (startstx != STX)
           {  errorflag = TRUE;  /* FTP 1 STX missing */
              errorchar = STX;
           }
           if (endetx != ETX)
           {  errorflag = TRUE;  /* FTP 1 ETX missing */
              errorchar = ETX;
           }
           if (endenq != ENQ)
           {  errorflag = TRUE;  /* FTP 1 ENQ missing */
              errorchar = ENQ;
           }
           if (errorflag && LOGFLAG)
           {  fprintf(LOGFP, "Invalid Packet-Control Character:  ");
              switch (errorchar) {
                case STX : fprintf(LOGFP, "STX"); break;
                case ETX : fprintf(LOGFP, "ETX"); break;
                case ENQ : fprintf(LOGFP, "ENQ"); break;
                default  : fprintf(LOGFP, "Error"); break;
              }
              fprintf(LOGFP, "\n");
           }
        }
        if (errorflag == TRUE)
        {  errors++;
           while (readbyte(3) != TIMEOUT);
           sendbyte(NAK);
        }
  }
  while ((firstchar != EOT) && (errors != ERRORMAX) && !fatalerror);
  if ((firstchar == EOT) && (errors < ERRORMAX))
  {  if (!FTP1) sendbyte(ACK);
     close(fd);
     restoremodes(FALSE);  /* restore normal tty modes */
     if (FTP1)
        while (readbyte(3) != TIMEOUT);  /* flush EOT's */
     sleep(3);  /* give other side time to return to terminal mode */
     if (LOGFLAG)
     {  fprintf(LOGFP, "\nReceive Complete\n");
        fprintf(LOGFP,"Number of Received CP/M Records is %ld\n", recvsectcnt);
     }
     printf("\n");
  }
  else
  {  if (LOGFLAG && FTP1 && fatalerror) fprintf(LOGFP,
        "Synchronization Error");
     error("TIMEOUT -- Too Many Errors", TRUE);
  }
}
 
/**  send a file  **/
sfile(name)
char *name;
{
        char mode, c;
        int fd, attempts, nlflag, sendfin, tmode, sendresp;
        register int bufctr, checksum, sectnum;
 
        nbchr = 0;  /* clear buffered read char count */
        mode = XMITTYPE;  /* set t/b mode */
        if ((fd = open(name, 0)) < 0)
        {  if (LOGFLAG) fprintf(LOGFP, "Can't Open File\n");
           error("Can't open file for send", FALSE);
        }
        setmodes();  /* setup tty modes for xfer */     
        printf("\r\nUMODEM:  File Name: %s", name);
        if (LOGFLAG)
        {   fprintf(LOGFP, "\n----\nUMODEM Send Function\n");
            fprintf(LOGFP, "File Name: %s\n", name);
        }
        printf("\r\n"); prfilestat(name);  /* print file size statistics */
        if (LOGFLAG)
        {  if (FTP1)
              if (!BIT7)
                fprintf(LOGFP, "TERM II File Transfer Protocol 1 Selected\n");
              else
                fprintf(LOGFP, "TERM II File Transfer Protocol 4 Selected\n");
           else
                fprintf(LOGFP,
                   "TERM II File Transfer Protocol 3 (CP/M UG) Selected\n");
           if (BIT7)
                fprintf(LOGFP, "7-Bit Transmission Enabled\n");
           else
                fprintf(LOGFP, "8-Bit Transmission Enabled\n");
        }
        printf("\r\nUMODEM:  ");
        if (BIT7)
                printf("7-Bit");
        else
                printf("8-Bit");
        printf(" Transmission Enabled");
        printf("\r\nUMODEM:  Ready to SEND File\r\n");
 
        if (mode == 't')
           tmode = TRUE;
        else
           tmode = FALSE;
 
        if (tmode && i_port)
                printf("\n\rText mode conversions activated (cp/m files)\n\r");
        sendfin = nlflag = FALSE;
        attempts = 0;
 
        if (i_port)
                printf("\nSync...\n\n");
        if (FTP1)
        {  sendbyte(SYN);  /* FTP 1 Synchronize with Receiver */
           while (readbyte(5) != ACK)
           {  if(++attempts > RETRYMAX*6) error("Remote System Not Responding",
                TRUE);
              sendbyte(SYN);
           }
        }
        else
        {  while (readbyte(30) != NAK)  /* FTP 3 Synchronize with Receiver */
           if (++attempts > RETRYMAX) error("Remote System Not Responding",
                TRUE);
        }
 
        sectnum = 1;  /* first sector number */
        attempts = 0;
 
        do 
        {   for (bufctr=0; bufctr < BBUFSIZ;)
            {   if (nlflag)
                {  buff[bufctr++] = LF;  /* leftover newline */
                   nlflag = FALSE;
                }
                if (getbyte(fd, &c) == EOF)
                {  sendfin = TRUE;  /* this is the last sector */
                   if (!bufctr)  /* if EOF on sector boundary */
                      break;  /* avoid sending extra sector */
                   if (tmode)
                      buff[bufctr++] = CTRLZ;  /* Control-Z for CP/M EOF */
                   else
                      bufctr++;
                   continue;
                }
                if (tmode && c == LF)  /* text mode & Unix newline? */
                {  if (c == LF)  /* Unix newline? */
                   {  buff[bufctr++] = CR;  /* insert carriage return */
                      if (bufctr < BBUFSIZ)
                         buff[bufctr++] = LF;  /* insert Unix newline */
                      else
                         nlflag = TRUE;  /* insert newline on next sector */
                   }
                   continue;
                }       
                buff[bufctr++] = c;  /* copy the char without change */
            }
            attempts = 0;
        
            if (!bufctr)  /* if EOF on sector boundary */
               break;  /* avoid sending empty sector */
 
            do
            {   
                if (i_port)
                        printf("Sending Sector %d\r", sectnum);
                sendbyte(SOH);  /* send start of packet header */
                if (FTP1) sendbyte(0);  /* FTP 1 Type 0 Packet */
                sendbyte(sectnum);  /* send current sector number */
                sendbyte(-sectnum-1);  /* and its complement */
                if (FTP1) sendbyte(STX);  /* send STX */
                checksum = 0;  /* init checksum */
                for (bufctr=0; bufctr < BBUFSIZ; bufctr++)
                {  sendbyte(buff[bufctr]);  /* send the byte */
                   if (ARPA && (buff[bufctr]==0xff))  /* ARPA Net FFH esc */
                        sendbyte(buff[bufctr]);  /* send 2 FFH's for one */
                   checksum = ((checksum+buff[bufctr])&BITMASK);
                }
/*              while (readbyte(3) != TIMEOUT);   flush chars from line */
                if (FTP1) sendbyte(ETX);  /* send ETX */
                sendbyte(checksum);  /* send the checksum */
                if (FTP1) sendbyte(ENQ);  /* send ENQ */
                attempts++;
                if (FTP1)
                {  sendresp = NAK;  /* prepare for NAK */
                   if (readbyte(10) == ESC) sendresp = readbyte(10);
                }
                else
                   sendresp = readbyte(10);  /* get response */
                if ((sendresp != ACK) && LOGFLAG)
                   { fprintf(LOGFP, "Non-ACK Received on Sector %d\n",
                      sectnum);
                     if (sendresp == TIMEOUT)
                        fprintf(LOGFP, "This Non-ACK was a TIMEOUT\n");
                   }
            }   while((sendresp != ACK) && (attempts != RETRYMAX));
            sectnum++;  /* increment to next sector number */
    }  while (!sendfin && (attempts != RETRYMAX));
 
    if (attempts == RETRYMAX)
        error("Remote System Not Responding", TRUE);
 
    attempts = 0;
    if (FTP1)
        while (attempts++ < 10) sendbyte(EOT);
    else
    {   sendbyte(EOT);  /* send 1st EOT */
        while ((readbyte(15) != ACK) && (attempts++ < RETRYMAX))
           sendbyte(EOT);
        if (attempts >= RETRYMAX)
           error("Remote System Not Responding on Completion", TRUE);
    }
 
    close(fd);
    restoremodes(FALSE);  
    sleep(5);  /* give other side time to return to terminal mode */
    if (LOGFLAG)
    {  fprintf(LOGFP, "\nSend Complete\n");
    }
    printf("\n");
 
}
 
/*  print file size status information  */
prfilestat(name)
char *name;
{
        struct stat filestatbuf; /* file status info */
 
        stat(name, &filestatbuf);  /* get file status bytes */
        printf("  Estimated File Size %ldK, %ld Records, %ld Bytes",
          (filestatbuf.st_size/1024)+1, (filestatbuf.st_size/128)+1,
          filestatbuf.st_size);
        if (LOGFLAG)
          fprintf(LOGFP,"Estimated File Size %ldK, %ld Records, %ld Bytes\n",
          (filestatbuf.st_size/1024)+1, (filestatbuf.st_size/128)+1,
          filestatbuf.st_size);
        return;
}
 
/* get a byte from data stream -- timeout if "seconds" elapses */
readbyte(seconds)
unsigned seconds;
{
        char c;
        
        signal(SIGALRM,alarmfunc);  /* catch alarms */  
        alarm(seconds);  /* set the alarm clock */
        if (read(i_port, &c, 1) < 0)  /* get a char; error means we timed out */
          {
             return(TIMEOUT);
          }
        alarm(0);  /* turn off the alarm */
        return((c&BITMASK));  /* return the char */
}
 
/* send a byte to data stream */
sendbyte(data)
char data;
{
        char dataout;
        dataout = (data&BITMASK);  /* mask for 7 or 8 bits */
        write(o_port, &dataout, 1);  /* write the byte */
        return;
}
 
/* function for alarm clock timeouts */
alarmfunc()
{
        return;  /* this is basically a dummy function to force error */
                 /* status return on the "read" call in "readbyte"    */
}
 
/* print data on TTY setting */
ttyparams()
{
        
        gtty(i_port, &ttystemp);  /* get current tty params */
 
        tty = ttyname(i_port);  /* get name of tty */
        stat(tty, &statbuf);  /* get more tty params */
 
        printf("\r\n\nTTY Device Parameter Display");
          printf("\r\n\tTTY Device Name is %s\r\n\n", tty);
          printf("\tAny Parity Allowed "); pryn(ANYP);
          printf("\tEven Parity Allowed"); pryn(EVENP);
          printf("\tOdd Parity Allowed "); pryn(ODDP);
          printf("\tEcho Enabled       "); pryn(ECHO);
          printf("\tLower Case Map     "); pryn(LCASE);
          printf("\tTabs Expanded      "); pryn(XTABS);
          printf("\tCR Mode Enabled    "); pryn(CRMOD);
          printf("\tRAW Mode Enabled   "); pryn(RAW);
 
          printf("\tTTY Input Rate     :   ");
            prbaud(ttystemp.sg_ispeed);
          printf("\tTTY Output Rate    :   ");
            prbaud(ttystemp.sg_ospeed);  /* print output baud rate */
}
 
pryn(iflag)
int iflag;
{
 
        if (ttystemp.sg_flags&iflag)
 
           printf(":   Yes\r\n");
        else
           printf(":   No\r\n");
}
 
 
prbaud(speed)
char speed;
{
        switch (speed) {
 
                case B75 : printf("75"); break;
                case B110 : printf("110"); break;
                case B134 : printf("134.5"); break;
                case B150 : printf("150"); break;
                case B200 : printf("200"); break;
                case B300 : printf("300"); break;
                case B600 : printf("600"); break;
                case B1200 : printf("1200"); break;
                case B1800 : printf("1800"); break;
                case B2400 : printf("2400"); break;
                case B4800 : printf("4800"); break;
                case B9600 : printf("9600"); break;
                default : printf("Error");break;

        }
        printf(" Baud\r\n");

}
 
                                                                                                                                