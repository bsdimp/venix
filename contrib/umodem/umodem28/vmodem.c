/*
 *  VMODEM
 *  VMS support for UMODEM program
 *
 *	Defined herein are some utility routines to make the UNIX
 *	program UMODEM run under VAX/VMS C:
 *
 *		ASSIGN_CHANNEL	Calls the VMS System Service $ASSIGN
 *				to assign a channel to a device.
 *				The routine currently has the device
 *				"TT" hardwired into it.
 *		GTTY		Gets terminal characteristics, almost
 *				like the UNIX GTTY system call.
 *		FILESTAT	Provide FILE STATistics under VMS:
 *				calls RMS to find out the length of
 *				the file specified in the argument.
 *		RAW_READ	Reads characters from the terminal
 *				without any echoing or interpretation
 *				and with an optional timeout period.
 *		RAW_WRITE	Writes a character to the terminal
 *				without any interpretation.
 *		STTY		Sets terminal characteristics, almost
 *				like the UNIX STTY system call.
 *
 *	Some of the ideas used here were obtained from code written
 *	by Max Benson and Robert Bruccoleri.
 *
 *  Walter Reiher
 *  Harvard University
 *  Department of Chemistry
 *  12 Oxford Street
 *  Cambridge, MA 02138
 *  March 11, 1983
 */
#include descrip
#include iodef
#include rms
#include ssdef
#include stdio
#include "vmodem.h"

#define  TRUE	1
#define  FALSE	0

static char	tt_name[]	= "TT";
static short	tt_chan		= -1;		/*  Terminal channel number  */

struct	tt_io_iosb				/*  Terminal I/O IOSB  */
		{
		short	status;
		short	byte_count;
		short	terminator;
		short	terminator_size;
		};

/*
 *	Terminator mask for PASSALL reads.
 *	Permits reads of all possible 8-bit characters.
 */
int	t_mask[32]	=  {	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
				 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
				 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
				 0,  0					};
struct	terminator_mask
		{
		short	size ;
		short	unused ;
		int	*mask ;
		}
		termin_mask	= { 32, 0, t_mask };
/*
 */
assign_channel()
	{
/*
 *	ASSIGN a channel to the logical name TT, which is usually
 *	the terminal.
 */
	int	status;
	$DESCRIPTOR(tt_descriptor, tt_name);

	if (tt_chan == -1)
		status	= sys$assign(&tt_descriptor, &tt_chan, 0, 0);
	else
		status	= SS$_NORMAL;

	if (status != SS$_NORMAL || tt_chan == -1)
		error("ASSIGN_CHANNEL:  error in SYS$ASSIGN\n", FALSE);

	return;
	}

gtty(tt_characteristics)
struct	tt_info	*tt_characteristics;
	{
/*
 *	Gets terminal information from VMS.
 */
	int				status;

	if (tt_chan == -1)
		assign_channel();

	status	= sys$qiow(0, tt_chan, IO$_SENSEMODE,
			&(tt_characteristics->dev_modes), NULL, 0,
			&(tt_characteristics->dev_characteristics), 12,
			0, 0, 0, 0);
	if (status != SS$_NORMAL ||
			tt_characteristics->dev_modes.status != SS$_NORMAL)
		error("GTTY:  sense mode QIO error return.\n", FALSE);

	return(status);
	}

/*
 *  FILESTAT
 *  Provide FILE STATistics under VMS
 *
 *	Calls RMS (ugh!) to find out something about the file whose
 *	name is pointed to in the argument FILENAME.
 *	Returns the number of bytes in the file, returns -1 on error.
 */
