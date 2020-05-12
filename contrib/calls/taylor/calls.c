/*
 * calls: calls prints a paragraphed list of who calls who within
 * a body of source code.
 *
 * Author: M.M. Taylor, DCIEM, Toronto, Canada.
 * Modified for V7 and stdio, 22/Jan/81, Alexis Kwan (HCR at DCIEM).
 */

#include <stdio.h>
#include <ctype.h>

#define CTRL_D 004
#define ATOMLENGTH 20
#define MAXNAME 500
#define MAXINST 4000
#define MAXDEPTH 25
#define MAXSEEN 100
#define PAPERWIDTH 132


int bracket, linect, terse, ntabs;
char aseen [MAXSEEN][ATOMLENGTH];

/* if lastchar==0, getch gets next character, else getch gets lastchar */
char lastchar;
char *sysword [ ] = {
	"if",
	"while",
	"for",
	"return",
	"switch",
	0
	};


struct rname{
	char namer[ATOMLENGTH];
	int rnamecalled;
	int rnameout;
	struct rinst *dlistp;
	} namelist[MAXNAME];
struct rinst{
	struct rname *namep;
	struct rinst *calls;
	struct rinst *calledby;
	} dlist[MAXINST];

struct rname *activelist[MAXDEPTH];
int activep = 0;
struct rinst *frp;


main(argc,argv)
int argc;
char *argv[];
{
int atoi(), gf, i, ok;
char atom[ATOMLENGTH];
char *startfunc;
struct rname *startp, *lookfor();
struct rinst *curproc, *initfree(), *newproc(), *add2call();

	bracket=0;
	terse=1;
	lastchar = 0;
	aseen[0][0] = 0;
	startfunc = "main" ; /* default */
	frp = dlist;
	initfree();
	for (i = 0; i < MAXDEPTH ; i++ ) activelist[i] = 0 ;
	activep = 0;
	ntabs = (PAPERWIDTH - 20)/8;
/*  get arguments and flags:
	c	c language
	f	fortran language
	p	pascal language
	g	list globals
	t	terse form  (default case)
	terse	ditto
	v	verbose form
	verbose	ditto
	w nn	paper width  (default 132)
	width nn 	ditto
	name	function to start from
*/
	for (i = 1 ; i<argc && argv[i][0]=='-'; i++){
		if (match(argv[i],"-c")) ;
		else if (match(argv[i],"-t") || match(argv[i],"-terse")){
			terse=1;
		}
		else if (match(argv[i],"-v") || match(argv[i],"-verbose"))
			terse=0;
		else if (match(argv[i],"-changes")) printf("-changes not yet implemented\n");
		else if (match(argv[i],"-width") || match(argv[i],"-w")){
			ntabs = (atoi(argv[++i])-20)/8;
		}
		else printf ("unknown command %s", argv[i]);
		printf ("\n");
		}
	ok = 1;
	while ((gf = getfunc(atom)) != -1 && ok ){
		if (gf) ok = (int)add2call(atom,curproc);
		else ok = (int)(curproc = newproc(atom));
	}
	if (i<argc){
		do{
			startfunc = argv[i];
			if (startp = (lookfor(startfunc))){
				output (startp,0);
				printf ("\n\n");
			}
			else printf ("*** error *** %s not found\n",startfunc);
		}while (++i<argc);
	}
	else {
		for (startp=namelist; startp->namer[0] ; startp++){
			if (!startp->rnamecalled){
				output (startp,0);
				printf ("\n\n");
			}
		}
	}
}

