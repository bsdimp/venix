#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "debug.hh"

#define printf(...)

class Venix : public MachineOS
{
	static const int HOST_MAXPATHLEN = 1025;

	// errno for Venix == Errno for FreeBSD <= 34
	static const int VENIX_EDEADLOCK  = 40;


	// tty ioctls from Venix
	static const int VENIX_TIOCGETD  = (('t'<<8)|0);	/** % get line disipline */
	static const int VENIX_TIOCSETD  = (('t'<<8)|1);	/** % set line disipline */
	static const int VENIX_TIOCHPCL  = (('t'<<8)|2);	/* hangup on close */
	static const int VENIX_TIOCGETP  = (('t'<<8)|8);	/* get current parameter */
	static const int VENIX_TIOCSETP  = (('t'<<8)|9);	/* set parameters */
	static const int VENIX_TIOCSETN  = (('t'<<8)|10);	/* set parameters without flush */
	static const int VENIX_TIOCEXCL  = (('t'<<8)|13);	/* set exclusive use */
	static const int VENIX_TIOCNXCL  = (('t'<<8)|14);	/* clear exclusive use */
	static const int VENIX_TIOCFLUSH = (('t'<<8)|16);	/* flush i/o */
	static const int VENIX_TIOCSETC  = (('t'<<8)|17);	/** % set special characters */
	static const int VENIX_TIOCGETC  = (('t'<<8)|18);	/** % get special characters */
	static const int VENIX_TIOCQCNT	 = (('t'<<8)|30);	/* get char counts on i/o queues */
	static const int VENIX_AIOCWAIT	 = (('a'<<8)|0);	/* wait/test outstanding requests */
	/* FIO and DIO features not supported */

	static const int VENIX_B0        = 0;
	static const int VENIX_B50       = 1;
	static const int VENIX_B75       = 2;
	static const int VENIX_B110      = 3;
	static const int VENIX_B134      = 4;
	static const int VENIX_B150      = 5;
	static const int VENIX_B200      = 6;
	static const int VENIX_B300      = 7;
	static const int VENIX_B600      = 8;
	static const int VENIX_B1200     = 9;
	static const int VENIX_B1800     = 10;
	static const int VENIX_B2400     = 11;
	static const int VENIX_B4800     = 12;
	static const int VENIX_B9600     = 13;
	static const int VENIX_EXTA      = 14; /* Not supported on Venix */
	static const int VENIX_EXTB      = 15; /* Not supported on Venix */
	static const int VENIX_TANDEM    = 0001;
	static const int VENIX_CBREAK    = 0002;
	static const int VENIX_LCASE     = 0004;
	static const int VENIX_ECHO      = 0010;
	static const int VENIX_CRMOD     = 0020;
	static const int VENIX_RAW       = 0040;
	static const int VENIX_ODDP      = 0100;
	static const int VENIX_EVENP     = 0200;
	static const int VENIX_ANYP      = 0300; /* All DELAY flags ignored, not reproduced here */

	static const int VENIX_NOFILE = 20;	// Max files per process on Venix (from sys/param.h, so maybe tunable)
	static const int VENIX_PATHSIZ = 256;
	int open_fd[VENIX_NOFILE];

	static const int VENIX_NSIG = 17;
	static const int VENIX_SIG_DFL = 0;
	static const int VENIX_SIG_IGN = 1;
	int venix_sighandle[VENIX_NSIG];

	int magic;
	int stack;

	static const int VENIX_MAXPROC=30000;
	static const int VENIX_NPROC=30;
	pid_t pid_host[VENIX_NPROC];
	typedef short venix_pid_t;
	venix_pid_t pid_venix[VENIX_NPROC];
	venix_pid_t pid;

	/* XXX need to translate errno */
	static const int VENIX_EAGAIN = 11;	/* Otherwise they are 1:1 on FreeBSD */
public:

Venix() : brk(0), length(0), scall(0) {
	/*
	 * Initialize our file descriptor table. On old-school Unix, like v7/Venix,
	 * there's 20 fd's per process. This is theoretically changeable in sys/param.h
	 * but it was rare that this was adjusted due to too many other matching constants
	 * that needed changing. So for this emulator, we'll just do the 20, even though
	 * we have this #define. Usually there will be a 1:1 match between our fd's and
	 * the emulator's. Just to be safe, go ahead and have a mapping in case it doesn't
	 * work out that way.
	 */
	open_fd[0] = 0;
	open_fd[1] = 1;
	open_fd[2] = 2;
	for (int i = 3; i < VENIX_NOFILE; i++)
		open_fd[i] = -1;

	/*
	 * Initialize the signal maps to the default
	 */
	for (int i = 0; i < VENIX_NSIG; i++)
		venix_sighandle[i] = VENIX_SIG_DFL;
	pid = 2;
	pid_host[0] = getpid();
	pid_venix[0] = pid++;
	for (int i = 1; i < VENIX_NPROC; i++)
		pid_host[i] = -1;
}

~Venix() {
	for (int i = 3; i < VENIX_NOFILE; i++) {
		if (open_fd[i] != -1) {
			close(open_fd[i]);
			open_fd[i] = -1;
		}
	}
}

private:
	uint16_t brk;
	int length;
	Word scall;
	static const int OMAGIC = 0407;		// Old impure format (TINY SS = CS = DS = ES)
	static const int NMAGIC = 0411;		// I&D fprmat (CS and SS = DS == ES)

