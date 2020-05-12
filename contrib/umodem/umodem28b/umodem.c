/*
 *  UMODEM -- Implements the "CP/M User's Group XMODEM" protocol and 
 *            the TERM II File Transfer Protocol (FTP) Number 1 for
 *            packetized file up/downloading.    
 *
 *    Note: UNIX System-Dependent values are indicated by the string [SD]
 *          in a comment field one the same line as the values.
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
 *                      "cc -7 umodem.c -o umodem -DVER7" for Version 7
 *                      "cc -7 umodem.c -o umodem -DJHU" for JHU
 *              . added 'y' file status display option; this option gives
 *                the user an estimate of the size of the target file to
 *                send from the UNIX system in terms of CP/M records (128
 *                bytes) and Kbytes (1024 byte units)
 *              . added '7' option which modifies the transmission protocols
 *                for 7 significant bits rather than 8; modifies both FTP 1
 *                and FTP 3
 *         -- Version 2.8 mods by Walter Reiher, 3/12/83 (Harvard Chem. Dept.)
 *              . Added definitions needed to run UMODEM under VMS.
 *                Support subroutines are in another file, VMODEM.C.  Both
 *                this file and VMODEM.C #include a third file, VMODEM.H,
 *                when being compiled under VMS.  All of the additions here
 *                are imbedded in "#ifdef vms" statements EXCEPT the addition
 *                of upper case synonyms for some of the command-line
 *                switches, since VMS changes the command line to upper case.
 *                To compile under VMS, no switches to the compiler are needed.
 *              . Fixed bug in the handling of long files (i.e., >255 sectors).
 *                This file now falls into this category.
 *              . Included code to cause the program to exit when in send
 *                mode and it receives a CAN instead of an ACK from the
 *                receiver.  This program, in receive mode, sends a CAN
 *                when it encounters a fatal error.  Comment out the
 *                "#define CAN_BOMB" line to disable this.
 *              . Small patches to get it to work under 4.1 BSD VM/UNIX,
 *                as running in the Harvard Science Center, where there is
 *                tighter checking of addresses vs. ints.
 *                Use -DVER7 for VM/UNIX.
 *              . Make text mode transfers accept lone CRs from CP/M (as
 *                opposed to CR/LF pairs) to UNIX.  This permits overstriking.
 *              . Added report of estimated file transfer time.
 *              . Removed some more [SD]'s which weren't marked that way;
 *                removed some unnecessary subroutine calls by pushing them
 *                into "JHU" or "VER7", as appropriate.
 *
 */

#include stdio

#ifdef vms
#include ssdef
#include tt2def
#include ttdef
#include "vmodem.h"

/*  #includes for BOTH JHU and VER7  */
#else
#include <signal.h>
#include <sys/types.h>                  /*  Ordering for these two  */
#include <sys/stat.h>                   /*  lines is important      */

/*  JHU UNIX tty parameter file  */
#ifdef JHU
#include <stty.h>
#endif

/*  Version 7 UNIX tty parameter file  */
#ifdef VER7
#include <sgtty.h>
#endif

#endif

#define VERSION         28      /* Version Number */
#define TRUE            1               
#define FALSE           0
#define SOH             001
#define STX             002
#define ETX             003
#define EOT             004
#define ENQ             005
#define ACK             006
#define LF              012     /* Unix LF/NL */
#define CR              015
#define NAK             025
#define SYN             026
#define CAN             030
#define ESC             033
#define CTRLZ           032     /* CP/M EOF for text (usually!) */
#define TIMEOUT         -1
#define ERRORMAX        10      /* maximum errors tolerated */
#define RETRYMAX        10      /* maximum retries to be made */
#define BBUFSIZ         128     /* buffer size -- do not change! */
#define CREATMODE       0644    /* mode for created files */
#define CAN_BOMB                /* Enable bomb out on receipt of CAN */

/*  ARPA Net Constants  */
#define      IAC        0377
#define      DO         0375
#define      DONT       0376
#define      WILL       0373
#define      WONT       0374
#define      TRBIN      0

/*  JHU UNIX structures  */
#ifdef JHU
struct sttybuf ttys, ttysnew, ttystemp;    /* for stty terminal mode calls */
struct stat statbuf;            /* for terminal message on/off control */
#endif

/*  Version 7 UNIX structures  */
#ifdef VER7
struct sgttyb  ttys, ttysnew, ttystemp;    /* for stty terminal mode calls */
#endif

