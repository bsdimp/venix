/*
 * calls: calls prints a paragraphed list of who calls whom within
 *	a body of source code.
 *
 * Author: M.M. Taylor, DCIEM, Toronto, Canada.
 * 22/Jan/81, Alexis Kwan (HCR at DCIEM).
 *	Modified for V7 and stdio,
 * 12-Jun-84, Kevin Szabo
 * watmath!wateng!ksbszabo (Elec Eng, U of Waterloo)
 *	Fixed bugs with '_' and variable names, names > ATOM_LENGTH chars.
 * 8/8/84, Tony Hansen, AT&T-IS, pegasus!hansen.
 *	Modified to use getopt,
 *	files are passed through CPP with "cc -E"
 *	multiple filenames and '-' are allowed on the command line,
 *	added -D, -U, -I and -f options,
 *		(CPP prefers filenames rather than stdin. To make this
 *		easier and faster, filenames needed to be allowed on the
 *		command line. This conflicted with the old usage of
 *		specifying function names on the command line, unfortunately.
 *		The -f option was added to put that capability back. Passing
 *		things through the CPP has the advantage of not picking up
 *		macros as function calls and avoids all hassles with the
 *		strangeness that can be done with the CPP. Also, it allows
 *		the addition of the -D, -U and -I options.)
 *	portable to unsigned char machines,
 *	handle 31 chars in variable names
 *		(chosen over flexnames because of the pending ANSI standards),
 *	fixed bug with some keywords tagged as function names,
 *	fixed bug with '_' at beginning of variable name,
 *	fixed bug scanning file with CPP statement in the first line,
 *	skips forward declarations external to a function body,
 *	fixed bug to print out functions which are defined and
 *		recursive to themselves, but aren't called elsewhere
 *		within the same file,
 *	added comments in many places,
 *	did more de-linting
 *		(I didn't cast functions to void because earlier
 *		compilers didn't have it, nor declare exit() because
 *		different versions of UNIX declare it differently),
 *	rearranged structures to minimize padding,
 *	dashes between deeply nested lists now vary with paper width,
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#define ATOMLENGTH 32			/* max size of name is 31 chars */
#define MAXNAME 500			/* # of names to be followed */
#define MAXINST 4000			/* # of instances of those names */
#define MAXSEEN 100			/* # of instances w/in a function */
#define MAXDEPTH 25			/* max output depth level */
#define PAPERWIDTH 132			/* limits tabbing */

int bracket = 0,			/* curly brace count */
    linect = 0;				/* line number */
int activep = 0;			/* current function being output */

/* options */
int terse = 1,				/* track functions only once */
    ntabs = (PAPERWIDTH - 20)/8,	/* how wide to go */
    functionlist = 0;			/* restrict to functions listed */

char *progname;				/* argv[0] */
FILE *input;				/* open file */
char *arglist = "tvw:f:D:U:I:";		/* valid options */
char *dashes;				/* separators for deep nestings */

/*
    These are C tokens after which a parenthesis is valid
    which would otherwise be tagged as function names. The
    reserved words which are not listed are break, continue,
    default and goto.
*/

char *sysword [] = {
	"auto", "case", "char", "do", "double", "else",
	"entry", "enum", "extern", "float", "for", "fortran",
	"if", "int", "long", "register", "return", "short",
	"sizeof", "static", "struct", "switch", "typedef",
	"union", "unsigned", "void", "while",
	0
	};

/* list of names being tracked */
struct rname {
	struct rinst *dlistp;
	int rnamecalled;
	int rnameout;
	char namer[ATOMLENGTH];
	} namelist[MAXNAME];

/* list of calling instances of those names */
struct rinst {
	struct rname *namep;
	struct rinst *calls;
	struct rinst *calledby;
	} dlist[MAXINST];

/* list of names currently being gathered within a function */
char aseen [MAXSEEN][ATOMLENGTH];