	/* Basic types */
	typedef	int16_t		venix_daddr_t;	/* disk address */
	typedef	int16_t		venix_ino_t;	/* i-node number */
	typedef	int32_t		venix_time_t;	/* a time */
	typedef	int16_t		venix_dev_t;	/* device code */
	typedef	int32_t		venix_off_t;	/* offset */

	/*
	 * Header prepended to each a.out file.
	 */
	struct venix_exec {
		int16_t		a_magic;	/* magic number */
		uint16_t	a_stack;	/* size of stack if Z type, 0 otherwise */
		int32_t		a_text;		/* size of text segment */
		int32_t		a_data;		/* size of initialized data */
		int32_t		a_bss;		/* size of uninitialized data */
		int32_t		a_syms;		/* size of symbol table */
		int32_t		a_entry;	/* entry point */
		int32_t		a_trsize;	/* size of text relocation */
		int32_t		a_drsize;	/* size of data relocation */
	};

	/*
	 * ftime return structure
	 */
	struct venix_timeb {
		venix_time_t	time;
		uint16_t	millitm;
		int16_t		timezone;
		int16_t		dstflag;
	} __packed;
	static_assert(sizeof(venix_timeb) == 10, "Bad venix_timeb size");

	/*
	 * tty ioctl
	 */
	struct	venix_sgttyb {
		int8_t		sg_ispeed;
		int8_t		sg_ospeed;
		int8_t		sg_erase;
		int8_t		sg_kill;
		int16_t		sg_flags;
	} __packed;
	static_assert(sizeof(venix_sgttyb) == 6, "Bad venix_sgttyb size");