long	filestat(name)
char	*name;
	{
	short		lastbyte;
	long		blocks, bytes;
	int		status;
	struct	FAB	fab;
	struct	XABFHC	xabfhc;

	fab		= cc$rms_fab;
	xabfhc		= cc$rms_xabfhc;

	fab.fab$l_xab	= &xabfhc;
	fab.fab$l_fna	= name;
	fab.fab$b_fns	= strlen(name);

	status	= sys$open(&fab);
	if (status != RMS$_NORMAL && status != RMS$_KFF)
		{
		if (status == RMS$_FNF)
			printf("FILESTAT:  File %s not found.\n", name);
		else
			{
			printf("FILESTAT:  Error in $OPEN of ");
			printf("file %s, status = 0x%X\n", name, status);
			}
		return(-1);
		}

/*
 *	Pull out the end-of-file block and the first free byte in the
 *	end-of-file block from the XAB.
 */
	blocks		= fab.fab$l_xab->xab$l_ebk - 1;
	lastbyte	= fab.fab$l_xab->xab$w_ffb - 1;

	bytes	= blocks * 512 + lastbyte;

	status	= sys$close(&fab);
	if (status != RMS$_NORMAL)
		{
		printf("FILESTAT:  Error in $CLOSE of ");
		printf("file %s, status = 0x%X\n", name, status);
		return(-1);
		}

	return(bytes);
	}

int		raw_read(nchar, charbuf, seconds)
char		*charbuf;
int		nchar;
unsigned	seconds;
	{
/*
 *	Read NCHAR characters from the terminal without echoing or
 *	interpretation.
 *	If the argument SECONDS is non-zero, use that as the
 *	timeout period in seconds for the read.
 *
 *	NOTE THAT THIS FUNCTION RETURNS AN INT, NOT A CHAR!
 *	That is because of the possibility of a SS$_TIMEOUT return.
 */
	short			function;
	int			status;
	struct	tt_io_iosb	iosb;

	if (tt_chan == -1)
		assign_channel();

	function	= IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR;

	if (seconds)
		status	= sys$qiow(0, tt_chan, function | IO$M_TIMED,
				&iosb, NULL, 0,
				charbuf, nchar, seconds,
				&termin_mask, NULL, 0);
	else
		status	= sys$qiow(0, tt_chan, function,
				&iosb, NULL, 0,
				charbuf, nchar, 0,
				&termin_mask, NULL, 0);

	if (status == SS$_TIMEOUT || iosb.status == SS$_TIMEOUT)
		return(SS$_TIMEOUT);
	else if (status != SS$_NORMAL || iosb.status != SS$_NORMAL)
		error("RAW_READ:  read QIO error return.\n", TRUE);

	return((int)*charbuf);
	}

raw_write(c)
char	c;
	{
/*
 *	Writes a character to the terminal without echoing or
 *	interpretation.
 */
	int			status;
	struct	tt_io_iosb	iosb;

	if (tt_chan == -1)
		assign_channel();

	status	= sys$qiow(0, tt_chan,
			IO$_WRITEVBLK | IO$M_CANCTRLO | IO$M_NOFORMAT,
			&iosb, NULL, 0,
			&c, 1, 0, 0, 0, 0);

	if (status != SS$_NORMAL || iosb.status != SS$_NORMAL)
		error("RAW_WRITE:  write QIO error return.\n", TRUE);

	return;
	}

stty(tt_characteristics)
struct	tt_info	*tt_characteristics;
	{
/*
 *	Sets terminal information from VMS.
 */
	short			*f_ptr, *p_ptr, *s_ptr;
	int			status;
	struct	tt_mode_iosb	iosb;

	if (tt_chan == -1)
		assign_channel();

/*
 *	We do the following in order to get a full short, concatenating
 *	two adjacent chars:
 */
	s_ptr	= &(tt_characteristics->dev_modes.t_speed);	/*  Speeds  */
	f_ptr	= &(tt_characteristics->dev_modes.CR_fill);	/*  Fills  */
	p_ptr	= &(tt_characteristics->dev_modes.parity_flags);

	status	= sys$qiow(0, tt_chan, IO$_SETMODE,
			&iosb, NULL, 0,
			&(tt_characteristics->dev_characteristics), 12,
			*s_ptr, *f_ptr, *p_ptr, 0);
	if (status != SS$_NORMAL || iosb.status != SS$_NORMAL)
		printf(stderr, "STTY:  set mode QIO error return.\n");

	return(status);
	}