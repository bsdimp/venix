#include <signal.h>
#undef NSIG                     /* to prevent redefine errors in <param.h> */
#include <sgtty.h>
#include <a.out.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/filsys.h>
#include <sys/devparm.h>
#include <sys/xtblk0.h>

#define	maximum(a,b)	(((a) > (b)) ? (a) : (b))
#define	minimum(a,b)	(((a) < (b)) ? (a) : (b))

#define DEL             ('H' & 037)     /* delete key */
#define	TMPMAX		35
#define	DOS12FAT	2	/* dos version to use 12 bit fat */
#define	DOS16FAT	3	/* dos version to use 16 bit fat */

#ifdef	MAIN_LINE

/*
	The defualt sizes are for 10MB drive.
	Do not change these numbers, since they constitute
	the smallest VENIX system.  By answering the installation
	prompts these sizes can by dynamically increased.
*/
int	ssize   = 2980;		/* default sys area size */
int	swapsiz = 750;		/* default swap size */
int	tblsize = 10;		/* default internal table area size */
int	tsize   = 340;		/* default tmp area size */
int	usize   = 16660;	/* default user area size */
int	dsize   = 0;		/* default dos area size */
int	swapint = 50;		/* default swap interval size */
int	maxfer  = 16*1024;	/* max single transfer size, don't change */

/*
	The following variables are defined here only for documentation.
	They are recalculated at run time.
	However, don't change sysmin and tmpmin since they represent
	the smallest possible file system sizes.
*/
int	sysmin  = 55;		/* min system size in cylinders, don't change */
int	sysmax  = 321;		/* max system size in cylinders */
int	tmpmin  = 5;		/* min temp   size in cylinders, don't change */
int	tmpmax  = TMPMAX;	/* max temp   size in cylinders */
int	usrmin  = 0;		/* min usr    size in cylinders */
int	usrmax  = 245;		/* max usr    size in cylinders */

/*
	Miscellaneous variables.
*/
int	dsysmin  = 0;		/* hold minimum # of cylinders for sys area */
int	dtmpmin  = 0;		/* hold minimum # of cylinders for tmp area */
int	dusrmin  = 0;		/* hold minimum # of cylinders for usr area */
int	dssize   = 0;		/* hold original values for calculations    */
int	dswapsiz = 0;		/* hold original values for calculations    */
int	sysyes   = 0;		/* is the SYS area defined		    */
int	dosversion = DOS16FAT;	/* dos version to use 16 bin fat	    */
int	doscutoff = 20000;	/* minimum blocks to use 16 bin fat 	    */
int	totcyl   = 0;
int	tmpoff   = 0;

char	devname[16] = "/dev/wX.phy";	/* build area for drive names	    */
char	rdevname[16] = "/dev/rwX.phy";	/* build area for drive names	    */
char	clear[] = "\033H\033J\033\016";	/* clear and reset screen 	    */
char	hlon[]  = "\033\006";		/* highlight on			    */
char	hloff[] = "\033\005";		/* highlight off		    */
char	rvon[]  = "\033\010";		/* reverse video on		    */
char	rvoff[] = "\033\007";		/* reverse video off		    */
char	*getstr();			/* get a string from stdin	    */

struct	xp xp;				/* partition definitions	    */
struct	xh *xh;				/* block 0 header structure	    */
struct	sgttyb sgbuf;

#else	MAIN_LINE

extern	int	ssize;		/* default sys area size		*/
extern	int	swapsiz;	/* default swap size			*/
extern	int	tblsize;	/* default internal table area size	*/
extern	int	tsize;		/* default tmp area size		*/
extern	int	usize;		/* default user area size		*/
extern	int	dsize;		/* default dos area size		*/
extern	int	swapint;	/* default swap interval size		*/
extern	int	sysmax;		/* max system size in cylinders		*/
extern	int	maxfer;		/* max single transfer size, don't change     */
extern	int	sysmin;		/* min system size in cylinders, don't change */
extern	int	tmpmin;		/* min temp   size in cylinders, don't change */
extern	int	tmpmax;		/* max temp   size in cylinders		*/
extern	int	usrmin;		/* min usr    size in cylinders		*/
extern	int	usrmax;		/* max usr    size in cylinders		*/
extern	int	dsysmin;	/* hold minimum # of cylinders for sys area */
extern	int	dtmpmin;	/* hold minimum # of cylinders for tmp area */
extern	int	dusrmin;	/* hold minimum # of cylinders for usr area */
extern	int	dssize;		/* hold original values for calculations    */
extern	int	dswapsiz;	/* hold original values for calculations    */
extern	int	sysyes;		/* is the SYS area defined		*/
extern	int	dosversion;	/* dos version to use 16 bin fat	*/
extern	int	doscutoff;	/* minimum blocks to use 16 bin fat	*/
extern	int	totcyl;
extern	int	tmpoff;
extern	long	lseek();
extern	char	devname[];	/* build area for drive names		*/
extern	char	rdevname[];	/* build area for drive names		*/
extern	char	clear[];	/* clear and reset screen		*/
extern	char	hlon[];		/* highlight on				*/
extern	char	hloff[];	/* highlight off			*/
extern	char	rvon[];		/* reverse video on			*/
extern	char	rvoff[];	/* reverse video off			*/
extern	char	*getstr();	/* get a string from stdin		*/
extern	struct	xp xp;		/* partition definitions		*/
extern	struct	xh *xh;		/* block 0 header structure		*/
extern	struct	sgttyb sgbuf;

#endif	MAIN_LINE