	struct venix_stat
	{
		venix_dev_t	st_dev;
		venix_ino_t	st_ino;
		uint16_t	st_mode;
		int16_t		st_nlink;
		int16_t		st_uid;
		int16_t		st_gid;
		venix_dev_t	st_rdev;
		venix_off_t	st_size;
		venix_time_t	st_atime_v;
		venix_time_t	st_mtime_v;
		venix_time_t	st_ctime_v;
	} __packed;
	static_assert(sizeof(venix_stat) == 30, "bad venix_stat size");

#define	VENIX_S_IFMT	0160000		/* type of file */
#define		VENIX_S_IFDIR	0140000	/* directory */
#define		VENIX_S_IFCHR	0120000	/* character special */
#define		VENIX_S_IFBLK	0160000	/* block special */
#define		VENIX_S_IFREG	0100000	/* regular file */
#define	VENIX_S_ILRG	010000	/* large file */
#define	VENIX_S_ISUID	004000	/* set user id on execution */
#define	VENIX_S_ISGID	002000	/* set group id on execution */
#define	VENIX_S_ISVTX	001000	/* save shared segment, even after use */
#define	VENIX_S_IREAD	000400	/* read permission, owner */
#define	VENIX_S_IWRITE	000200	/* write permission, owner */
#define	VENIX_S_IEXEC	000100	/* execute permission, owner */

void venix_to_host_path(char *fn, char *host_fn, size_t len)
{
	strlcpy(host_fn, fn, len);
}

Word callno() { return bx(); }
Word arg1() { return ax(); }
Word arg2() { return dx(); }
Word arg3() { return cx(); }
Word arg4() { return si(); }

int copyinstr(Word uptr, void *kaddr, size_t len)
{
	char ch;
	char *str = (char *)kaddr;

	for (int i = 0; i < len; i++) {
		ch = readByte(uptr + i, DSeg);
		str[i] = ch;
		if (ch == '\0')
			break;
	}

	return (0);
}

int copyin(Word uptr, void *kaddr, size_t len)
{
	char ch;
	char *str = (char *)kaddr;

	for (int i = 0; i < len; i++) {
		ch = readByte(uptr + i, DSeg);
		str[i] = ch;
	}

	return (0);
}

/*
 * Copy data from the 'kernel' kptr to 'userland' uptr for len bytes.
 */
int copyout(void *kptr, Word uptr, size_t len)
{
	uint8_t *p = reinterpret_cast<uint8_t *>(kptr);

	for (int i = 0; i < len; i++) {
		writeByte(p[i], uptr + i, DSeg);
	}
	return 0;
}

int copyinfn(Word uptr, char *hostfn, size_t len)
{
	char fn[VENIX_PATHSIZ];

	if (copyinstr(uptr, fn, sizeof(fn)) != 0) {
		sys_error(EFAULT);
		return 1;
	}
	venix_to_host_path(fn, hostfn, len);
	return 0;
}

mode_t venix_mode_to_host(uint16_t vmode)
{
	return vmode;
}

uint16_t venix_host_to_mode(mode_t hmode)
{
	uint16_t vmode;

	vmode = hmode & 07777;
	/* No clue what S_ILRG is "large addressing algorithm" ??? */
	switch (hmode & S_IFMT) {
	case S_IFDIR:
		vmode |= VENIX_S_IFDIR;
		break;
	case S_IFCHR:
		vmode |= VENIX_S_IFCHR;
		break;
	case S_IFBLK:
		vmode |= VENIX_S_IFBLK;
		break;
	case S_IFREG:
		vmode |= VENIX_S_IFREG;
		break;
	}
	return vmode;
}

int venix_o_to_host(int mode)
{
	return (mode);
}

int venix_host_to_speed(speed_t s)
{
	static struct {
		int host;
		int venix;
	} speeds[] = {
		{ B0, VENIX_B0 },
		{ B50, VENIX_B50 },
		{ B75, VENIX_B75 },
		{ B110, VENIX_B110 },
		{ B134, VENIX_B134 },
		{ B150, VENIX_B150 },
		{ B200, VENIX_B200 },
		{ B300, VENIX_B300 },
		{ B600, VENIX_B600 },
		{ B1200, VENIX_B1200 },
		{ B1800, VENIX_B1800 },
		{ B2400, VENIX_B2400 },
		{ B4800, VENIX_B4800 },
		{ B9600, VENIX_B9600 },
	};

	for (int i = 0; i < nitems(speeds); i++)
		if (speeds[i].host == s)
			return speeds[i].venix;

	return VENIX_B9600;
}

int venix_host_to_tc_flags(struct termios *attr)
{
	int f = 0;

	if (attr->c_iflag & IXOFF)
		f |= VENIX_TANDEM;
	if ((attr->c_iflag & (BRKINT | IXON | IMAXBEL)) ||
	    (attr->c_lflag & (ISIG | IEXTEN)) ||
	    (attr->c_oflag & OPOST))
		f |= VENIX_CBREAK;
	f |= VENIX_LCASE;
	if (attr->c_lflag & ECHO)
		f |= VENIX_ECHO;
	if (attr->c_iflag & ICRNL)
		f |= VENIX_CRMOD;
	if (!(attr->c_lflag & ICANON))
		f |= VENIX_RAW;
	if (attr->c_cflag & PARODD)
		f |= VENIX_ODDP;
	else if (attr->c_cflag & PARENB)
		f |= VENIX_EVENP;
	return (f);
}

void sys_retval_long(uint32_t r)
{
	if (r == (uint32_t)-1) {
		sys_error(errno);
	} else {
		setCX(0);			// No errno
		setAX(r & 0xffff);
		setDX(r >> 16);
	}
}

void sys_retval_int(uint16_t r)
{
	if (r == (uint16_t)-1) {
		sys_error(errno);
	} else {
		setCX(0);			// No errno
		setAX(r);
	}
}


void sys_error(int e)
{
	setCX(e);		// cx = errno
	setAX(0xffff);		// return -1;
}

bool bad_addr(Word addr)
{
	if (stack == 0 && addr > sp())
		return false;
	return (addr >= brk);
}

bool bad_fd(int fd)
{
	return (fd < 0 || fd >= VENIX_NOFILE || open_fd[fd] == -1);
}

void *u2k(Word addr) { return (&ram[physicalAddress(addr, DSeg, false)]); }

void host_to_venix_sb(struct stat *sb, Word usb)
{
	struct venix_stat vsb;

	vsb.st_dev = sb->st_dev;
	vsb.st_ino = sb->st_ino;
	vsb.st_mode = venix_host_to_mode(sb->st_mode);
	vsb.st_nlink = sb->st_nlink;
	vsb.st_uid = sb->st_uid;
	vsb.st_gid = sb->st_gid;
	vsb.st_rdev = sb->st_rdev;
	vsb.st_size = sb->st_size;
	vsb.st_atime_v = sb->st_atime;
	vsb.st_mtime_v = sb->st_mtime;
	vsb.st_ctime_v = sb->st_ctime;
	copyout(&vsb, usb, sizeof(vsb));
	sys_retval_int(0);
}

void load(int argc, char **argv)
{
	struct venix_exec hdr;
	Word sp;
	uint32_t b;

	filename = argv[1];
	FILE* fp = fopen(filename, "rb");
	if (fp == 0)
		error("opening");
	int loadOffset = loadSegment << 4;

	/*
	 * Read the whole program into memory
	 */
	if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
		error("reading");

	debug(dbg_load, "Magic is 0%o (%s)\n", hdr.a_magic,
	    hdr.a_magic == OMAGIC ? "OMAGIC" : (hdr.a_magic == NMAGIC ? "NMAGIC" : "CRAZY"));
	if (hdr.a_magic != OMAGIC &&
	    hdr.a_magic != NMAGIC)
		error("Unsupported magic number");

	for (int i = 0; i < FirstS; i++)
		registers[i] = 0;

	uint32_t ptr = 0;
	uint32_t startMem, endMem;
	registers[CS] = loadSegment;
	startMem = loadSegment << 4;
	magic = hdr.a_magic;
	stack = hdr.a_stack;
	if (hdr.a_magic == OMAGIC) {
		/*
		 * OMAGIC + -z stack layout looks like:
		 *
		 * 0 :  stack
		 *	text
		 *	data
		 *	bss
		 * With all the segment registers the same.
		 *
		 * OMAGIC + w/o -z stack layout looks like:
		 *
		 * 0 :  text
		 *	data
		 *	bss
		 *	...
		 *	stack
		 * With all the segment registers the same.
		 *
		 * All the segments are in the file one after the other with
		 * no padding. HDR TEXT (a_text bytes) DATA (d_data bytes)
		 * <rest of stuff, reloc, symbols etc>
		 */
		registers[DS] = registers[ES] = registers[SS] = loadSegment;
		if (hdr.a_stack != 0) {
			debug(dbg_load, "stack from %#x:%#x-%#x\n", registers[SS], ptr, hdr.a_stack - 1);
			memset(&ram[loadOffset + ptr], 0, hdr.a_stack);			// stack (under the text)
			ptr += hdr.a_stack;
		} else {
			debug(dbg_load, "stack down from %#x:%#x\n", registers[SS], 0xff80);
		}
		debug(dbg_load, "Text from %#x:%#x-%#x\n", registers[CS], ptr, ptr + hdr.a_text - 1);
		fread(&ram[loadOffset + ptr], hdr.a_text, 1, fp);		// text
		ptr += hdr.a_text;
		debug(dbg_load, "Data from %#x:%#x-%#x\n", registers[CS], ptr, ptr + hdr.a_data - 1);
		fread(&ram[loadOffset + ptr], hdr.a_data, 1, fp);		// data
		ptr += hdr.a_data;
		debug(dbg_load, "BSS from %#x:%#x-%#x\n", registers[CS], ptr, ptr + hdr.a_bss - 1);
		memset(&ram[loadOffset + ptr], 0, hdr.a_bss);			// bss
		ptr += hdr.a_bss;
		if (hdr.a_stack != 0)
			endMem = (loadOffset + ptr) << 4;
		else
			endMem = (loadOffset + 0x10000) << 4;
	} else {
		/*
		 * NMAGIC + -z stack layout looks like:
		 *
		 * cs :  text (rounded to next 'click' or 512 byte boundary)
		 * ds	stack
		 *	data
		 *	bss
		 * With separate cs and ds regsiters. ss and es are set to ds.
		 */
		debug(dbg_load, "Text from %#x:%#x-%#x\n", registers[CS], ptr, ptr + hdr.a_text - 1);
		fread(&ram[loadOffset + ptr], hdr.a_text, 1, fp);		// text
		ptr += hdr.a_text;
		ptr = (ptr + 511) & ~511;					// Round to nearest click
		registers[DS] = registers[ES] = registers[SS] = loadSegment + (ptr >> 4);
		Word dataOffset = loadOffset + ptr;
		ptr = 0;
		if (hdr.a_stack != 0) {
			debug(dbg_load, "stack from %#x:%#x-%#x\n", registers[SS], ptr, hdr.a_stack - 1);
			memset(&ram[dataOffset + ptr], 0, hdr.a_stack);			// stack (under the text)
			ptr += hdr.a_stack;
		} else {
			debug(dbg_load, "stack down from %#x:%#x\n", registers[SS], 0xff80);
		}
		debug(dbg_load, "Data from %#x:%#x-%#x\n", registers[DS], ptr, ptr + hdr.a_data - 1);
		fread(&ram[dataOffset + ptr], hdr.a_data, 1, fp);		// data
		ptr += hdr.a_data;
		debug(dbg_load, "BSS from %#x:%#x-%#x\n", registers[DS], ptr, ptr + hdr.a_bss - 1);
		memset(&ram[dataOffset + ptr], 0, hdr.a_bss);			// bss
		ptr += hdr.a_bss;
		if (hdr.a_stack != 0)
			endMem = (dataOffset + ptr) << 4;
		else
			endMem = (dataOffset + 0x10000) << 4;
	}
	b = ptr;
	if (hdr.a_stack != 0)
		sp = hdr.a_stack - 2;
	else
		sp = 0xff80;
	ip = hdr.a_entry;					// jump to CS:a_entry
	debug(dbg_load, "Starting at %#x:%#x\n", registers[CS], ip);
	debug(dbg_load, "break is %#x sp is %#x\n", b, sp);

	/*
	 * Mark the memory in use, including the stack.
	 */
	for (uint32_t i = startMem; i < endMem; i++)
		initialized[i >> 3] |= 1 << (i & 7);

	brk = b;
	debug(dbg_load, "text %#x data %#x bss %#x stack %#x -> brk %#x\n",
	    hdr.a_text, hdr.a_data, hdr.a_bss, hdr.a_stack, brk);

	/*
	 * Setup the stack by first 'pushing' the args onto it. First
	 * the strings, then argv, then a couple of 0's for the env (Bad), then
	 * a pointer to argv, then argc.
	 */
	Word args[100];
	debug(dbg_load, "%d args\n", argc - 1);
	/* Note: argv[1] is the program name or argv[0] in the target */
	for (int i = 1; i < argc; i++) {
		int len;

		len = strlen(argv[i]) + 1;
		sp -= len;
		if (sp & 1) sp--;
		copyout(argv[i], sp, len);
		args[i - 1] = sp;
		debug(dbg_load, "argv[%d] = %#x '%s'\n", i - 1, sp, argv[i]);
	}
	args[argc - 1] = 0;
	if (sp & 1) sp--;
	Word env = 0;				// Push 1 words for environ
	sp -= 2;
	copyout(&env, sp, 2);
	sp -= argc * 2;
	copyout(args, sp, argc * 2);
	Word vargc = argc - 1;
	sp -= 2;
	copyout(&vargc, sp, 2);

	registers[SP] = sp;
	debug(dbg_load, "Launching at %#x:0 with ds %#x\n", registers[CS], registers[DS]);
	debug(dbg_load, "sp is %#x\n", registers[SP]);
}

/* 1 _rexit */
void
venix_rexit()
{
	pid_t p;
	venix_pid_t vp;
	int i;

	p = getpid();
	for (i = 0; i < VENIX_NPROC; i++) {
		if (p == pid_host[i]) {
			vp = pid_venix[i];
			break;
		}
	}
	assert(i != VENIX_NPROC);
	pid_venix[i] = -1;
	debug(dbg_syscall, "venix pid %d exit(%d)\n", vp, arg1());
	exit(arg1());
}

/* 2 _fork */
void
venix_fork()
{
	pid_t p;
	venix_pid_t vp;

	p = fork();
	if (p == -1) {
		sys_error(errno);
		return;
	}
	if (p == 0) {
		/* child */
		sys_retval_int(0);
		return;
	} else {
		/* parent */
		vp = pid;
	again:
		vp++;
		if (vp == 0)
			vp++;
		if (vp >= VENIX_MAXPROC)
			vp = 1;
		for (int i = 0; i < VENIX_NPROC; i++)
			if (pid_venix[i] == vp)
				goto again;
		for (int i = 0; i < VENIX_NPROC; i++) {
			if (pid_host[i] == -1) {
				pid_host[i] = p;
				pid_venix[i] = vp;
				sys_retval_int(vp);
				return;
			}
		}
		sys_error(VENIX_EAGAIN);
	}
}

typedef ssize_t (rdwr_fn)(int, void *, size_t);

void rdwr(rdwr_fn *fn, bool isread)
{
	int fd = arg1();
	Word ptr = arg2();
	Word len = arg3();;
	ssize_t rv;
	void *buffer;

	debug(dbg_syscall, "%s(%d, %#x, %d)\n", isread ? "read" : "write", fd, ptr, len);

	if (bad_fd(fd)) {
		sys_error(EBADF);
		return;
	}
	if (bad_addr(ptr)) {
		sys_error(EFAULT);
		return;
	}
	buffer = malloc(len);
	if (buffer == NULL)
		error("Can't malloc");
	if (!isread) {
		copyin(ptr, buffer, len);
	}
	rv = fn(open_fd[fd], buffer, (size_t)len);
	if (rv == -1) {
		sys_error(errno);
		free(buffer);
		return;
	}
	if (isread)
		copyout(buffer, ptr, len);
	sys_retval_int(rv);
	free(buffer);
}

/* 3 _read */
void
venix_read()
{
	rdwr(::read, true);
}

/* 4 _write */
void
venix_write()
{
	rdwr((rdwr_fn *)::write, false);
}

/* 5 _open */
void
venix_open()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();
	Word mode = arg2();
	int fd, i;
	int host_mode;

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;