/* list of names currently being output */
struct rname *activelist[MAXDEPTH];

/* free list pointer */
struct rinst *frp = dlist;

extern int atoi();
extern char *strcat(), *strcpy();
extern int getopt();
extern char *optarg;
extern int optind;
extern char *tmpnam();
extern char *sys_errlist[];
extern int sys_nerr;

/* forward declarations */
struct rname *lookfor(), *place();
struct rinst *newproc(), *getfree(), *install();
char *syserrlist(), *addtocpp();

main(argc,argv)
int argc;
char *argv[];
{
    char cppcommand[5120];	/* 5120 is the max # chars on command line */
    register char *cppptr = cppcommand;
    char _dashes[1024];
    register int c, i, width = PAPERWIDTH;

    progname = argv[0];
    cppptr = addtocpp(cppptr, &cppcommand[5120], "cc -E", "");	/* /lib/cpp */
    initfree();

/*
    get arguments and flags:
	-t	terse form  (default case)
	-v	verbose form
	-w nn	paper width  (default 132)
	-f name	function to start printing from
    arguments to pass on to CPP
	-D def	#define def
	-U def	#undef def
	-I inc	#include inc
*/
    while ((c = getopt (argc, argv, arglist)) != EOF)
	switch (c)
	    {
	    case 't':	terse = 1;		break;
	    case 'v':	terse = 0;		break;
	    case 'f':	functionlist = 1;	break;
	    case 'w':
		width = atoi(optarg);
		if (width <= 0)
		    width = PAPERWIDTH;
		break;
	    case 'I':
		cppptr = addtocpp (cppptr, &cppcommand[5120], " -I", optarg);
		break;
	    case 'D':
		cppptr = addtocpp (cppptr, &cppcommand[5120], " -D", optarg);
		break;
	    case 'U':
		cppptr = addtocpp (cppptr, &cppcommand[5120], " -U", optarg);
		break;
	    case '?':
		(void) fprintf (stderr,
		    "usage: %s [-tv] [-f function] [-w width] [-D define] [-U undefine] [-I include-dir] [filenames]\n",
		    progname);
		exit (1);
	    }

    /* initialize the dashed separator list for deep nesting */
    ntabs = (width - 20) / 8;
    for (i = 0; (i < width) && (i < 1024); i += 2)
	{
	_dashes[i] = '-';
	_dashes[i+1] = ' ';
	}
    if (i < 1024)
	_dashes[i] = '\0';
    else
	_dashes[1023] = '\0';
    dashes = _dashes;

    scanfiles(argc, argv, cppcommand);
    exit(0);
}

/*
    Add the given string onto the end of the CPP command string.
*/

char *
addtocpp(cppptr, endptr, first, second)
register char *cppptr, *endptr, *first, *second;
{
    while ((cppptr < endptr) && *first)
	*cppptr++ = *first++;
    while ((cppptr < endptr) && *second)
	*cppptr++ = *second++;
    *cppptr = '\0';
    return cppptr;
}

/*
    Process() invokes the C preprocessor on the named file so that
    its output may be used as input for scanning.
*/

process(cppcommand, filename)
register char *cppcommand;
register char *filename;
{
    char command[5120];
    register int ret;
    FILE *popen();

    if (access (filename, 04) != 0)
	{
	(void) fprintf (stderr, "%s: cannot open file '%s' (%s).\n",
	    progname, filename, syserrlist());
	return;
	}
    sprintf (command, "%s %s", cppcommand, filename);
    input = popen (command, "r");
    if (input == NULL)
	{
	(void) fprintf (stderr,
	    "%s: fork of CPP command '%s' failed on file '%s' (%s).\n",
	    progname, command, filename, syserrlist());
	return;
	}
    addfuncs();
    ret = pclose(input);
    if (ret != 0)
	(void) fprintf (stderr,
	    "%s: CPP command '%s' failed on file '%s' with return code %d (%s).\n",
	    progname, command, filename, ret, syserrlist());
}