/*  VMS structures  */
#ifdef vms
/*
 *      TT_INFO structures are used for passing information about
 *      the terminal.  Used in GTTY and STTY calls.
 */
struct  tt_info ttys, ttysnew, ttystemp;
#endif

FILE    *LOGFP, *fopen();
char    buff[BBUFSIZ + 1];

#ifdef JHU
int wason;
#endif

#ifdef VER7
int pagelen;
#endif

#ifndef vms
char *tty, *ttyname();
#endif

char    XMITTYPE;
int     ARPA, BIT7, BITMASK, DELFLAG, FTP1, LOGFLAG;
int     MUNGMODE, PMSG, RECVFLAG, SENDFLAG, STATDISP;
int     delay;

main(argc, argv)
int argc;
char **argv;
{
        char *logfile;
        int index;
        char flag;

        logfile = "umodem.log";  /* Name of LOG File */

        printf("\nUMODEM Version %d.%d", VERSION/10, VERSION%10);
        printf(" -- UNIX-Based Remote File Transfer Facility\n");

        if (argc < 3 || *argv[1] != '-')
        {  printf("\nUsage:  \n\tumodem ");
#ifndef vms
                printf("-[rb!rt!sb!st][p][l][1][a][m][d][y][7]");
#else
                printf("-[rb!rt!sb!st][l][p][y][1][7]");
#endif
                printf(" filename\n");
           printf("\n");
           printf("\trb <-- Receive Binary\n");
           printf("\trt <-- Receive Text\n");
           printf("\tsb <-- Send    Binary\n");
           printf("\tst <-- Send    Text\n");
           printf("\tl  <-- (ell) Turn OFF LOG File Entries\n");
           printf("\tp  <-- Turn ON Parameter Display\n");
           printf("\ty  <-- Display file status (size) information only\n");
           printf("\t1  <-- (one) Employ TERM II FTP 1\n");
           printf("\t7  <-- Enable 7-bit transfer mask\n");
#ifndef vms
           printf("\ta  <-- Turn ON ARPA Net Flag\n");
           printf("\td  <-- Delete umodem.log File before starting\n");
           printf("\tm  <-- Allow file overwiting on receive\n");
#endif
           printf("\n");

#ifdef vms
                exit(SS$_NORMAL);
#else
                exit(-1);
#endif
        }

/*
 *      Initializations
 */
        index           = 1;            /* set index for loop */
        delay           = 3;            /* assume FTP 3 delay */
        RECVFLAG        = FALSE;        /* not receive */
        SENDFLAG        = FALSE;        /* not send either */
        XMITTYPE        = 't';          /* assume text */
        FTP1            = FALSE;        /* assume FTP 3 (CP/M UG XMODEM2) */
        LOGFLAG         = TRUE;         /* assume log messages */
        PMSG            = FALSE;        /* turn off flags */
        STATDISP        = FALSE;        /* assume not a status display */
        BIT7            = FALSE;        /* assume 8-bit communication */
        ARPA            = FALSE;        /* assume not on ARPA Net */
        DELFLAG         = FALSE;        /* do NOT delete log file
                                         *   before starting */
        MUNGMODE        = FALSE;        /* protect files from overwriting */

        while ((flag = argv[1][index++]) != '\0')
            switch (flag) {
                case '1' : FTP1 = TRUE;  /* select FTP 1 */
                           delay = 5;  /* FTP 1 delay constant */
                           printf("\nUMODEM:  TERM II FTP 1 Selected\n");
                           break;
                case '7' : BIT7 = TRUE;  /* transfer only 7 bits */
                           break;
                case 'a' : ARPA = TRUE;  /* set ARPA Net */
                           break;
                case 'd' : DELFLAG = TRUE;  /* delete log file first */
                           break;
                case 'L':
                case 'l' : LOGFLAG = FALSE;  /* turn off log report */
                           break;
                case 'm' : MUNGMODE = TRUE; /* allow overwriting of files */
                           break;
                case 'P':
                case 'p' : PMSG = TRUE;  /* print all messages */
                           break;
                case 'R':
                case 'r' : RECVFLAG = TRUE;  /* receive file */
                           XMITTYPE = gettype(argv[1][index++]);  /* get t/b */
                           break;
                case 'S':
                case 's' : SENDFLAG = TRUE;  /* send file */
                           XMITTYPE = gettype(argv[1][index++]);
                           break;
                case 'Y':
                case 'y' : STATDISP = TRUE;  /* display file status */
                           break;
                default  : error("Invalid Flag", FALSE);
                }

        if (BIT7 && (XMITTYPE == 'b'))
        {  printf("\nUMODEM:  Fatal Error -- Both 7-Bit Transfer and ");
           printf("Binary Transfer Selected");
#ifdef vms
           exit(SS$_NORMAL);
#else
           exit(-1);
#endif
        }

        if (BIT7)  /* set MASK value */
           BITMASK = 0177;  /* 7 significant bits */
        else
           BITMASK = 0377;  /* 8 significant bits */

        if (PMSG)
           { printf("\nSupported File Transfer Protocols:");
             printf("\n\tTERM II FTP 1");
             printf("\n\tCP/M UG XMODEM2 (TERM II FTP 3)");
             printf("\n\n");
           }

        if (LOGFLAG)
           { if (!DELFLAG)
                LOGFP = fopen(logfile, "a");  /* append to LOG file */
             else
                LOGFP = fopen(logfile, "w");  /* new LOG file */
             fprintf(LOGFP,"\n\n++++++++\n");
             fprintf(LOGFP,"\nUMODEM Version %d.%d\n", VERSION/10, VERSION%10);
             printf("\nUMODEM:  LOG File '%s' is Open\n", logfile);
           }

        if (STATDISP) yfile(argv[2]);  /* status of a file */

        if (RECVFLAG && SENDFLAG)
                error("Both Send and Receive Functions Specified", FALSE);
        if (!RECVFLAG && !SENDFLAG)
                error("Neither Send nor Receive Functions Specified", FALSE);

/*
 *      If receiving a file, first check to see if it exists.
 *      If MUNGMODE is off, then abort with error unless we're running
 *      under VMS, where CREAT() makes a file with a new version number.
 */
        if (RECVFLAG)
                {
#ifndef vms
                if (open(argv[2], 0) != -1)     /*  If file exists  */
                        {
                        printf("\nUMODEM:  Warning -- Target File Exists\n");
                        if ( MUNGMODE == FALSE )
                                error("Fatal - Can't overwrite file\n", FALSE);

                        printf("UMODEM:  Overwriting Target File\n");
                        }
#endif
                rfile(argv[2]);                 /*  Receive file  */
                }
        else
                sfile(argv[2]);                 /*  Send file  */

}