	for (i = 0; i < VENIX_NOFILE; i++)
		if (open_fd[i] == -1)
			break;

	if (i == VENIX_NOFILE) {
		debug(dbg_syscall, "open(%s, 0%o) -- EMFILE\n", host_fn, mode);
		sys_error(EMFILE);
		return;
	}
	host_mode = venix_o_to_host(mode);
	/*
	 * XXX directories -- V7 had no readdir and read directories
	 * and know about the dirent from the v7 filesystem.
	 * #define	DIRSIZ	14
	 * struct	direct
	 * {
	 * 	ino_t	d_ino;
	 * 	char	d_name[DIRSIZ];
	 * };
	 * If we get a directory open, we'll have to do special things
	 * on read. Also, 14 character name limit... woof... this affects
	 * du, ls, etc
	 */
	debug(dbg_syscall, "open(%s, 0%o)\n", host_fn, mode);
	fd = open(host_fn, host_mode);
	if (fd == -1) {
		sys_error(errno);
		return;
	}
	open_fd[i] = fd;
	sys_retval_int(i);
	debug(dbg_syscall, "-> %d %d\n", i, fd);
}

/* 6 _close */
void
venix_close()
{
	int fd = arg1();

	if (bad_fd(fd)) {
		sys_error(EBADF);
		return;
	}
	debug(dbg_syscall, "close (%d)\n", fd);
	if (open_fd[fd] > 2)
		close(open_fd[fd]);
	open_fd[fd] = -1;
	sys_retval_int(0);
}