char *
syserrlist()
{
    extern int errno;
    register char *ret =
	errno == 0 ?
	    "errno = 0" :
	errno < sys_nerr ?
	    sys_errlist[errno] :
	    "errno out of range";
    errno = 0;
    return ret;
}

/*
    addfuncs() scans the input file for function names and adds them to
    the calling list.
*/

addfuncs()
{
    register int ok = 1, internal;
    char atom[ATOMLENGTH];
    register struct rinst *curproc = 0;

    atom[0] = '\0';
    while ((internal = getfunc(atom)) != -1 && ok )
	if (internal)	ok = add2call(atom,curproc);
	else		ok = (int)(curproc = newproc(atom));
}

/*
    Since CPP can't be piped into, dostandardinput() takes the standard
    input and stuffs it into a file so that process() can work on it.
*/

dostandardinput(cppcommand)
char *cppcommand;
{
    register int c;
    char *mktemp();
    register char *filename = mktemp ("/tmp/callsXXXXXX");
    register FILE *ofileptr = fopen (filename, "w");

    if (ofileptr == NULL)
	{
	(void) fprintf (stderr,
	    "%s: cannot open tempfile '%s' for writing (%s).\n",
	    progname, filename, syserrlist());
	return;
	}
    while ( (c = getchar()) != EOF)
	putc (c, ofileptr);
    fclose (ofileptr);
    process (cppcommand, filename);
    unlink(filename);
}

/* Scan the input files. */
scanfiles(argc, argv, cppcommand)
int argc;
char **argv;
char *cppcommand;
{
    /* Dumptree modifies optind, so use a local version here. */
    register int loptind = optind;

    if (loptind >= argc)
	{
	dostandardinput(cppcommand);
	dumptree(argc,argv);
	}
    else
	{
	for ( ; loptind < argc ; loptind++)
	    if (strcmp(argv[loptind], "-") == 0)
		dostandardinput(cppcommand);
	    else
		process(cppcommand,argv[loptind]);
	dumptree(argc,argv);
	}
}

/*
    Dumptree() lists out the calling stacks. All names will be listed out
    unless some function names are specified in -f options.
*/

dumptree(argc,argv)
int argc;
char **argv;
{
    register int c;
    register struct rname *startp;

    if (functionlist)
	{
	/* restart argument list and only print functions listed */
	for (optind = 1 ; (c = getopt (argc, argv, arglist)) != EOF ; )
	    if (c == 'f')
		if (startp = lookfor(optarg))
		    {
		    output (startp, 0);
		    printf ("\n\n");
		    }
		else
		    (void) fprintf (stderr,
			"%s: *** error *** function '%s' not found\n",
			progname, optarg);
	}
    else
	/* output everything */
	for (startp = namelist ; startp->namer[0] ; startp++)
	    if (!startp->rnamecalled)
		{
		output (startp, 0);
		printf ("\n\n");
		}
}

#define BACKSLASH '\\'
#define QUOTE '\''

/*
    getfunc() returns the name of a function in atom and
    0 for a definition, 1 for an internal call
*/