/* return name of a function. value 0 == definition, 1 == called */
getfunc(atom)
char atom[];
{
	char c;
	int ss;
	for(;;){
		c= getch();
		if (isalpha(c)){
			lastchar=c;
			scan(atom);
			continue;
			}
		else{
			switch(c){
			case '\t':
			case ' ':
				continue;
			case '\n':
				if ((c= getch())=='#')
 					while ((c= getch()) != '\n')
 						if (c == '\\')
 							getch();
				lastchar = c;
				continue;
			case '\'':
				atom[0]='\0';
				while ((c= getch()) != '\'')
					if (c == '\\') getch();
				continue;
			case '\"':
				while (( c = getch()) != '\"')
					if (c=='\\') getch();
				continue;
			case '\\':
				atom[0] = '\0';
				getch();
				continue;
			case '{':
				bracket++;
				atom[0]='\0';
				continue;
			case '}':
				--bracket;
				if (bracket < 0)
					printf ("bracket underflow");
				atom[0]='\0';
				continue;
			case '(':
				if( ! atom[0] ) continue;
				if (!checksys(atom)) {
					if (!bracket) return (0);
					if ((ss=seen(atom))==-1)printf("aseen overflow");
					if (bracket && !ss) return (1);
				}
				atom[0]='\0';
				continue;
			case ')':
				atom[0]='\0';
				continue;
			case CTRL_D:
			case EOF:
				return (-1);
			case '/':
				if (( c = getch())=='*'){
					for (;;){
						while (getch() != '*');
						if ((c = getch()) == '/') break;
						lastchar = c;
						}
					continue;
				}
				else {
					lastchar = c;
					continue;
				}
			case '*':
				atom [0] = '\0';
				continue;
			default:
				atom[0]='\0';
				continue;
			}
		}
	}
}

/* scan text until an atom that could be a function name is found */
scan (atom)
char atom[];
{
	char c;
	int i;

	c = lastchar;
	i = 0;
	while( isalpha(c) || isdigit(c) || (c == '_') ){
		atom [i++] = (c=getch());
		if (i == ATOMLENGTH ) break;
		}
	atom [i-1] = '\0';
	while( isalpha(c) || isdigit(c) || (c == '_') ) c = getch();
	lastchar= c;
	return (1);
}

/* return value 1 if atom is a system keyword, else 0 */
checksys (atom)
char atom[];
{
int i;
	for (i=0; sysword[i] ; i++)
		if (match(atom,sysword[i])) return (1);
	return (0);
}

/*** see if we have seen this function within this process ***/

seen (atom)
char *atom;
{
int i,j;
	for (i=0; aseen[i][0] && i < MAXSEEN ; i++){
		if (match (atom, aseen[i])) return (1);
	}
	if (i >= MAXSEEN) return (-1);
	for (j=0; (aseen[i][j] = atom[j]) != '\0' && j < ATOMLENGTH ; j++) ;
	aseen[i+1][0] = '\0';
	return (0);
}
/* return 1 if strings match, else 0 */
match (atom,name)
char name[];
char atom[];
{
	register char *ap,*np;

	ap = atom;
	np = name;
	while (*ap==*np){
		if (*ap== 0) return (1);
		ap++; np++;
	}
	return (0);
}


/* get a character, but perhaps return previous one instead */
getch()
{
char c;
	if( lastchar == 0 )
		{
		c = getchar();
		return(c);
		}
	c = lastchar;
	lastchar = 0;
	return (c);
}

/*
when scanning the text each function instance is inserted into a
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
struct rinst *install();
struct rname *place();

	aseen[0][0] = '\0';
	return (install(place(name),(struct rinst *)0));
}

struct rinst *
add2call (name,curp)
char name[];
struct rinst *curp;
{
struct rinst *install(),*ip;
struct rname *place(),*p;


	ip=install (p=place (name),curp);
	if (p) ++(p->rnamecalled);
	return (ip);
}

/**
place (name)  returns a pointer to the name on the namelist.
	If the name was not there, it puts it at the end of the list.
	If there was no room, it returns -1.
**/

struct rname *
place (name)
char name[];
{
int i,j;
struct rname *npt;
	for (i = 0 ; (npt = &namelist[i])->namer[0] && i<MAXNAME ; i++){
		if (match(name,npt->namer)){
			return (npt);
		}
		if (i >= MAXNAME){
			printf ("namelist overflown");
			return ( (struct rname *) -1);
		}
/* name was not on list, so put it on */
		}

	for (j=0 ; name[j]; j++)
		npt->namer[j] = name[j];
	npt->namer[j] = '\0';
	(npt+1)->namer[0] = '\0';
	npt->rnamecalled = 0;
	npt->rnameout=0;
	return (npt);
}