/* 7 _wait */
void
venix_wait()
{
	Word status = arg1();
	Word rv;
	int mystat;
	pid_t pid;

//	error("Unimplemented system call 7 _wait\n");

	debug(dbg_syscall, "wait(%#x)\n", status);

	pid = wait(&mystat);
	rv = mystat;
	copyout(&rv, status, sizeof(rv));
	sys_retval_int(pid);//XXX xlate
}

/* 8 _creat */
void
venix_creat()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();
	Word mode = arg2();
	int fd, i;

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	for (i = 0; i < VENIX_NOFILE; i++)
		if (open_fd[i] == -1)
			break;

	debug(dbg_syscall, "creat(%#x %s, 0%o)\n", ufn, host_fn, mode);
	if (i == VENIX_NOFILE) {
		sys_error(EMFILE);
		return;
	}
	if (strlen(host_fn) == 0) {
		sys_error(EINVAL);
		return;
	}
	fd = creat(host_fn, mode);
	if (fd == -1) {
		sys_error(errno);
		return;
	}
	open_fd[i] = fd;
	sys_retval_int(i);
}

/* 9 _link */
void
venix_link()
{
	char host_fn1[HOST_MAXPATHLEN];
	char host_fn2[HOST_MAXPATHLEN];
	Word ufn1 = arg1();
	Word ufn2 = arg2();

	if (copyinfn(ufn1, host_fn1, sizeof(host_fn1)))
		return;
	if (copyinfn(ufn2, host_fn2, sizeof(host_fn2)))
		return;
	sys_retval_int(link(host_fn1, host_fn2));
}