getfunc(atom)
char atom[];
{
    register int c, ss;

    for ( ; ; )
	if (isalpha(c = getc(input)) || (c == '_'))
	    {
	    ungetc(c,input);
	    scan(atom);
	    continue;
	    }
	else
	    switch(c)
		{
		case '\t':		/* white space */
		case ' ':
		case '\n':
		case '\f':
		case '\r':
		    continue;
		case '#':		/* eat C compiler line control info */
					/* CPP output will not span lines */
		    while ((c= getc(input)) != '\n')
			;
		    ungetc(c,input);
		    continue;
		case QUOTE:		/* character constant */
		    atom[0]='\0';
		    while ((c= getc(input)) != QUOTE)
			if (c == BACKSLASH)
			    getc(input);
		    continue;
		case '\"':		/* string constant */
		    while (( c = getc(input)) != '\"')
			if (c==BACKSLASH)
			    getc(input);
		    continue;
		case BACKSLASH:		/* ? why is this here ? */
		    atom[0] = '\0';
		    getc(input);
		    continue;
		case '{':		/* start of a block */
		    bracket++;
		    atom[0]='\0';
		    continue;
		case '}':		/* end of a block */
		    --bracket;
		    if (bracket < 0)
			(void) fprintf (stderr, "%s: bracket underflow!\n",
			    progname);
		    atom[0]='\0';
		    continue;
		case '(':		/* parameter list for function? */
		    if( ! atom[0] )
			continue;
		    if (!checksys(atom)) {
			if (!bracket)
			    if (checkinternal())
				return (0);
			    else
				continue;
			if ((ss = seen(atom)) == -1)
			    (void) fprintf(stderr, "%s: aseen overflow!\n",
				progname);
			if (bracket && !ss)
			    return (1);
		    }
		    atom[0]='\0';
		    continue;
		case EOF:		/* end of file */
		    return (-1);
		case '/':		/* comment? */
		    if (( c = getc(input))=='*')
			for (;;)
			    {
			    while (getc(input) != '*')
				;
			    if ((c = getc(input)) == '/')
				break;
			    ungetc(c,input);
			    }
		    else
			ungetc(c,input);
		    continue;
		case ')':		/* end of parameter list */
		default:
		    atom[0]='\0';
		    continue;
		}
}

/*
    Skipblanksandcomments() skips past any blanks and comments
    in the input stream.
*/

skipblanksandcomments()
{
    register int c;

    for (c = getc(input);
	 (c == ' ') || (c == '\t') ||
	 (c == '\n') || (c == '\r') || (c == '\b') || (c == '\f') ||
	 (c == '/');
	 c = getc(input))
	if (c == '/')
	    if ((c = getc(input)) == '*')	/* start of comment? */
		for (;;)
		    {
		    while (getc(input) != '*')
			;
		    if ((c = getc(input)) == '/')
			break;
		    ungetc(c,input);
		    }
	    else
		{
		ungetc(c,input);
		return;
		}
    ungetc(c,input);
    return;
}

/*
    checkinternal differentiates between an external declaration and
    a real function definition. For instance, between:

	extern char *getenv(), *strcmp();

    and

	char *getenv(name)
	char *name;
	{}

    It does it by making the two observations that nothing (except blanks and
    comments) can be between the parentheses of external calls nor between the
    right parenthesis and the semi-colon or comma following the definition.
    If the proposed ANSI standard is accepted, the first observation will no
    longer be valid. We can still use the second observation, however. The 
    code will have to be changed at that point.
*/

checkinternal()
{
    register int c;

    skipblanksandcomments();		/* skip blanks between parens */
    c = getc(input);
    if (c != ')')
	{
	ungetc(c,input);
	return 1;
	}
    skipblanksandcomments();		/* skip blanks between paren and ; */
    c = getc(input);
    if (c == ';' || c == ',')
	return 0;
    ungetc(c,input);
    return 1;
}

/*
    scan text until a function name is found
*/

scan (atom)
char atom[];
{
    register int c, i = 0;

    for (c = getc(input);
	 (i < ATOMLENGTH) && isascii(c) &&
	 ( isalpha(c) || isdigit(c) || (c == '_') );
	 c = getc(input))
	atom [i++] = c;
    if (i == ATOMLENGTH)
	atom [i-1] = '\0';
    else
	atom [i] = '\0';
    while( isascii(c) && ( isalpha(c) || isdigit(c) || (c == '_') ))
	    c = getc(input);
    ungetc(c,input);
}

/*
    checksys returns 1 if atom is a system keyword, else 0
*/