gettype(ichar)
char ichar;
        {
        switch (ichar)
                {
                case 'b':       case 'B':
                case 't':       case 'T':
                        return(ichar);

                default:
                        error("Invalid Send/Receive Parameter - not t or b",
                                                                FALSE);
                }

        return;
        }

/* set tty modes for UMODEM transfers */
setmodes()
{

/*  Device characteristics for JHU UNIX  */
#ifdef JHU      
        if (gtty(0, &ttys) < 0)  /* get current tty params */
                error("Can't get TTY Parameters", TRUE);

        tty = ttyname(0);  /* identify current tty */

        /* duplicate current modes in ttysnew structure */
        ttysnew.ispeed = ttys.ispeed;   /* copy input speed */
        ttysnew.ospeed = ttys.ospeed;   /* copy output speed */
        ttysnew.xflags = ttys.xflags;   /* copy JHU/UNIX extended flags */
        ttysnew.mode   = ttys.mode;     /* copy standard terminal flags */

        ttysnew.mode |= RAW;    /* set for RAW Mode */
                        /* This ORs in the RAW mode value, thereby
                           setting RAW mode and leaving the other
                           mode settings unchanged */
        ttysnew.mode &= ~ECHO;  /* set for no echoing */
                        /* This ANDs in the complement of the ECHO
                           setting (for NO echo), thereby leaving all
                           current parameters unchanged and turning
                           OFF ECHO only */
        ttysnew.mode &= ~XTABS;  /* set for no tab expansion */
        ttysnew.mode &= ~LCASE;  /* set for no upper-to-lower case xlate */
        ttysnew.mode |= ANYP;  /* set for ANY Parity */
        ttysnew.mode &= ~NL3;  /* turn off ALL delays - new line */
        ttysnew.mode &= ~TAB3; /* turn off tab delays */
        ttysnew.mode &= ~CR3;  /* turn off CR delays */
        ttysnew.mode &= ~FF1;  /* turn off FF delays */
        ttysnew.mode &= ~BS1;  /* turn off BS delays */
        /* the following are JHU/UNIX xflags settings; they are [SD] */
        ttysnew.xflags &= ~PAGE;  /* turn off paging */
        ttysnew.xflags &= ~STALL;  /* turn off ^S/^Q recognition */
        ttysnew.xflags &= ~TAPE;  /* turn off ^S/^Q input control */
        ttysnew.xflags &= ~FOLD;  /* turn off CR/LF folding at col 72 */
        ttysnew.xflags |= NB8;  /* turn on 8-bit input/output */

        if (stty(0, &ttysnew) < 0)  /* set new params */
                error("Can't set new TTY Parameters", TRUE);

        if (stat(tty, &statbuf) < 0)  /* get tty status */ 
                error("Can't get your TTY Status", TRUE);

        if (statbuf.st_mode&011)  /* messages are on [SD] */
        {       wason = TRUE;
                if (chmod(tty, 020600) < 0)  /* turn off tty messages [SD] */
                        error("Can't change TTY Mode", TRUE);
        }       
        else
                wason = FALSE;  /* messages are already off */
#endif          

/*  Device characteristics for Version 7 UNIX  */
#ifdef VER7
        if (ioctl(0,TIOCGETP,&ttys)<0)  /* get tty params [V7] */
                error("Can't get TTY Parameters", TRUE);
        tty = ttyname(0);  /* identify current tty */
        
        /* transfer current modes to new structure */
        ttysnew.sg_ispeed = ttys.sg_ispeed;     /* copy input speed */
        ttysnew.sg_ospeed = ttys.sg_ospeed;     /* copy output speed */
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
        ttysnew.sg_flags &= ~TAB2; /* turn off tab delays */
        ttysnew.sg_flags &= ~CR3;  /* turn off CR delays */
        ttysnew.sg_flags &= ~FF1;  /* turn off FF delays */
        ttysnew.sg_flags &= ~BS1;  /* turn off BS delays */
        ttysnew.sg_flags &= ~TANDEM;  /* turn off flow control */

        /* make sure page mode is off */
/*      ioctl(0,TIOCSSCR,&pagelen);             This is [SD]!  */
        
        /* set new paramters */
        if (ioctl(0,TIOCSETP,&ttysnew) < 0)
                error("Can't set new TTY Parameters", TRUE);
#endif  

/*  Device characteristics for VMS  */
#ifdef vms
        int     *iptr, parameters;

/*
 *      Get current terminal parameters.
 */
        if (gtty(&ttys) != SS$_NORMAL)
                error("SETMODES:  error return from GTTY (1)", FALSE);
        if (gtty(&ttysnew) != SS$_NORMAL)
                error("SETMODES:  error return from GTTY (2)", FALSE);

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
                error("SETMODES:  error return from STTY", TRUE);
#endif

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
        sendbyte(IAC);  /* set B O S (Binary Output Start) */
        sendbyte(WILL);
        sendbyte(TRBIN);

        sendbyte(IAC);  /* set B I S (Binary Input Start) */
        sendbyte(DO);
        sendbyte(TRBIN);

        sleep(3);  /* wait for TIP to configure */

        return;
}