/* 10 _unlink */
void
venix_unlink()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(unlink(host_fn));
}

/* 11 _exec */
void
venix_exec()
{

	error("Unimplemented system call 11 _exec\n");
}

/* 12 _chdir */
void
venix_chdir()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(chdir(host_fn));
}

/* 13 _gtime */
void
venix_gtime()
{
	sys_retval_long(time(NULL));
}

/* 14 _mknod */
void
venix_mknod()
{

	error("Unimplemented system call 14 _mknod\n");
}

/* 15 _chmod */
void
venix_chmod()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();
	Word mode = arg2();

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(chmod(host_fn, mode));
}

/* 16 _chown */
void
venix_chown()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();
	Word uid = arg2();

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(chmod(host_fn, uid));
}

/* 17 _sbreak */
void
venix_sbreak()
{

// XXX -- we need to limit this properly. It appears there's
// a bug in malloc that calls this a lot, so we have to limit
// it to some sane value...
//
// Also, there may be a bug with split I/D space progreams, since that's
// accounted a bit differently.
//
// There's also a bug with the stack. For the assembler, we have:
// Data from 0x1192:0-0x626f
// BSS from 0x1192:0x6270-0xe8b5
// sp is 0xff4c
// which leaves just 0xe8b6 to 0xff4c or 5782 between the top of bss and
// the bottom of the stack.... maybe I should trim argv[0] to just the
// command name rather than the full path... It would save ~30 bytes. But
// we have no sanity checks and we sbrk into the stack, it seems...
//
	Word obrk;

	obrk = brk;
	brk = arg1();
	if (brk == 0)
		brk = obrk;
	if (obrk < brk) {
		/* Mark the memory now in use */
		for (Word i = obrk; i < brk + 15; i++) {
			registers[ES] = registers[DS] + (i >> 4);
			physicalAddress(i & 15, 0, true);
		}
		registers[ES] = registers[DS];
	} /* else shrink ??? */
	debug(dbg_syscall, "sbreak(%#x) %#x\n", arg1(), obrk);
	sys_retval_int(0);
}

/* 18 _stat */
void
venix_stat()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();
	Word usb = arg2();
	struct stat sb;
	int rv;

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	debug(dbg_syscall, "stat(%s)\n", host_fn);
	rv = stat(host_fn, &sb);
	if (rv == -1) {
		sys_error(errno);
		return;
	}
	host_to_venix_sb(&sb, usb);
}

/* 19 _seek */
void
venix_seek()
{
	Word fd = arg1();
	Word off1 = arg2();
	Word off2 = arg3();
	Word whence = arg4();
	off_t off = off1 | (off2 << 16);
	off_t rv;

	debug(dbg_syscall, "lseek(%d, %ld, %d)\n", fd, (long)off, whence);
	rv = lseek(open_fd[fd], off, whence);
	debug(dbg_syscall, "-> rv %#x\n", rv);
	sys_retval_long((uint32_t)rv);
}

/* 20 _getpid */
void
venix_getpid()
{

	sys_retval_int(getpid());
}

/* 21 _smount */
void
venix_smount()
{

	error("Unimplemented system call 21 _smount\n");
}

/* 22 _sumount */
void
venix_sumount()
{

	error("Unimplemented system call 22 _sumount\n");
}

/* 23 _setuid */
void
venix_setuid()
{
	Word uid = arg1();

	sys_retval_int(setuid(uid));
}

/* 24 _getuid */
void
venix_getuid()
{

	sys_retval_int(getuid());
}

/* 25 _stime */
void
venix_stime()
{

	error("Unimplemented system call 25 _stime\n");
}

/* 26 _ptrace */
void
venix_ptrace()
{

	error("Unimplemented system call 26 _ptrace\n");
}

/* 27 _alarm */
void
venix_alarm()
{

	sys_retval_int(alarm(arg1()));
}

/* 28 _fstat */
void
venix_fstat()
{
	int fd = arg1();
	Word usb = arg2();
	struct stat sb;
	int rv;

	if (bad_fd(fd)) {
		sys_error(EBADF);
		return;
	}
	debug(dbg_syscall, "fstat(%d)\n", fd);
	rv = fstat(open_fd[fd], &sb);
	if (rv == -1) {
		sys_error(errno);
		return;
	}
	host_to_venix_sb(&sb, usb);
}

/* 29 _pause */
void
venix_pause()
{

	pause();
	errno = EINTR;
	sys_retval_int(0xffff);
}