checksys (atom)
char atom[];
{
    register int i;

    for (i=0; sysword[i] ; i++)
	if (strcmp(atom,sysword[i]) == 0)
	    return (1);
    return (0);
}

/*
    see if we have seen this function within this process
*/

seen (atom)
char *atom;
{
    register int i, j;

    for (i=0; aseen[i][0] && i < MAXSEEN ; i++)
	if (strcmp (atom, aseen[i]) == 0)
	    return (1);
    if (i >= MAXSEEN)
	return (-1);
    for (j=0; (aseen[i][j] = atom[j]) != '\0' && j < ATOMLENGTH ; j++)
	;
    aseen[i+1][0] = '\0';
    return (0);
}

/*
    When scanning the text each function instance is inserted into a
    linear list of names, using the rname structure, when it is first
    encountered. It is also inserted into the linked list using the rinst
    structure. The entry into the name list has a pointer to the defining
    instance in the linked list, and each entry in the linked list has
    a pointer back to the relevant name. Newproc makes an entry in the
    defining list, which is distinguished from the called list only because
    it has no calledby link (value=0). Add2proc enters into the called
    list, by inserting a link to the new instance in the calls pointer of
    the last entry (may be a defining instance, or a function called by
    that defining instance), and points back to the defining instance of
    the caller in its called-by pointer.
*/

struct rinst *
newproc (name)
char name[];
{
    aseen[0][0] = '\0';
    return (install(place(name),(struct rinst *)0));
}

/*
    add the function name to the calling stack of the current function.
*/

add2call (name,curp)
char name[];
struct rinst *curp;
{
    register struct rname *p;
    register struct rinst *ip;

    p = place (name);
    ip = install (p, curp);
    if (p && (strcmp(p->namer,curp->namep->namer) != 0))
	p->rnamecalled = 1;
    return (ip != (struct rinst *) 0);
}

/*
    place() returns a pointer to the name on the namelist.
    If the name was not there, it puts it at the end of the list.
    If there was no room, it returns -1.
*/

struct rname *
place (name)
char name[];
{
    register int i, j;
    register struct rname *npt;

    for (i = 0 ; (npt = &namelist[i])->namer[0] && i<MAXNAME ; i++)
	{
	if (strcmp(name,npt->namer) == 0)
	    return (npt);
	if (i >= MAXNAME)
	    {
	    (void) fprintf (stderr, "%s: namelist overflown!\n", progname);
	    return ( (struct rname *) -1);
	    }
	}

    /* name was not on list, so put it on */
    for (j=0 ; name[j]; j++)
	npt->namer[j] = name[j];
    npt->namer[j] = '\0';
    (npt+1)->namer[0] = '\0';
    npt->rnamecalled = 0;
    npt->rnameout=0;
    return (npt);
}

/*
    install (np,rp) puts a new instance of a function into the linked list.
    It puts a pointer (np) to its own name (returned by place) into its
    namepointer, a pointer to the calling routine (rp) into
    its called-by pointer, and zero into the calls pointer. It then
    puts a pointer to itself into the last function in the chain.
*/

struct rinst *
install (np,rp)
struct rname *np;
struct rinst *rp;
{
    register struct rinst *newp;
    register struct rinst *op;

    if (!np)
	return ( (struct rinst *) -1);
    if ( !(newp = getfree()))
	return ( (struct rinst *) 0);
    newp->namep = np;
    newp->calls = 0;
    if (rp)
	{
	op = rp;
	while (op->calls) op = op->calls;
	newp->calledby = op->calledby;
	op->calls = newp;
	}
    else
	{
	newp->calledby = (struct rinst *) np;
	np->dlistp = newp;
	}
    return (newp);
}

/*
    getfree returns a pointer to the next free instance block on the list
*/

struct rinst *
getfree()
{
    register struct rinst *ret;

    ret = frp;
    if (!ret)
	(void) fprintf (stderr, "%s: out of instance blocks!\n", progname);
    frp=frp->calls;
    return (ret);
}