/**
install (np,rp) puts a new instance of a function into the linked list.
	It puts a pointer (np) to its own name (returned by place) into its
	namepointer, a pointer to the calling routine (rp) into
	its called-by pointer, and zero into the calls pointer. It then
	puts a pointer to itself into the last function in the chain.
**/

struct rinst *
install (np,rp)
struct rname *np;
struct rinst *rp;
{
struct rinst *newp;
struct rinst *op;
struct rinst *getfree();
	if (!np) return ( (struct rinst *) -1);
	if ( !(newp = getfree())) return ( (struct rinst *) 0);
	newp->namep = np;
	newp->calls = 0;
	if (rp){
		op = rp;
		while (op->calls) op = op->calls;
		newp->calledby = op->calledby;
		op->calls = newp;
	}
	else {
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
struct rinst *ret;
	ret=frp;
	if (!ret) printf ("out of instance blocks\n");
	frp=frp->calls;
	return(ret);
}

/*
initfree makes a linked list of instance blocks, and returns a pointer
to the first one. it is called only once, at the beginning of the programme.
*/

struct rinst *
initfree()
{
int i;
	for (i = 0 ; i < MAXINST-2 ; i++) {
		frp->namep = 0;
		frp->calls = frp+1;
		(frp+1)->calledby = frp;
		frp++;
	}
	frp->namep=0;
	frp->calls = 0;
	frp= dlist;
	return(dlist);
}

/********
output is a recursive routine which is supposed to print one tab for each
level of recursion, then the name of the function called, followed by the
next function called by the same higher level routine. In doing this, it
calls itself to output the name of the first function called by the
function whose name it is outputting. It maintains an active list of
functions currently being output by the different levels of recursion,
and if it finds itself asked to output one which is already active,
it terminates, marking that call with a * .
*********/

output (func,tabc)
struct rname *func;
int tabc;
{
struct rinst *nextp;
int i, tabd, tabstar, tflag;

	++linect;
	printf ("\n%d  ",linect);
	if (!(makeactive(func))) printf ("*"); /* calls nested too deep */
	else {
		tabstar= 0;
		tabd = tabc;
		for (;tabd >ntabs; tabstar++) tabd = tabd-ntabs;
		for (i = 0 ; i < tabstar ; i++ ) printf ("<");
		printf (" ");
		for (i = 0 ; i < tabd ; i++ ) printf ("\t");
		if (active(func)) printf ("^ %s ^",func->namer); /* recursive call */
		else {
			if (func->dlistp){
				printf ("%s", func->namer);
				nextp = func->dlistp->calls;
				if (!terse || !func->rnameout) {
					++tabc;
					if (!func->rnameout) func->rnameout = linect;
					if (tabc > ntabs && tabc%ntabs==1 && nextp){
						printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
						tflag = 1;
					}
					else tflag = 0;
					for (;nextp; nextp = nextp->calls){
							output (nextp->namep, tabc);
					}
					if (tflag){
						printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
						tflag = 0;
					}
				}
				else if (nextp)
					printf (" ... [see line %d]", func->rnameout);
			}
			else printf ("%s [ext]",func->namer); /* library or external call */
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
	if (activep < MAXDEPTH){
		activelist[activep] = func;
		activep++;
		return (1);
	}
	else return (0);
}

/*
backup removes an item from the active stack
*/

backup()
{
	if (activep){
		activelist [activep--] = 0;
		return (1);
	}
	else return (-1);
}

/*
active checks whether the pointer which is its argument has already
occurred on the active list, and returns 1 if so.
*/

active (func)
struct rname *func;
{
int i;
	for (i = 0; i < activep-1 ; i++){
		if (func == activelist[i]) return (1);
	}
	return (0);
}

/* lookup (name) accepts a pointer to a name and sees if the name is on the
namelist. If so, it returns a pointer to the nameblock. Otherwise
returns zero.
*/

struct rname *
lookfor(name)
char *name;
{
struct rname *np;
	for (np = namelist; np->namer[0] ; np++){
		if (match (name, np->namer)) return (np);
	}
	return (0);
}
