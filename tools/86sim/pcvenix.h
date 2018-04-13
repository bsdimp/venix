#include <sys/time.h>

class Venix : public MachineOS
{
	static const int VENIX_NOFILE = 20;	// Max files per process on Venix (from sys/param.h, so maybe tunable)
	int open_fd[VENIX_NOFILE];
public:

Venix() : brk(0), length(0), call(0) {
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
	uint32_t brk;
	int length;
	Word call;
	static const int OMAGIC = 0407;		// Old impure format (TINY SS = CS = DS = ES)
	static const int NMAGIC = 0411;		// I&D fprmat (CS and SS = DS == ES)

	/* Basic types */
	typedef int32_t venix_time_t;

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

/*
 * Copy data from the 'kernel' kptr to 'userland' uptr for len bytes.
 */
int copyout(void *kptr, Word uptr, size_t len)
{
	uint8_t *p = reinterpret_cast<uint8_t *>(kptr);

	for (int i = 0; i < len; i++)
		writeByte(p[i], uptr + i, DSeg);
	return 0;
}

void load(int argc, char **argv)
{
	struct venix_exec hdr;

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
	if (hdr.a_magic != OMAGIC)
		error("Not OMAGIC");
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
	 * Initialize all the segment registers to be the same. For
	 * venix, we read the whole image into memory, move the data
	 * segment, setup the stack and go.
	 *
	 * For NMAGIC, we need to adjust, but  for OMAGIC things are fine.
	 */
	for (int i = 0; i < 4; i++)
		registers[FirstS + i] = loadSegment;
	for (int i = 0; i < FirstS; i++)
		registers[i] = 0;

	/*
	 * Hack for the moment -- enough 0's on the stack work until we
	 * need command line args.
	 */
	registers[SP] = hdr.a_text + hdr.a_stack - 32;
	ip = 0;			// jump to CS:0
}

/* 1 _rexit */
void
venix_rexit()
{
	printf("exit(%d)\n", ax());
	exit(ax());
}

/* 2 _fork */
void
venix_fork()
{

	error("Unimplemented system call 2 _fork\n");
}

/* 3 _read */
void
venix_read()
{

	error("Unimplemented system call 3 _read\n");
}

/* 4 _write */
void
venix_write()
{

	printf("write(%d, %#x, %d)\n", ax(), dx(), cx());
	write(1, &ram[physicalAddress(dx(), DSeg, false)], cx());
	setAX(cx());
}

/* 5 _open */
void
venix_open()
{

	error("Unimplemented system call 5 _open\n");
}

/* 6 _close */
void
venix_close()
{

	error("Unimplemented system call 6 _close\n");
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

	error("Unimplemented system call 8 _creat\n");
}

/* 9 _link */
void
venix_link()
{

	error("Unimplemented system call 9 _link\n");
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

	error("Unimplemented system call 12 _chdir\n");
}

/* 13 _gtime */
void
venix_gtime()
{
	uint32_t t;

	t = time(NULL);
	setAX(t & 0xffff);
	setDX(t >> 16);
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

	error("Unimplemented system call 17 _sbreak\n");
}

/* 18 _stat */
void
venix_stat()
{

	error("Unimplemented system call 18 _stat\n");
}

/* 19 _seek */
void
venix_seek()
{

	error("Unimplemented system call 19 _seek\n");
}

/* 20 _getpid */
void
venix_getpid()
{

	error("Unimplemented system call 20 _getpid\n");
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

	error("Unimplemented system call 24 _getuid\n");
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

	error("Unimplemented system call 27 _alarm\n");
}

/* 28 _fstat */
void
venix_fstat()
{

	error("Unimplemented system call 28 _fstat\n");
}

/* 29 _pause */
void
venix_pause()
{

	error("Unimplemented system call 29 _pause\n");
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
	Word dst = ax();
	struct venix_timeb tb;

	rv = gettimeofday(&tv, NULL);
	if (rv) {
		printf("Failed gettimeofday\n");
		setCX(errno);
		setAX(0xffff);
		return;
	}
	tb.time = (uint32_t)tv.tv_sec;
	tb.millitm = (uint16_t)(tv.tv_usec / 1000);
	tb.timezone = 6 * 60;
	tb.dstflag = 1;
	copyout(&tb, ax(), sizeof(tb));
	setAX(0);
	return;
}

/* 36 _sync */
void
venix_sync()
{

	sync();
	setAX(0);
}

/* 37 _kill */
void
venix_kill()
{

	error("Unimplemented system call 37 _kill\n");
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

	error("Unimplemented system call 47 _getgid\n");
}

/* 48 _ssig */
void
venix_ssig()
{

	error("Unimplemented system call 48 _ssig\n");
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

	error("Unimplemented system call 54 _ioctl\n");
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

	error("Unimplemented system call 60 _umask\n");
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
	printf("Venix unimplemented system call %d\n", call);
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
		printf("FPU\n");
		break;
	case 0xf3:
	case 0xf2:
		printf("abort / emt\n");
		exit(0);
	case 0xf1:
		call = bx();
		if (call == 0)
			call = ax();
		if (call < NSYS) {
			printf("Calling system call %d\n", call);
			(this->*sysent[call])();
		} else {
			printf("Unimplemented system call %d\n", call);
			exit(0);
		}
		break;
	default:
		printf("Unimplemented interrupt %#x\n", data);
		exit(0);
	}
}

};