/* restore normal tty modes */
restoremodes(errcall)
int errcall;
{
        if (ARPA)  /* if ARPA Net, reconfigure */
                resetarpa();

/*  Device characteristic restoration for JHU UNIX  */
#ifdef JHU
        if (wason)  /* if messages were on originally */
                if (chmod(tty, 020611) < 0)  /*  [SD]  */
                        error("Can't change TTY Mode", FALSE);

        if (stty(0, &ttys) < 0)  /* restore original tty modes */
                { if (!errcall)
                   error("RESET - Can't restore normal TTY Params", FALSE);
                else
                   { printf("UMODEM:  ");
                     printf("RESET - Can't restore normal TTY Params\n");
                   }
                }
#endif

/*  Device characteristic restoration for Version 7 UNIX  */
#ifdef VER7
        if (ioctl(0,TIOCSETP,&ttys) < 0)
                { if (!errcall)
                   error("RESET - Can't restore normal TTY Params", FALSE);
                else
                   { printf("UMODEM:  ");
                     printf("RESET - Can't restore normal TTY Params\n");
                   }
                }
#endif

/*  Device characteristic restoration for VMS  */
#ifdef vms
        if (stty(&ttys) != SS$_NORMAL)          /*  Restore original modes  */
                {
                if (!errcall)
                        error("Error restoring original terminal params.",
                                                                        FALSE);
                else
                        {
                        printf("UMODEM/RESTOREMODES:  ");
                        printf("Error restoring original terminal params.\n");
                        }
                }
#endif

        if (PMSG)
                { printf("\nUMODEM:  TTY Device Parameters Restored");
                  ttyparams();  /* print tty params */
                }

        return;
}