/*
    Initfree makes a linked list of instance blocks. It is called once,
    at the beginning of the programme, and between files if the -s option
    is specified.
*/

initfree()
{
    register int i;
    register struct rinst *rp = dlist;

    for (i = 0 ; i < MAXINST-2 ; i++)
	{
	rp->namep = 0;
	rp->calls = rp+1;
	(rp+1)->calledby = rp;
	rp++;
	}
    rp->namep=0;
    rp->calls = 0;
}

/*
    output is a recursive routine which is supposed to print one tab for each
    level of recursion, then the name of the function called, followed by the
    next function called by the same higher level routine. In doing this, it
    calls itself to output the name of the first function called by the
    function whose name it is outputting. It maintains an active list of
    functions currently being output by the different levels of recursion,
    and if it finds itself asked to output one which is already active,
    it terminates, marking that call with a '*'.
*/

output (func,tabc)
struct rname *func;
int tabc;
{
    register int i, tabd, tabstar, tflag;
    struct rinst *nextp;

    ++linect;
    printf ("\n%d", linect);
    if (!makeactive(func))
	printf ("   * nesting is too deep"); /* calls nested too deep */
    else
	{
	tabstar= 0;
	tabd = tabc;
	for ( ; tabd > ntabs; tabstar++)
	    tabd = tabd - ntabs;
	if (tabstar > 0)
	    {
	    printf ("  ");
	    for (i = 0 ; i < tabstar ; i++ )
		printf ("<");
	    }
	if (tabd == 0)
	    printf ("   ");
	else
	    for (i = 0 ; i < tabd ; i++ )
		printf ("\t");
	if (active(func))
	    printf ("<<< %s",func->namer); /* recursive call */
	else
	    {
	    if (func->dlistp)
		{
		printf ("%s", func->namer);
		nextp = func->dlistp->calls;
		if (!terse || !func->rnameout)
		    {
		    ++tabc;
		    if (!func->rnameout)
			func->rnameout = linect;
		    if (tabc > ntabs && tabc%ntabs==1 && nextp)
			{
			printf("\n%s", dashes);
			tflag = 1;
			}
		    else
			tflag = 0;
		    for ( ; nextp; nextp = nextp->calls)
			output (nextp->namep, tabc);
		    if (tflag)
			{
			printf("\n%s", dashes);
			tflag = 0;
			}
		    }
		else if (nextp)
		    printf (" ... [see line %d]", func->rnameout);
		}
	    else
		printf ("%s [external]",func->namer); /* library or external call */
	    }
	backup ();
	}
    return;
}

/*
    makeactive simply puts a pointer to the nameblock into a stack with
    maximum depth MAXDEPTH. the error return only happens for stack overflow.
*/

makeactive (func)
struct rname *func;
{
    if (activep < MAXDEPTH)
	{
	activelist[activep] = func;
	activep++;
	return (1);
	}
    return (0);
}

/*
    backup removes an item from the active stack
*/

backup()
{
    if (activep)
	activelist [activep--] = 0;
}

/*
    active checks whether the pointer which is its argument has already
    occurred on the active list, and returns 1 if so.
*/

active (func)
register struct rname *func;
{
    register int i;

    for (i = 0; i < activep-1 ; i++)
	if (func == activelist[i])
	    return (1);
    return (0);
}

/*
    lookup (name) accepts a pointer to a name and sees if the name is on the
    namelist. If so, it returns a pointer to the nameblock. Otherwise it
    returns zero. If the name from argv is > ATOMLENGTH-1, then it is
    truncated.
*/

struct rname *
lookfor(name)
register char *name;
{
    register struct rname *np;

    if (strlen(name) >= ATOMLENGTH)
	name[ATOMLENGTH] = '\0';

    for (np = namelist; np->namer[0] ; np++)
	if (strcmp (name, np->namer) == 0)
	    return (np);
    return (0);
}