/* 30 _utime */
void
venix_utime()
{

	error("Unimplemented system call 30 _utime\n");
}

/* 33 _saccess */
void
venix_saccess()
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1();
	Word mode = arg2();

	if (copyinfn(ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(access(host_fn, mode));
}

/* 34 _nice */
void
venix_nice()
{

	error("Unimplemented system call 34 _nice\n");
}

/* 35 _ftime */
void
venix_ftime()
{
	int rv;
	struct timeval tv;
	Word dst = arg1();
	struct venix_timeb tb;

	rv = gettimeofday(&tv, NULL);
	if (rv) {
		sys_error(errno);
		return;
	}
	tb.time = (uint32_t)tv.tv_sec;
	tb.millitm = (uint16_t)(tv.tv_usec / 1000);
	tb.timezone = 6 * 60;
	tb.dstflag = 1;
	copyout(&tb, dst, sizeof(tb));
	sys_retval_int(0);
	return;
}

/* 36 _sync */
void
venix_sync()
{

	sync();
	sys_retval_int(0);
}

/* 37 _kill */
void
venix_kill()
{
	Word pid = arg1();
	Word sig = arg2();
	int rv;

	if (sig > VENIX_NSIG) {
		sys_error(EINVAL);
		return;
	}
	rv = kill(pid, sig);
	if (rv == -1) {
		sys_error(errno);
		return;
	}
	sys_retval_int(rv);
}

/* 41 _dup */
void
venix_dup()
{

	error("Unimplemented system call 41 _dup\n");
}

/* 42 _pipe */
void
venix_pipe()
{

	error("Unimplemented system call 42 _pipe\n");
}

/* 43 _times */
void
venix_times()
{

	error("Unimplemented system call 43 _times\n");
}

/* 44 _profil */
void
venix_profil()
{

	error("Unimplemented system call 44 _profil\n");
}

/* 45 _syssema */
void
venix_syssema()
{

	error("Unimplemented system call 45 _syssema\n");
}

/* 46 _setgid */
void
venix_setgid()
{
	Word gid = arg1();

	sys_retval_int(setgid(gid));
}

/* 47 _getgid */
void
venix_getgid()
{

	sys_retval_int(getgid());
}

/* 48 _ssig */
void
venix_ssig()
{
	/*
	 * To implement I have to catch signals and redirect since
	 * function pointer addresses are relative to usrland I'm
	 * emulating, not this process. I'll also need to emulating
	 * delivery of the signal, which I'm currently unsure about.
	 *
	 * I also need to arrange so that the signal is delivered
	 * before we start to fetch the next instruction (though
	 * after any rep has finished)
	 */
	Word sig = arg1();
	Word fn = arg2();
	Word oldfn;

	if (sig >= VENIX_NSIG) {
		sys_error(EINVAL);
		return;
	}
	/*
	 * XXX -- need to establish signal handlers for at least some of
	 * the signals.
	 */
	oldfn = venix_sighandle[sig];
	venix_sighandle[sig] = fn;
	sys_retval_int(oldfn);
}

/* 49 _sysdata */
void
venix_sysdata()
{

	error("Unimplemented system call 49 _sysdata\n");
}

/* 50 _suspend */
void
venix_suspend()
{

	error("Unimplemented system call 50 _suspend\n");
}

/* 52 _sysphys */
void
venix_sysphys()
{

	error("Unimplemented system call 52 _sysphys\n");
}

/* 53 _syslock */
void
venix_syslock()
{

	error("Unimplemented system call 53 _syslock\n");
}

/* 54 _ioctl */
void
venix_ioctl()
{
	int fd = arg1();
	int cmd = arg2();
	Word arg = arg3();
	struct venix_sgttyb sg;
	struct termios attr;
	int e;

	/* Should validate fd and arg */
	switch (cmd) {
	case VENIX_TIOCGETP:
		debug(dbg_syscall, "ioctl(TIOCGETP, %d %#x %#x)\n", fd, cmd, arg);
		if (tcgetattr(open_fd[fd], &attr) == -1) {
			e = errno;
			debug(dbg_syscall, "error %d\n", e);
			sys_error(e);
			break;
		}
		sg.sg_ispeed = venix_host_to_speed(cfgetispeed(&attr));
		sg.sg_ospeed = venix_host_to_speed(cfgetospeed(&attr));
		sg.sg_erase = attr.c_cc[VERASE];
		sg.sg_kill = attr.c_cc[VINTR];
		sg.sg_flags = venix_host_to_tc_flags(&attr);
		copyout(&sg, arg, sizeof(sg));
		debug(dbg_syscall, "success!\n");
		sys_retval_int(0);
		break;
	default:
		fprintf(stderr, "undefined ioctl fd %d cmd %#x arg %d\n", fd, cmd, arg);
		error("Unimplemented system call 54 _ioctl\n");
		break;
	}
}

/* 59 _exece */
void
venix_exece()
{
	Word name = arg1();
	Word argv = arg2();
	Word envp = arg3();
	char host_name[HOST_MAXPATHLEN];
	Word args[100];
	Word envs[100];
	char *str_args[100];
	char *str_envs[100];
	int i;

	if (copyinfn(name, host_name, sizeof(host_name))) {
		sys_error(EFAULT);
		return;
	}
	debug(dbg_syscall, "exece(%#x %s %#x %#x)\n", name, host_name, argv, envp);
	memset(args, 0, sizeof(*args));
	memset(str_args, 0, sizeof(*str_args));
	memset(envs, 0, sizeof(*envs));
	memset(str_envs, 0, sizeof(*str_envs));
	i = 0;
	do {
		if (copyin(argv + 2 * i, &args[i], sizeof(Word)))
			goto errout;
//		fprintf(stderr, "---args[%d]=%#x\n", i, args[i]);
		if (args[i] == 0) {
			i++;
			break;
		}
		str_args[i] = (char *)malloc(1024);
		if (copyinstr(args[i], str_args[i], 1024))
			goto errout;
		fprintf(stderr, "---arg[%d]=\"%s\"\n", i, str_args[i] ? str_args[i] : "NULL");
		i++;
	} while (1);
#if 0
	i = 0;
	do {
		if (copyin(envp + 2 * i, envs + i, sizeof(Word)))
			goto errout;
		str_envs[i] = (char *)malloc(1024);
		if (copyinstr(envs[i], str_envs + i, 1024))
			goto errout;
		debug(dbg_syscall, "---env[%d]=\"%s\"\n", i, str_envs[i] ? str_envs[i] : "NULL");
	} while (envs[i] != 0);
#endif
	error("Unimplemented system call 59 _exece\n");
errout:
	for (i = 0; str_args[i]; i++)
		free(str_args[i]);
	for (i = 0; str_envs[i]; i++)
		free(str_envs[i]);
	sys_error(EFAULT);
}

/* 60 _umask */
void
venix_umask()
{
	int numask = arg1();

	sys_retval_int(umask(numask));
}

/* 61 _chroot */
void
venix_chroot()
{

	error("Unimplemented system call 61 _chroot\n");
}

/* 64 _locking */
void
venix_locking()
{

	error("Unimplemented system call 64 _locking\n");
}

void
venix_nosys()
{
	fprintf(stderr, "Venix unimplemented system call %d\n", scall);
	exit(1);
}

typedef void (Venix::*sysfn)(void);

#define NSYS 72
Venix::sysfn sysent[NSYS] = {
	&Venix::venix_nosys,
	&Venix::venix_rexit,
	&Venix::venix_fork,
	&Venix::venix_read,
	&Venix::venix_write,
	&Venix::venix_open,
	&Venix::venix_close,
	&Venix::venix_wait,
	&Venix::venix_creat,
	&Venix::venix_link,
	&Venix::venix_unlink,
	&Venix::venix_exec,
	&Venix::venix_chdir,
	&Venix::venix_gtime,
	&Venix::venix_mknod,
	&Venix::venix_chmod,
	&Venix::venix_chown,
	&Venix::venix_sbreak,
	&Venix::venix_stat,
	&Venix::venix_seek,
	&Venix::venix_getpid,
	&Venix::venix_smount,
	&Venix::venix_sumount,
	&Venix::venix_setuid,
	&Venix::venix_getuid,
	&Venix::venix_stime,
	&Venix::venix_ptrace,
	&Venix::venix_alarm,
	&Venix::venix_fstat,
	&Venix::venix_pause,
	&Venix::venix_utime,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_saccess,
	&Venix::venix_nice,
	&Venix::venix_ftime,
	&Venix::venix_sync,
	&Venix::venix_kill,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_dup,
	&Venix::venix_pipe,
	&Venix::venix_times,
	&Venix::venix_profil,
	&Venix::venix_syssema,
	&Venix::venix_setgid,
	&Venix::venix_getgid,
	&Venix::venix_ssig,
	&Venix::venix_sysdata,
	&Venix::venix_suspend,
	&Venix::venix_nosys,
	&Venix::venix_sysphys,
	&Venix::venix_syslock,
	&Venix::venix_ioctl, // 54
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_exece,
	&Venix::venix_umask,
	&Venix::venix_chroot,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_locking,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
	&Venix::venix_nosys,
};

void int_cd(void)
{
	data = fetchByte();
	switch (data) {
	case 0xf4:
		/* Ignore FPU emulation */
		break;
	case 0xf3:
	case 0xf2:
		printf("abort / emt\n");
		exit(0);
	case 0xf1:
		scall = callno();
		if (scall == 0)
			scall = arg1();
		if (scall < NSYS) {
			printf("Calling system call %d\n", scall);
			(this->*sysent[scall])();
		} else {
			printf("Unimplemented system call %d\n", scall);
			exit(0);
		}
		break;
	default:
		printf("Unimplemented interrupt %#x\n", data);
		exit(0);
	}
}

void start_of_instruction(void)
{
	if (dodis) {
		debug(dbg_emul, "\t\t\t\t| AX %#x BX %#x CX %#x DX %#x SP %#x BP %#x SI %#x DI %#x *IP = %#x FLAGS %#x CS %#x DS %#x ES %#x SS %#x\n",
		    ax(), bx(), cx(), dx(), sp(), bp(), si(), di(), readByte(ip, 1), getFlags(),
		    cs(), ds(), es(), ss());
		debug(dbg_emul, "%04X:%04X: ", cs(), ip);
		db_disasm(ip, false);
	}
	/* XXX check signal mask here */
}

};

#undef printf