/* reset the ARPA Net */
resetarpa()
{
        sendbyte(IAC);  /* send B O E (Binary Output End) */
        sendbyte(WONT);
        sendbyte(TRBIN);

        sendbyte(IAC);  /* send B I E (Binary Input End) */
        sendbyte(DONT);
        sendbyte(TRBIN);

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
        if (LOGFLAG)
        {   fprintf(LOGFP, "UMODEM Fatal Error:  %s\n", msg);
            fclose(LOGFP);
        }
#ifdef vms
        exit(SS$_NORMAL);
#else
        exit(-1);
#endif
}

/**  print status (size) of a file  **/
yfile(name)
char *name;
{
        printf("UMODEM File Status Display for %s\n", name);
        if (LOGFLAG) fprintf(LOGFP,"UMODEM File Status Display for %s\n",
          name);

        if (open(name,0) < 0)
        {  printf("File %s does not exist\n", name);
           if (LOGFLAG) fprintf(LOGFP,"File %s does not exist\n", name);
#ifdef vms
           exit(SS$_NORMAL);
#else
           exit(-1);
#endif
        }

        prfilestat(name);  /* print status */
        printf("\n");
        if (LOGFLAG)
        {  fprintf(LOGFP,"\n");
           fclose(LOGFP);
        }

#ifdef vms
        exit(SS$_NORMAL);
#else
        exit(0);
#endif
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
        int             lowlim, nlflag;
#ifdef vms
        char            inbuf[BBUFSIZ + 7];
#endif

        mode = XMITTYPE;  /* set t/b mode */
        if ((fd = creat(name, CREATMODE)) < 0)
                error("Can't create file for receive", FALSE);
        printf("\r\nUMODEM:  File Name: %s", name);
        if (LOGFLAG)
        {    fprintf(LOGFP, "\n----\nUMODEM Receive Function\n");
             fprintf(LOGFP, "File Name: %s\n", name);
             if (FTP1)
                fprintf(LOGFP, "TERM II File Transfer Protocol 1 Selected\n");
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

        setmodes();  /* setup tty modes for xfer */
        recfin = FALSE;
        sectnum = errors = 0;
        fatalerror = FALSE;  /* NO fatal errors */
        recvsectcnt = 0;  /* number of received sectors */

        if (mode == 't' || mode == 'T')
                tmode   = TRUE;
        else
                tmode   = FALSE;

        if (FTP1)
        {
          while (readbyte(4) != SYN);
          sendbyte(ACK);  /* FTP 1 Sync */
        }
        else sendbyte(NAK);  /* FTP 3 Sync */

        nlflag  = FALSE;
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
               {
#ifdef vms
/*
 *      Under VMS, read the whole block at once.
 */
               if (FTP1)
                        {
                        raw_read(BBUFSIZ + 7, inbuf,
                                                5 + delay * (BBUFSIZ + 6));
                        sectcurr = inbuf[1] & BITMASK;
                        sectcomp = inbuf[2] & BITMASK;
                        startstx = inbuf[3] & BITMASK;
                        }
               else
                        {
                        raw_read(BBUFSIZ + 3, inbuf, delay * (BBUFSIZ + 3));
                        sectcurr = inbuf[0] & BITMASK;
                        sectcomp = inbuf[1] & BITMASK;
                        }

#else
               if (FTP1) readbyte(5);  /* discard leading zero */
               sectcurr = readbyte(delay);
               sectcomp = readbyte(delay);
               if (FTP1) startstx = readbyte(delay);  /* get leading STX */

#endif
               if ((sectcurr + sectcomp) == BITMASK)
               {  if (sectcurr == ((sectnum + 1) % 256) & BITMASK)
                  {  checksum = 0;
#ifdef vms
                     if (FTP1)
                        lowlim  = 4;
                     else
                        lowlim  = 2;
#else
                     lowlim     = 0;
#endif

                     bufctr     = 0;
                     for (j = lowlim; j < lowlim + BBUFSIZ; j++)
                        {

#ifdef vms
                        buff[bufctr] = c = inbuf[j] & BITMASK;
#else
                        buff[bufctr] = c = readbyte(delay);
#endif

                        checksum = (checksum+c)&BITMASK;
                        if (!tmode)  /* binary mode */
                        {  bufctr++;
                           continue;
                        }

/*
 *      Translate CP/M's CR/LF into UNIX's LF, but don't do it by
 *      simply ignoring a CR from CP/M--it may indicate an
 *      overstruck line!
 */
                        if (nlflag)
                                {
/*
 *      Last packet ended with a CR.  If the first character of this
 *      packet ISN'T a LF, then be sure to send the CR.
 */
                                nlflag  = FALSE;
                                if (c != LF)
                                        {
                                        buff[bufctr++]  = CR;
                                        buff[bufctr]    = c;
                                        }
                                }
                        if (j == lowlim + BBUFSIZ - 1 && c == CR)
                                {
/*
 *      Last character in the packet is a CR.  Don't send it, but
 *      turn on NLFLAG to make sure we check the first character in
 *      the next packet to see if it's a LF.
 */
                                nlflag  = TRUE;
                                continue;
                                }
                        if (c == LF && bufctr && buff[bufctr - 1] == CR)
                                {
/*
 *      We have a CR/LF pair.  Discard the CR.
 */
                                buff[bufctr - 1]        = LF;
                                continue;
                                }
                        if (c == CTRLZ)  /* skip CP/M EOF char */
                        {  recfin = TRUE;  /* flag EOF */
                           continue;
                        }
                        if (!recfin)
                           bufctr++;
                     }

#ifdef vms
                     if (FTP1)
                        {
/*  Ending ETX  */      endetx          = inbuf[lowlim + BBUFSIZ] & BITMASK;
/*  Checksum  */        inchecksum      = inbuf[lowlim + BBUFSIZ + 1] & BITMASK;
/*  ENQ  */             endenq          = inbuf[lowlim + BBUFSIZ + 2] & BITMASK;
                        }
                     else
/*  Checksum  */        inchecksum      = inbuf[lowlim + BBUFSIZ] & BITMASK;
                        
#else
                     if (FTP1) endetx = readbyte(delay);  /* get ending ETX */
                     inchecksum = readbyte(delay);  /* get checksum */
                     if (FTP1) endenq = readbyte(delay); /* get ENQ */
#endif

                     if (checksum == inchecksum)  /* good checksum */
                     {  errors = 0;
                        recvsectcnt++;
                        sectnum = sectcurr;  /* update sector counter */
                        if (write(fd, buff, bufctr) < 0)
                           error("File Write Error", TRUE);
                        else
                        {  if (FTP1) sendbyte(ESC);  /* FTP 1 requires <ESC> */
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
                  { if (sectcurr == (sectnum % 256) & BITMASK)
                    {  while(readbyte(3) != TIMEOUT);
                       if (FTP1) sendbyte(ESC);  /* FTP 1 requires <ESC> */
                       sendbyte(ACK);
                    }
                    else
                    {  if (LOGFLAG)
                        { fprintf(LOGFP, "Phase Error--received sector is %d ",
                                        sectcurr);
                          fprintf(LOGFP, "while expected sector is %d (%d)\n",
                                        ((sectnum + 1) % 256) & BITMASK,
                                        sectnum);
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
        fclose(LOGFP);
     }
     printf("\n");
#ifdef vms
     exit(SS$_NORMAL);
#else
     exit(0);
#endif
  }
  else
  {  if (LOGFLAG && FTP1 && fatalerror) fprintf(LOGFP,
        "Synchronization Error");
     if (errors == ERRORMAX)
             error("Too many errors", TRUE);
     if (fatalerror)
             error("Fatal error", TRUE);
  }
}

/**  send a file  **/
sfile(name)
char *name;
{
        char mode;
        int fd, charval, attempts;
        int nlflag, sendfin, tmode;
        register int bufctr, checksum, sectnum;
        char c;
        int sendresp;  /* response char to sent block */

        mode = XMITTYPE;  /* set t/b mode */
        if ((fd = open(name, 0)) < 0)
        {  if (LOGFLAG) fprintf(LOGFP, "Can't Open File\n");
           error("Can't open file for send", FALSE);
        }
        printf("\r\nUMODEM:  File Name: %s", name);
        if (LOGFLAG)
        {   fprintf(LOGFP, "\n----\nUMODEM Send Function\n");
            fprintf(LOGFP, "File Name: %s\n", name);
        }
        prfilestat(name);  /* print file size statistics */
        if (LOGFLAG)
        {  if (FTP1)
                fprintf(LOGFP, "TERM II File Transfer Protocol 1 Selected\n");
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

        setmodes();  /* setup tty modes for xfer */     
        if (mode == 't' || mode == 'T')
                tmode   = TRUE;
        else
                tmode   = FALSE;

        sendfin = nlflag = FALSE;
        attempts = 0;

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
                if ((charval = read(fd, &c, 1)) < 0)
                   error("File Read Error", TRUE);
                if (charval == 0)  /* EOF for read */   
                {  sendfin = TRUE;  /* this is the last sector */
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
            do
            {   sendbyte(SOH);  /* send start of packet header */
                if (FTP1) sendbyte(0);  /* FTP 1 Type 0 Packet */
                sendbyte(sectnum);       /* send current sector number */
                sendbyte(-sectnum-1);    /* and its complement */

                if (FTP1) sendbyte(STX);  /* send STX */
                checksum = 0;  /* init checksum */
                for (bufctr=0; bufctr < BBUFSIZ; bufctr++)
                {  sendbyte(buff[bufctr]);  /* send the byte */
                   if (ARPA && (buff[bufctr]==0xff))  /* ARPA Net FFH esc */
                        sendbyte(buff[bufctr]);  /* send 2 FFH's for one */
                   checksum = (checksum+buff[bufctr])&BITMASK;
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
                        fprintf(LOGFP, "This non-ACK was a TIMEOUT\n");
                     else if (sendresp == NAK)
                        fprintf(LOGFP, "This non-ACK was a NAK\n");
                   }
                if (sendresp == CAN)
                        {
                        if (LOGFLAG)
                                fprintf(LOGFP, "This non-ACK was a CAN\n");

#ifdef CAN_BOMB
                        if (LOGFLAG)
                                fprintf(LOGFP,
                                        "Exiting:  got a CAN from receiver.\n");
                        close(fd);
                        restoremodes(TRUE);
                        sleep(5);       /*  Allow other side to recover  */
                        printf("Exiting:  got a CAN from receiver.\n\n");
#ifdef vms
                        exit(SS$_NORMAL);
#else
                        exit(-1);
#endif

#endif
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
       fclose(LOGFP);
    }
    printf("\n");
#ifdef vms
    exit(SS$_NORMAL);
#else
    exit(0);
#endif

}

/*  print file size status information  */
prfilestat(name)
char *name;
{
        long            bytes, Kbytes, records;

#ifdef vms
        bytes           = filestat(name);       /*  Gets file length  */
        if (bytes < 0)
                {
                printf("PRFILESTAT:  error return from FILESTAT\n");
                return;
                }

#else
        struct stat     filestatbuf;            /*  File status info  */

        stat(name, &filestatbuf);               /*  Get file status bytes  */
        bytes           = filestatbuf.st_size;
#endif

        Kbytes          = (bytes / 1024) + 1;
        records         = (bytes /  128) + 1;

        printf("\nUMODEM:  Estimated File Size %ldK, %ld Records, %ld Bytes",
                                Kbytes, records, bytes);
        printf("\n         Estimated transfer time at 300 baud:  ");
        printf("%ld min, %ld sec.",
                                bytes / (30*60), ((bytes % (30*60)) / 30) + 1);

        if (LOGFLAG)
                fprintf(LOGFP,
                        "Estimated File Size %ldK, %ld Records, %ld Bytes\n",
                                Kbytes, records, bytes);

        return;
}

/* get a byte from data stream -- timeout if "seconds" elapses */
/*      NOTE, however, that this function returns an INT, not a BYTE!!!  */
readbyte(seconds)
unsigned        seconds;
        {
        int     c;

#ifdef vms
        c       = raw_read(1, &c, seconds);

        if (c == SS$_TIMEOUT)
                return(TIMEOUT);

#else
        int     alarmfunc();
        
        signal(SIGALRM,alarmfunc);  /* catch alarms */  
        alarm(seconds);  /* set the alarm clock */

        if (read(0, &c, 1) < 0)     /* get a char; error means we timed out */
          {
             return(TIMEOUT);
          }
        alarm(0);  /* turn off the alarm */

#endif

        return(c & BITMASK);  /* return the char */
        }

/* send a byte to data stream */
sendbyte(data)
char    data;
        {
        char    dataout;

        dataout = data & BITMASK;               /*  Mask for 7 or 8 bits  */

#ifdef vms
        raw_write(dataout);

#else
        write(1, &dataout, 1);  /* write the byte */

#endif

        return;
        }

#ifndef vms
/* function for alarm clock timeouts */
alarmfunc()
{
        return;  /* this is basically a dummy function to force error */
                 /* status return on the "read" call in "readbyte"    */
}
#endif

/* print data on TTY setting */
ttyparams()
{

/*  For VMS, report that no information is available and return  */
#ifdef vms
        printf("\nUMODEM/TTYPARAMS:  ");
        printf("TT device parameter display not implemented under VMS.\n");

#else

/*  Obtain TTY parameters for JHU UNIX  */
#ifdef JHU      
        gtty(0, &ttystemp);  /* get current tty params */
#endif

/*  Obtain TTY parameters for Version 7 UNIX  */
#ifdef VER7
        ioctl(0,TIOCGETP,&ttystemp);
#endif

        printf("\r\n\nTTY Device Parameter Display\r\n");

        tty = ttyname(0);  /* get name of tty */
          printf("\tTTY Device Name is %s\r\n\n", tty);
          printf("\tAny Parity Allowed "); pryn(ANYP);
          printf("\tEven Parity Allowed"); pryn(EVENP);
          printf("\tOdd Parity Allowed "); pryn(ODDP);
          printf("\tLower Case Map     "); pryn(LCASE);
          printf("\tTabs Expanded      "); pryn(XTABS);
          printf("\tCR Mode Enabled    "); pryn(CRMOD);
          printf("\tEcho Enabled       "); pryn(ECHO);
          printf("\tRAW Mode Enabled   "); pryn(RAW);

/*  Print extended terminal characteristics for JHU UNIX  */
#ifdef JHU
        stat(tty, &statbuf);  /* get more tty params */

          printf("\tBinary Mode Enabled"); pryn1(NB8);
          printf("\tCR/LF in Col 72    "); pryn1(FOLD);
          printf("\tRecognize ^S/^Q    "); pryn1(STALL);
          printf("\tSend ^S/^Q         "); pryn1(TAPE);
          printf("\tTerminal can BS    "); pryn1(SCOPE);
          printf("\r\n");  /* New line to separate topics */
          printf("\tTerminal Paging is "); pryn1(PAGE);
            if (ttystemp.xflags&PAGE)
                printf("\t  Lines/Page is %d\r\n", ttystemp.xflags&PAGE);
          printf("\r\n");  /* New line to separate topics */
          printf("\tTTY Input Rate     :   ");
            prbaud(ttystemp.ispeed);  /* print baud rate */
          printf("\tTTY Output Rate    :   ");
            prbaud(ttystemp.ospeed);  /* print output baud rate */
          printf("\r\n");  /* New line to separate topics */
          printf("\tMessages Enabled   ");
                if (statbuf.st_mode&011)
                   printf(":   Yes\r\n");
                else
                   printf(":   No\r\n");
#endif

/*  Print extended characteristics for Version 7 UNIX  */
#ifdef VER7
          printf("\tTTY Input Rate     :   ");
            prbaud(ttystemp.sg_ispeed);
          printf("\tTTY Output Rate    :   ");
            prbaud(ttystemp.sg_ospeed);  /* print output baud rate */
#endif

#endif                                  /*  END #ifdef vms...#else...  */
}

#ifndef vms
pryn(iflag)
int iflag;
{

/*  JHU UNIX flag test  */
#ifdef JHU
        if (ttystemp.mode&iflag)

/*  Version 7 UNIX and VMS flag test  */
#else
        if (ttystemp.sg_flags&iflag)
#endif

           printf(":   Yes\r\n");
        else
           printf(":   No\r\n");
}
#endif

/*  Extended flag test for JHU UNIX only  */
#ifdef JHU
pryn1(iflag)
int iflag;
{
        if (ttystemp.xflags&iflag)
           printf(":   Yes\r\n");
        else
           printf(":   No\r\n");
}
#endif

#ifndef vms
prbaud(speed)
char speed;
{
        switch (speed) {

/*  JHU UNIX speed flag cases  */
#ifdef JHU              
                case B0050 : printf("50"); break;
                case B0075 : printf("75"); break;
                case B0110 : printf("110"); break;
                case B0134 : printf("134.5"); break;
                case B0150 : printf("150"); break;
                case B0200 : printf("200"); break;
                case B0300 : printf("300"); break;
                case B0600 : printf("600"); break;
                case B1200 : printf("1200"); break;
                case B1800 : printf("1800"); break;
                case B2400 : printf("2400"); break;
                case B4800 : printf("4800"); break;
                case B9600 : printf("9600"); break;
                case EXT_A : printf("External A"); break;
                case EXT_B : printf("External B"); break;
#endif

/*  Version 7 UNIX speed flag cases  */
#ifdef VER7
                case B50 : printf("50"); break;
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
                case EXTA : printf("External A"); break;
                case EXTB : printf("External B"); break;
#endif

                default    : printf("Error"); break;
        }
        printf(" Baud\r\n");
}
#endif
