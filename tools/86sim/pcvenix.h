#include <sys/time.h>
#include <signal.h>

class Venix : public MachineOS
{
	static const int MAXPATHLEN = 1025;

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
	static const int VENIX_TIOCGETC  = (('t'<<8)|18);	/** %get special characters */
	static const int VENIX_TIOCQCNT	 = (('t'<<8)|30);	/* get char counts on i/o queues */
	static const int VENIX_AIOCWAIT	 = (('a'<<8)|0);	/* wait/test outstanding requests */
	/* FIO and DIO features not supported */

	static const int VENIX_B0        = 0;
	static const int VENIX_B50       = 1;
	static const int VENIX_B75       = 2;
	static const int VENIX_B11       = 3;
	static const int VENIX_B13       = 4;
	static const int VENIX_B15       = 5;
	static const int VENIX_B20       = 6;
	static const int VENIX_B30       = 7;
	static const int VENIX_B60       = 8;
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

	printf("Copyin from %#x cs %#x ds %#x\n", uptr, cs(), ds());
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

void sys_retval_long(uint32_t r)
{
	setCX(0);			// No errno
	setAX(r & 0xffff);
	setDX(r >> 16);
}

void sys_retval_int(uint16_t r)
{
	setCX(0);			// No errno
	setAX(r);
}


void sys_error(int e)
{
	setCX(e);		// cx = errno
	sys_retval_int(0xffff); // return -1;
}

bool bad_addr(Word addr)
{
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

	printf("Magic is 0%o\n", hdr.a_magic);
	if (hdr.a_magic != OMAGIC &&
	    hdr.a_magic != NMAGIC)
		error("Unsupported magic number");

	/*
	 * Layout in memory is text, stack, data, bss, but is
	 * hdr, text, data in the disk file. Move things around
	 * to cope.
	 *	Move data above stack
	 *	bzero the stack
	 *	bzero bss
	 * we'll likely need to create a pseudo-a area to propery
	 * emulate Venix, and we should note brk there as the end
	 * of bss.
	 */
	fread(&ram[loadOffset], hdr.a_text, 1, fp);			// text
	memset(&ram[loadOffset + hdr.a_text], 0, hdr.a_stack);		// stack
	fread(&ram[loadOffset + hdr.a_text + hdr.a_stack],		// data
	    hdr.a_data, 1, fp);
	memset(&ram[loadOffset + hdr.a_text + hdr.a_stack + hdr.a_data], // bss
	    0, hdr.a_bss);
	brk = hdr.a_text + hdr.a_stack + hdr.a_data + hdr.a_bss;

	/*
	 * Mark the memory in use, including the stack.
	 */
	for (int i = 0; i < brk + 15; i++) {
		registers[ES] = loadSegment + (i >> 4);
		physicalAddress(i & 15, 0, true);
	}

	/*
	 * For NMAGIC binaries, the 'break' address doesn't include the text section,
	 * so adjust that after we've marked all the memory in use.
	 */
	if (hdr.a_magic == NMAGIC)
		brk -= hdr.a_text;
	/*
	 * Initialize all the segment registers to be the same. For
	 * venix, we read the whole image into memory, move the data
	 * segment, setup the stack and go.
	 *
	 * For NMAGIC, we need to adjust, but  for OMAGIC things are fine.
	 */
	registers[CS] = loadSegment;
	registers[DS] = registers[ES] = registers[SS] = (hdr.a_magic == OMAGIC) ?
	    loadSegment : loadSegment + ((hdr.a_text + 15) >> 4);
	for (int i = 0; i < FirstS; i++)
		registers[i] = 0;
	printf("Reloc sizes: text %d bytes data %d bytes\n",
	    hdr.a_trsize, hdr.a_drsize);
	printf("Launching at %#x:0 with ds %#x\n", registers[CS], registers[DS]);

	/*
	 * Setup the stack by first 'pushing' the args onto it. First
	 * the strings, then argv, then a couple of 0's for the env (Bad), then
	 * a pointer to argv, then argc.
	 */
	sp = hdr.a_text + hdr.a_stack;
	Word args[100];
	printf("%d args\n", argc - 1);
	/* Note: argv[1] is the program name or argv[0] in the target */
	for (int i = 1; i < argc; i++) {
		int len;

		len = strlen(argv[i]) + 1;
		sp -= len;
		if (sp & 1) sp--;
		copyout(argv[i], sp, len);
		args[i - 1] = sp;
		printf("argv[%d] = %#x '%s'\n", i, sp, argv[i]);
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
	ip = 0;			// jump to CS:0
}

/* 1 _rexit */
void
venix_rexit()
{
	printf("exit(%d)\n", arg1());
	exit(arg1());
}

/* 2 _fork */
void
venix_fork()
{

	error("Unimplemented system call 2 _fork\n");
}

typedef ssize_t (rdwr_fn)(int, void *, size_t);

void rdwr(rdwr_fn *fn, bool isread)
{
	int fd = arg1();
	Word ptr = arg2();
	Word len = arg3();;
	ssize_t rv;
	void *buffer;

	printf("%s(%d, %#x, %d)\n", isread ? "read" : "write", fd, ptr, len);

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
		printf("openfd %d first char %#x\n", open_fd[fd], *(char *)buffer);
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
	char fn[VENIX_PATHSIZ];
	char host_fn[MAXPATHLEN];
	Word ufn = arg1();
	Word mode = arg2();
	int fd, i;
	int host_mode;

	if (copyinstr(ufn, fn, sizeof(fn)) != 0) {
		sys_error(EFAULT);
		return;
	}
	for (i = 0; i < VENIX_NOFILE; i++)
		if (open_fd[i] == -1)
			break;

	if (i == VENIX_NOFILE) {
		sys_error(EMFILE);
		return;
	}
	venix_to_host_path(fn, host_fn, sizeof(host_fn));
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
	fd = open(host_fn, host_mode);
	if (fd == -1) {
		sys_error(errno);
		return;
	}
	open_fd[i] = fd;
	sys_retval_int(i);
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
	printf("close %d\n", fd);
	if (open_fd[fd] > 2)
		close(open_fd[fd]);
	open_fd[fd] = -1;
	sys_retval_int(0);
}

/* 7 _wait */
void
venix_wait()
{

	error("Unimplemented system call 7 _wait\n");
}

/* 8 _creat */
void
venix_creat()
{
	char fn[VENIX_PATHSIZ];
	char host_fn[MAXPATHLEN];
	Word ufn = arg1();
	Word mode = arg2();
	int fd, i;

	if (copyinstr(ufn, fn, sizeof(fn)) != 0) {
		sys_error(EFAULT);
		return;
	}
	for (i = 0; i < VENIX_NOFILE; i++)
		if (open_fd[i] == -1)
			break;

	if (i == VENIX_NOFILE) {
		sys_error(EMFILE);
		return;
	}
	venix_to_host_path(fn, host_fn, sizeof(host_fn));
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
	char fn1[VENIX_PATHSIZ];
	char fn2[VENIX_PATHSIZ];
	char host_fn1[MAXPATHLEN];
	char host_fn2[MAXPATHLEN];
	Word ufn1 = arg1();
	Word ufn2 = arg2();

	if (copyinstr(ufn1, fn1, sizeof(fn1)) != 0) {
		sys_error(EFAULT);
		return;
	}
	venix_to_host_path(fn1, host_fn1, sizeof(host_fn1));
	if (copyinstr(ufn2, fn2, sizeof(fn2)) != 0) {
		sys_error(EFAULT);
		return;
	}
	venix_to_host_path(fn2, host_fn2, sizeof(host_fn2));
	if (link(host_fn1, host_fn2) == -1) {
		sys_error(errno);
		return;
	}
	sys_retval_int(0);
}

/* 10 _unlink */
void
venix_unlink()
{

	error("Unimplemented system call 10 _unlink\n");
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
	char fn[VENIX_PATHSIZ];
	char host_fn[MAXPATHLEN];
	Word ufn = arg1();

	if (copyinstr(ufn, fn, sizeof(fn)) != 0) {
		sys_error(EFAULT);
		return;
	}
	venix_to_host_path(fn, host_fn, sizeof(host_fn));
	if (chdir(host_fn) == -1) {
		sys_error(errno);
		return;
	}
	sys_retval_int(0);
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

	error("Unimplemented system call 15 _chmod\n");
}

/* 16 _chown */
void
venix_chown()
{

	error("Unimplemented system call 16 _chown\n");
}

/* 17 _sbreak */
void
venix_sbreak()
{

//	printf("sbreak(%#x) old %#x\n", arg1(), brk);
// XXX -- we need to limit this properly. It appears there's
// a bug in malloc that calls this a lot, so we have to limit
// it to some sane value...
	if (arg1() >= 0x8000) {
		sys_error(0xffff);
		return;
	}
	brk = arg1();
	sys_retval_int(0);
}

/* 18 _stat */
void
venix_stat()
{
	char fn[VENIX_PATHSIZ];
	char host_fn[MAXPATHLEN];
	Word ufn = arg1();
	Word usb = arg2();
	struct stat sb;
	int rv;

	if (copyinstr(ufn, fn, sizeof(fn)) != 0) {
		sys_error(EFAULT);
		return;
	}
	venix_to_host_path(fn, host_fn, sizeof(host_fn));
	printf("stat %s\n", host_fn);
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

	printf("lseek(%d, %ld, %d)\n", fd, (long)off, whence);
	rv = lseek(fd, off, whence);
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

	error("Unimplemented system call 23 _setuid\n");
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
	int rv;

	rv = alarm(arg1());
	if (rv == -1) {
		sys_error(errno);
		return;
	}
	sys_retval_int(rv);
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
	printf("fstat %d\n", fd);
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

	error("Unimplemented system call 33 _saccess\n");
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
		printf("Failed gettimeofday\n");
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

	error("Unimplemented system call 46 _setgid\n");
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

	/* Should validate fd and arg */
	switch (cmd) {
	case VENIX_TIOCGETP:
		printf("TIOCGETP\n");
		/* should really do the conversion */
		sg.sg_ispeed = sg.sg_ospeed = VENIX_B9600;
		sg.sg_erase = 0x7f;	// DEL
		sg.sg_kill = 3;		// ^C
		sg.sg_flags = 0;	// Flags, any interesting?
		copyout(&sg, arg, sizeof(sg));
		sys_retval_int(0);
		break;
	default:
		printf("undefined ioctl fd %d cmd %#x arg %d\n", fd, cmd, arg);
		error("Unimplemented system call 54 _ioctl\n");
		break;
	}
}

/* 59 _exece */
void
venix_exece()
{

	error("Unimplemented system call 59 _exece\n");
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
	printf("Venix unimplemented system call %d\n", scall);
	exit(0);
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
	&Venix::venix_ioctl,
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

};
