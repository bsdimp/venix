/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2018 The FreeBSD Foundation
 * All rights reserved.
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");
/* $Id: vm86_test.c,v 1.10 2018/05/12 11:35:58 kostik Exp kostik $ */

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <machine/cpufunc.h>
#include <machine/psl.h>
#include <machine/sysarch.h>
#include <machine/vm86.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>

#include "debug.h"

typedef uint16_t Word;

static u_int gs;
struct vm86_init_args va;
void *load_addr;

#define VENIX_ROOT "."
const char *venix_root;

void venix_to_host_path(char *fn, char *host_fn, size_t len)
{
	strlcpy(host_fn, fn, len);
}

Word callno(ucontext_t *uc) { return uc->uc_mcontext.mc_ebx; }
Word arg1(ucontext_t *uc) { return uc->uc_mcontext.mc_eax; }
Word arg2(ucontext_t *uc) { return uc->uc_mcontext.mc_edx; }
Word arg3(ucontext_t *uc) { return uc->uc_mcontext.mc_ecx; }
Word arg4(ucontext_t *uc) { return uc->uc_mcontext.mc_esi; }

static const int HOST_MAXPATHLEN = 1025;

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

// errno for Venix == Errno for FreeBSD <= 34
static const int VENIX_EDEADLOCK  = 40;
/* XXX need to translate errno */
static const int VENIX_EAGAIN = 11;	/* Otherwise they are 1:1 on FreeBSD */

uint16_t venix_break;
int length;
uint16_t scall;

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
_Static_assert(sizeof(struct venix_timeb) == 10, "Bad venix_timeb size");

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
_Static_assert(sizeof(struct venix_sgttyb) == 6, "Bad venix_sgttyb size");

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
_Static_assert(sizeof(struct venix_stat) == 30, "bad venix_stat size");

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

/*
 * Buffer for the times(2) system call.
 */
struct venix_tbuffer
{
	int32_t		proc_user_time;
	int32_t		proc_system_time;
	int32_t		child_user_time;
	int32_t		child_system_time;
};
_Static_assert(sizeof(struct venix_tbuffer) == 4 * 4, "bad venix_tbuffer size");


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

void sys_error(ucontext_t *uc, int e)
{
	mcontext_t *mc = &uc->uc_mcontext;

	mc->mc_ecx = e;		// cx = errno
	mc->mc_eax = 0xffff;	// return -1;
	debug(dbg_syscall, "returning error %d\n", e);
}

int copyinstr(ucontext_t *uc, Word uptr, void *kaddr, size_t len)
{
	char ch;
	char *str = (char *)kaddr;
	mcontext_t *mc = &uc->uc_mcontext;
	uint8_t *up = (uint8_t*)(uintptr_t)(uptr + (mc->mc_ds << 4));

	for (int i = 0; i < len; i++) {
		ch = up[i];
		str[i] = ch;
		if (ch == '\0')
			break;
	}

	return (0);
}

int copyin(ucontext_t *uc, Word uptr, void *kaddr, size_t len)
{
	char *str = (char *)kaddr;
	mcontext_t *mc = &uc->uc_mcontext;
	uint8_t *up = (uint8_t*)(uintptr_t)(uptr + (mc->mc_ds << 4));

	for (int i = 0; i < len; i++) {
		str[i] = up[i];
	}

	return (0);
}

/*
 * Copy data from the 'kernel' kptr to 'userland' uptr for len bytes.
 */
int copyout(ucontext_t *uc, void *kptr, Word uptr, size_t len)
{
	mcontext_t *mc = &uc->uc_mcontext;
	uint8_t *p = kptr;
	uint8_t *up = (uint8_t*)(uintptr_t)(uptr + (mc->mc_ds << 4));

	for (int i = 0; i < len; i++) {
		up[i] = p[i];
	}
	return 0;
}

int copyinfn(ucontext_t *uc, Word uptr, char *hostfn, size_t len)
{
	char fn[VENIX_PATHSIZ];

	if (copyinstr(uc, uptr, fn, sizeof(fn)) != 0) {
		sys_error(uc, EFAULT);
		return 1;
	}
	venix_to_host_path(fn, hostfn, len);
	return 0;
}

void sys_retval_long(ucontext_t *uc, uint32_t r)
{
	mcontext_t *mc = &uc->uc_mcontext;

	if (r == (uint32_t)-1) {
		sys_error(uc, errno);
	} else {
		mc->mc_ecx = 0;			// No errno
		mc->mc_eax = r & 0xffff;
		mc->mc_edx = r >> 16;
		debug(dbg_syscall, "returning long %d %#x\n", r, r);
	}
}

void sys_retval_int(ucontext_t *uc, uint16_t r)
{
	mcontext_t *mc = &uc->uc_mcontext;

	if (r == (uint16_t)-1) {
		sys_error(uc, errno);
	} else {
		mc->mc_ecx = 0;			// No errno
		mc->mc_eax = r;
		debug(dbg_syscall, "returning int %d %#x\n", r, r);
	}
}

bool bad_addr(ucontext_t *uc, Word addr)
{
	mcontext_t *mc = &uc->uc_mcontext;

	if (stack == 0 && addr > mc->mc_esp)
		return false;
	return (addr >= venix_break);
}

bool bad_fd(int fd)
{
	return (fd < 0 || fd >= VENIX_NOFILE || open_fd[fd] == -1);
}

void host_to_venix_sb(ucontext_t *uc, struct stat *sb, Word usb)
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
	copyout(uc, &vsb, usb, sizeof(vsb));
	sys_retval_int(uc, 0);
}

//void *u2k(Word addr) { return (&ram[physicalAddress(addr, DSeg, false)]); }

#define TODO(uc) \
{ \
	mcontext_t *mc = &uc->uc_mcontext;	\
						\
	dump_state(mc);				\
	errx(1, "TODO %s", __func__);		\
}

void
dump_state(mcontext_t *mc)
{
	fprintf(stderr, "Dump state:\n");
	fprintf(stderr, "Executing at %#x:%#x\n", mc->mc_cs, mc->mc_eip);
	fprintf(stderr, "%%ax %#x %%bx %#x %%cx %#x %%dx %#x\n",
	    mc->mc_eax, mc->mc_ebx, mc->mc_ecx, mc->mc_edx);
	fprintf(stderr, "%%di %#x %%si %#x %%bp %#x %%sp %#x\n",
	    mc->mc_edi, mc->mc_esi, mc->mc_ebp, mc->mc_esp);
	fprintf(stderr, "%%cs %#x %%ds %#x %%es %#x %%ss %#x\n",
	    mc->mc_cs, mc->mc_ds, mc->mc_es, mc->mc_ss);
}

static void
venix_init(void)
{
	/*
	 * Initialize our file descriptor table. On old-school Unix, like v7/Venix,
	 * there's 20 fd's per process. This is theoretically changeable in sys/param.h
	 * but it was rare that this was adjusted due to too many other matching constants
	 * that needed changing. So for this emulator, we'll just do the 20, even though
	 * we have this #define. We keep a table of FDs so that we can map back and forth
	 * because other files may be incidentally opened by the host and we want to preserve
	 * the 'highly packed' properties of the fildes table that many programs depend on.
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

	venix_root = getenv("VENIX_ROOT");
	if (venix_root == NULL)
		venix_root = VENIX_ROOT;
}

int
venix_load(ucontext_t *uc, uint8_t *memory, const char *fn, int argc, char **argv, char **envp)
{
	struct venix_exec hdr;
	mcontext_t *mc;
	Word sp;
	uint32_t b;
	char fnb[MAXPATHLEN];
	FILE* fp;

	if (*fn == '/') {
		snprintf(fnb, sizeof(fnb), "%s/%s", venix_root, fn);
		fp = fopen(fnb, "rb");
	}
	if (fp == NULL)
		fp = fopen(fn, "rb");
	if (fp == NULL)
		return -1;

	uintptr_t loadOffset = (uintptr_t)memory;
	uintptr_t loadSegment = (uintptr_t)memory >> 4;

	/*
	 * Read the whole program into memory
	 */
	if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
		err(1, "reading");

	debug(dbg_load, "Magic is 0%o (%s)\n", hdr.a_magic,
	    hdr.a_magic == OMAGIC ? "OMAGIC" : (hdr.a_magic == NMAGIC ? "NMAGIC" : "CRAZY"));
	if (hdr.a_magic != OMAGIC &&
	    hdr.a_magic != NMAGIC)
		err(1, "Unsupported magic number 0%o", hdr.a_magic);

	memset(uc, 0, sizeof(*uc));
	mc = &uc->uc_mcontext;
	mc->mc_eflags = PSL_VM | PSL_USER;

	uint32_t ptr = 0;
	mc->mc_cs = loadSegment;
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
		mc->mc_ds = mc->mc_es = mc->mc_ss = loadSegment;
		if (hdr.a_stack != 0) {
			debug(dbg_load, "stack from %#x:%#x-%#x\n", mc->mc_ss, ptr, hdr.a_stack - 1);
			memset(memory + ptr, 0, hdr.a_stack);			// stack (under the text)
			ptr += hdr.a_stack;
		} else {
			debug(dbg_load, "stack down from %#x:%#x\n", mc->mc_ss, 0xff80);
		}
		debug(dbg_load, "Text from %#x:%#x-%#x\n", mc->mc_cs, ptr, ptr + hdr.a_text - 1);
		fread(memory + ptr, hdr.a_text, 1, fp);				// text
		ptr += hdr.a_text;
		debug(dbg_load, "Data from %#x:%#x-%#x\n", mc->mc_cs, ptr, ptr + hdr.a_data - 1);
		fread(memory + ptr, hdr.a_data, 1, fp);				// data
		ptr += hdr.a_data;
		debug(dbg_load, "BSS from %#x:%#x-%#x\n", mc->mc_cs, ptr, ptr + hdr.a_bss - 1);
		memset(memory + ptr, 0, hdr.a_bss);				// bss
		ptr += hdr.a_bss;
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
		debug(dbg_load, "Text from %#x:%#x-%#x\n", mc->mc_cs, ptr, ptr + hdr.a_text - 1);
		fread(memory + ptr, hdr.a_text, 1, fp);				// text
		ptr += hdr.a_text;
		ptr = (ptr + 511) & ~511;					// Round to nearest click
		mc->mc_ds = mc->mc_es = mc->mc_ss = loadSegment + (ptr >> 4);
		uint8_t *data = memory + ptr;
		ptr = 0;
		if (hdr.a_stack != 0) {
			debug(dbg_load, "stack from %#x:%#x-%#x\n", mc->mc_ss, ptr, hdr.a_stack - 1);
			memset(data + ptr, 0, hdr.a_stack);			// stack (under the text)
			ptr += hdr.a_stack;
		} else {
			debug(dbg_load, "stack down from %#x:%#x\n", mc->mc_ss, 0xff80);
		}
		debug(dbg_load, "Data from %#x:%#x-%#x\n", mc->mc_ds, ptr, ptr + hdr.a_data - 1);
		fread(data + ptr, hdr.a_data, 1, fp);				// data
		ptr += hdr.a_data;
		debug(dbg_load, "BSS from %#x:%#x-%#x\n", mc->mc_ds, ptr, ptr + hdr.a_bss - 1);
		memset(data + ptr, 0, hdr.a_bss);				// bss
		ptr += hdr.a_bss;
	}
	b = ptr;
	if (hdr.a_stack != 0)
		sp = hdr.a_stack - 2;
	else
		sp = 0xff80;
	mc->mc_eip = hdr.a_entry;					// jump to CS:a_entry
	debug(dbg_load, "Starting at %#x:%#x\n", mc->mc_cs, mc->mc_eip);
	debug(dbg_load, "break is %#x sp is %#x\n", b, sp);

	venix_break = b;
	debug(dbg_load, "text %#x data %#x bss %#x stack %#x -> brk %#x\n",
	    hdr.a_text, hdr.a_data, hdr.a_bss, hdr.a_stack, venix_break);

	/*
	 * Setup the stack by first 'pushing' the args onto it. First
	 * the strings, then argv, then a couple of 0's for the env (Bad), then
	 * a pointer to argv, then argc.
	 */
	Word args[100];
	debug(dbg_load, "%d args\n", argc);
	for (int i = 0; i < argc; i++) {
		int len;

		len = strlen(argv[i]) + 1;
		sp -= len;
		if (sp & 1) sp--;
		copyout(uc, argv[i], sp, len);
		args[i] = sp;
		debug(dbg_load, "argv[%d] = %#x '%s'\n", i, sp, argv[i]);
	}
	args[argc] = 0;
	if (sp & 1) sp--;
	Word env = 0;				// Push 1 words for environ
	sp -= 2;
	copyout(uc, &env, sp, 2);
	sp -= argc * 2;
	copyout(uc, args, sp, argc * 2);
	Word vargc = argc;
	sp -= 2;
	copyout(uc, &vargc, sp, 2);

	mc->mc_esp = sp;
	debug(dbg_load, "Launching at %#x:0 with ds %#x\n", mc->mc_cs, mc->mc_ds);
	debug(dbg_load, "sp is %#x\n", mc->mc_esp);
	return(0);
}

void
clear_pid(venix_pid_t vp)
{
	int i;

	for (i = 0; i < VENIX_NPROC; i++) {
		if (vp == pid_venix[i]) {
			pid_venix[i] = (venix_pid_t)-1;
			pid_host[i] = (pid_t)-1;
			break;
		}
	}
}

venix_pid_t
h2v_pid(pid_t p)
{
	int i;

	for (i = 0; i < VENIX_NPROC; i++) {
		if (p == pid_host[i])
			return pid_venix[i];
	}

	return p;
}

/* 1 _rexit */
void
venix_rexit(ucontext_t *uc)
{
	pid_t p;
	venix_pid_t vp;
	int i;

	debug(dbg_syscall, "venix pid %d exit(%d)\n", vp, arg1(uc));
	exit(arg1(uc));
}

/* 2 _fork */
void
venix_fork(ucontext_t *uc)
{
	pid_t p;
	venix_pid_t vp;

	debug(dbg_syscall, "fork\n");
	p = fork();
	if (p == -1) {
		sys_error(uc, errno);
		return;
	}
	if (p == 0) {
		/* child */
		sys_retval_int(uc, 0);
		/*
		 * Start the vm86 subsystem...
		 */
		memset(&va, 0, sizeof(va));
		if (i386_vm86(VM86_INIT, &va) == -1)
			err(1, "VM86_INIT");
		/* XXX MORE? XXX */
		debug(dbg_syscall, "child %d\n", getpid());
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
				sys_retval_int(uc, vp);
				debug(dbg_syscall, "forked %d vp %d\n", p, vp);
				return;
			}
		}
		sys_error(uc, VENIX_EAGAIN); // XXX WRONG?
	}
}

typedef ssize_t (rdwr_fn)(int, void *, size_t);

void rdwr(ucontext_t *uc, rdwr_fn *fn, bool isread)
{
	int fd = arg1(uc);
	Word ptr = arg2(uc);
	Word len = arg3(uc);
	ssize_t rv;
	void *buffer;

	debug(dbg_syscall, "%s(%d, %#x, %d)\n", isread ? "read" : "write", fd, ptr, len);

	if (bad_fd(fd)) {
		sys_error(uc, EBADF);
		return;
	}
	if (len == 0) {
		sys_retval_int(uc, 0);
		return;
	}
	if (bad_addr(uc, ptr)) {
		sys_error(uc, EFAULT);
		return;
	}
	buffer = malloc(len);
	if (buffer == NULL)
		error("Can't malloc");
	if (!isread) {
		copyin(uc, ptr, buffer, len);
	}
	rv = fn(open_fd[fd], buffer, (size_t)len);
	if (rv == -1) {
		sys_error(uc, errno);
		free(buffer);
		return;
	}
	if (isread)
		copyout(uc, buffer, ptr, len);
	sys_retval_int(uc, rv);
	free(buffer);
}

/* 3 _read */
void
venix_read(ucontext_t *uc)
{
	rdwr(uc, read, true);
}

/* 4 _write */
void
venix_write(ucontext_t *uc)
{
	rdwr(uc, (rdwr_fn *)write, false);
}

/* 5 _open */
void
venix_open(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);
	Word mode = arg2(uc);
	int fd, i;
	int host_mode;

	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;

	for (i = 0; i < VENIX_NOFILE; i++)
		if (open_fd[i] == -1)
			break;

	if (i == VENIX_NOFILE) {
		debug(dbg_syscall, "open(%s, 0%o) -- EMFILE\n", host_fn, mode);
		sys_error(uc, EMFILE);
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
	if (host_fn[0] == '/') {
		strcpy(host_fn, venix_root);
		strcat(host_fn, "/");
		if (copyinfn(uc, ufn, host_fn + strlen(host_fn), sizeof(host_fn) - strlen(host_fn)))
			return;//XXX
		fd = open(host_fn, host_mode);
	}
	if (fd == -1) {
		if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
			return;//XXX
		fd = open(host_fn, host_mode);
	}
	if (fd == -1) {
		sys_error(uc, errno);
		return;
	}
	open_fd[i] = fd;
	sys_retval_int(uc, i);
	debug(dbg_syscall, "-> open %s %d %d\n", host_fn, i, fd);
}

/* 6 _close */
void
venix_close(ucontext_t *uc)
{
	int fd = arg1(uc);

	if (bad_fd(fd)) {
		sys_error(uc, EBADF);
		return;
	}
	debug(dbg_syscall, "close (%d)\n", fd);
	if (open_fd[fd] > 2)
		close(open_fd[fd]);
	open_fd[fd] = -1;
	sys_retval_int(uc, 0);
}

/* 7 _wait */
void
venix_wait(ucontext_t *uc)
{
	Word statusp = arg1(uc);
	Word rv;
	int mystat;
	pid_t pid;
	venix_pid_t vp;
	int16_t vstat;

//	error("Unimplemented system call 7 _wait\n");

	debug(dbg_syscall, "wait(%#x)\n", statusp);

	pid = wait(&mystat);
	rv = mystat;
	if (pid != (pid_t)-1) {
		vstat = (int16_t) rv;
		copyout(uc, &vstat, statusp, sizeof(vstat)); // XLATE?
		vp = h2v_pid(pid);
		sys_retval_int(uc, vp);
		debug(dbg_syscall, "wait() status %d pid %d\n", vstat, vp);
		clear_pid(vp);
	} else {
		debug(dbg_syscall, "wait() no kids\n");
		sys_error(uc, ECHILD);
	}
}

/* 8 _creat */
void
venix_creat(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);
	Word mode = arg2(uc);
	int fd, i;

	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	for (i = 0; i < VENIX_NOFILE; i++)
		if (open_fd[i] == -1)
			break;

	debug(dbg_syscall, "creat(%#x %s, 0%o)\n", ufn, host_fn, mode);
	if (i == VENIX_NOFILE) {
		sys_error(uc, EMFILE);
		return;
	}
	if (strlen(host_fn) == 0) {
		sys_error(uc, EINVAL);
		return;
	}
	fd = creat(host_fn, mode);
	if (fd == -1) {
		sys_error(uc, errno);
		return;
	}
	open_fd[i] = fd;
	sys_retval_int(uc, i);
}

/* 9 _link */
void
venix_link(ucontext_t *uc)
{
	char host_fn1[HOST_MAXPATHLEN];
	char host_fn2[HOST_MAXPATHLEN];
	Word ufn1 = arg1(uc);
	Word ufn2 = arg2(uc);

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn1, host_fn1, sizeof(host_fn1)))
		return;
	if (copyinfn(uc, ufn2, host_fn2, sizeof(host_fn2)))
		return;
	debug(dbg_syscall, "link(%s, %s)\n", host_fn1, host_fn2);
	sys_retval_int(uc, link(host_fn1, host_fn2));
}

/* 10 _unlink */
void
venix_unlink(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	debug(dbg_syscall, "unlink(%s)\n", host_fn);
	sys_retval_int(uc, unlink(host_fn));
}

/* 11 _exec */
/*
 * Obsolete, see 59 _exece
 */
void
venix_exec(ucontext_t *uc)
{

	error("Unimplemented system call 11 _exec\n");
}

/* 12 _chdir */
void
venix_chdir(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(uc, chdir(host_fn));
}

/* 13 _gtime */
void
venix_gtime(ucontext_t *uc)
{
	sys_retval_long(uc, time(NULL));
}

/* 14 _mknod */
void
venix_mknod(ucontext_t *uc)
{

	error("Unimplemented system call 14 _mknod\n");
}

/* 15 _chmod */
void
venix_chmod(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);
	Word mode = arg2(uc);

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(uc, chmod(host_fn, mode));
}

/* 16 _chown */
void
venix_chown(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);
	Word uid = arg2(uc);

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	sys_retval_int(uc, chmod(host_fn, uid));
}

/* 17 _sbreak */
void
venix_sbreak(ucontext_t *uc)
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

	obrk = venix_break;
	venix_break = arg1(uc);
	if (venix_break == 0)
		venix_break = obrk;
	debug(dbg_syscall, "sbreak(%#x) %#x\n", arg1(uc), obrk);
	sys_retval_int(uc, 0);
}

/* 18 _stat */
void
venix_stat(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);
	Word usb = arg2(uc);
	struct stat sb;
	int rv;

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	debug(dbg_syscall, "stat(%s)\n", host_fn);
	rv = stat(host_fn, &sb);
	if (rv == -1) {
		sys_error(uc, errno);
		return;
	}
	host_to_venix_sb(uc, &sb, usb);
}

/* 19 _seek */
void
venix_seek(ucontext_t *uc)
{
	Word fd = arg1(uc);
	Word off1 = arg2(uc);
	Word off2 = arg3(uc);
	Word whence = arg4(uc);
	off_t off = off1 | (off2 << 16);
	off_t rv;

	debug(dbg_syscall, "lseek(%d, %ld, %d)\n", fd, (long)off, whence);
	rv = lseek(open_fd[fd], off, whence);
	debug(dbg_syscall, "-> rv %#x\n", rv);
	sys_retval_long(uc, (uint32_t)rv);
}

/* 20 _getpid */
void
venix_getpid(ucontext_t *uc)
{

	sys_retval_int(uc, getpid());
}

/* 21 _smount */
void
venix_smount(ucontext_t *uc)
{

	error("Unimplemented system call 21 _smount\n");
}

/* 22 _sumount */
void
venix_sumount(ucontext_t *uc)
{

	error("Unimplemented system call 22 _sumount\n");
}

/* 23 _setuid */
void
venix_setuid(ucontext_t *uc)
{
	Word uid = arg1(uc);

	sys_retval_int(uc, setuid(uid));
}

/* 24 _getuid */
void
venix_getuid(ucontext_t *uc)
{

	sys_retval_int(uc, getuid());
}

/* 25 _stime */
void
venix_stime(ucontext_t *uc)
{

	error("Unimplemented system call 25 _stime\n");
}

/* 26 _ptrace */
void
venix_ptrace(ucontext_t *uc)
{

	error("Unimplemented system call 26 _ptrace\n");
}

/* 27 _alarm */
void
venix_alarm(ucontext_t *uc)
{

	sys_retval_int(uc, alarm(arg1(uc)));
}

/* 28 _fstat */
void
venix_fstat(ucontext_t *uc)
{
	int fd = arg1(uc);
	Word usb = arg2(uc);
	struct stat sb;
	int rv;

	if (bad_fd(fd)) {
		sys_error(uc, EBADF);
		return;
	}
	debug(dbg_syscall, "fstat(%d)\n", fd);
	rv = fstat(open_fd[fd], &sb);
	if (rv == -1) {
		sys_error(uc, errno);
		return;
	}
	host_to_venix_sb(uc, &sb, usb);
}

/* 29 _pause */
void
venix_pause(ucontext_t *uc)
{

	pause();
	errno = EINTR;
	sys_retval_int(uc, 0xffff);
}

/* 30 _utime */
void
venix_utime(ucontext_t *uc)
{

	error("Unimplemented system call 30 _utime\n");
}

/* 33 _saccess */
void
venix_saccess(ucontext_t *uc)
{
	char host_fn[HOST_MAXPATHLEN];
	Word ufn = arg1(uc);
	Word mode = arg2(uc);
	int rv;

	// XXX need VENIX_ROOT here?
	if (copyinfn(uc, ufn, host_fn, sizeof(host_fn)))
		return;
	rv = access(host_fn, mode);
	debug(dbg_syscall, "access(%s, 0%o)\n", host_fn, mode);
	sys_retval_int(uc, rv);
}

/* 34 _nice */
void
venix_nice(ucontext_t *uc)
{

	error("Unimplemented system call 34 _nice\n");
}

/* 35 _ftime */
void
venix_ftime(ucontext_t *uc)
{
	int rv;
	struct timeval tv;
	Word dst = arg1(uc);
	struct venix_timeb tb;

	rv = gettimeofday(&tv, NULL);
	if (rv) {
		sys_error(uc, errno);
		return;
	}
	tb.time = (uint32_t)tv.tv_sec;
	tb.millitm = (uint16_t)(tv.tv_usec / 1000);
	tb.timezone = 6 * 60;
	tb.dstflag = 1;
	copyout(uc, &tb, dst, sizeof(tb));
	sys_retval_int(uc, 0);
	return;
}

/* 36 _sync */
void
venix_sync(ucontext_t *uc)
{

	sync();
	sys_retval_int(uc, 0);
}

/* 37 _kill */
void
venix_kill(ucontext_t *uc)
{
	Word pid = arg1(uc);
	Word sig = arg2(uc);
	int rv;

	if (sig > VENIX_NSIG) {
		sys_error(uc, EINVAL);
		return;
	}
	rv = kill(pid, sig);
	if (rv == -1) {
		sys_error(uc, errno);
		return;
	}
	sys_retval_int(uc, rv);
}

/* 41 _dup */
/*
 * Paraphrased from Venix dup(2):
 * dup(fildes);
 * dup2(fildes,fildes2);
 *
 * dup opens filedes again with the new value
 * dup2 closes fildes2 and opens fildes again on fildes2
 * -1 returned on errors
 *
 * 8086: BX=41; AX=fildes; DX=fildes2
 * the dup2 entry is implemented by adding 0100 (octal) to
 * fildes.
 */
void
venix_dup(ucontext_t *uc)
{
	Word fildes = arg1(uc);
	Word fildes2 = arg2(uc);
	int fd;
	int i;

	if (fildes & 0100) {
		/* DUP2 */
		fildes &= ~0100;
		if (bad_fd(fildes) || fildes2 > VENIX_NOFILE || fildes2 < 0) {
			sys_error(uc, EBADF);
			return;
		}
		if (open_fd[fildes2] != -1) {
			close(open_fd[fildes2]);
			open_fd[fildes2] = -1;
		}
	} else {
		/* DUP */
		if (bad_fd(fildes)) {
			sys_error(uc, EBADF);
			return;
		}
		for (i = 0; i < VENIX_NOFILE; i++) {
			if (open_fd[i] == -1) {
				fildes2 = i;
				break;
			}
		}
		if (i >= VENIX_NOFILE) {
			sys_error(uc, EMFILE);
			return;
		}
	}
	fd = dup(open_fd[fildes]);
	if (fd < 0) {
		sys_error(uc, errno);
		return;
	}
	open_fd[fildes2] = fd;
	sys_retval_int(uc, fd);
}

/* 42 _pipe */
void
venix_pipe(ucontext_t *uc)
{

	error("Unimplemented system call 42 _pipe\n");
}

/* 43 _times */
/*
 * From Venix's times(2):
 * times(buffer)
 * struct tbuffer *buffer;
 *
 * times returns time-accounting information for the current process and for
 * the terminated child processes of the current process. All times are in
 * 1/Hz seconds, where Hz = 60.
 *
 * the children times are the sum of the children's process times and their
 * children's times.
 *
 * times = 43.
 * 8086: BX=53; AX=buffer; int 0xf1
 *
 * We assume Venix runs at 60Hz, and that the host runs at 1000Hz.
 */
void
venix_times(ucontext_t *uc)
{
	struct tms tms;
	clock_t up;
	Word dst = arg1(uc);
	struct venix_tbuffer tbuf;

	up = times(&tms);
	if (up == (clock_t)-1) {
		sys_error(uc, errno);
		return;
	}
	tbuf.proc_user_time = tms.tms_utime * 60 / 1000;
	tbuf.proc_system_time = tms.tms_stime * 60 / 1000;
	tbuf.child_user_time = tms.tms_cutime * 60 / 1000;
	tbuf.child_system_time = tms.tms_cstime * 60 / 1000;
	copyout(uc, &tbuf, dst, sizeof(tbuf));
	sys_retval_int(uc, 0);
}

/* 44 _profil */
void
venix_profil(ucontext_t *uc)
{

	error("Unimplemented system call 44 _profil\n");
}

/* 45 _syssema */
void
venix_syssema(ucontext_t *uc)
{

	error("Unimplemented system call 45 _syssema\n");
}

/* 46 _setgid */
void
venix_setgid(ucontext_t *uc)
{
	Word gid = arg1(uc);

	sys_retval_int(uc, setgid(gid));
}

/* 47 _getgid */
void
venix_getgid(ucontext_t *uc)
{

	sys_retval_int(uc, getgid());
}

/* 48 _ssig */
void
venix_ssig(ucontext_t *uc)
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
	Word sig = arg1(uc);
	Word fn = arg2(uc);
	Word oldfn;

	if (sig >= VENIX_NSIG) {
		sys_error(uc, EINVAL);
		return;
	}
	/*
	 * XXX -- need to establish signal handlers for at least some of
	 * the signals.
	 */
	oldfn = venix_sighandle[sig];
	venix_sighandle[sig] = fn;
	sys_retval_int(uc, oldfn);
}

/* 49 _sysdata */
void
venix_sysdata(ucontext_t *uc)
{

	error("Unimplemented system call 49 _sysdata\n");
}

/* 50 _suspend */
void
venix_suspend(ucontext_t *uc)
{

	error("Unimplemented system call 50 _suspend\n");
}

/* 52 _sysphys */
void
venix_sysphys(ucontext_t *uc)
{

	error("Unimplemented system call 52 _sysphys\n");
}

/* 53 _syslock */
void
venix_syslock(ucontext_t *uc)
{

	error("Unimplemented system call 53 _syslock\n");
}

/* 54 _ioctl */
void
venix_ioctl(ucontext_t *uc)
{
	int fd = arg1(uc);
	int cmd = arg2(uc);
	Word arg = arg3(uc);
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
			sys_error(uc, e);
			break;
		}
		sg.sg_ispeed = venix_host_to_speed(cfgetispeed(&attr));
		sg.sg_ospeed = venix_host_to_speed(cfgetospeed(&attr));
		sg.sg_erase = attr.c_cc[VERASE];
		sg.sg_kill = attr.c_cc[VINTR];
		sg.sg_flags = venix_host_to_tc_flags(&attr);
		copyout(uc, &sg, arg, sizeof(sg));
		debug(dbg_syscall, "success!\n");
		sys_retval_int(uc, 0);
		break;
	default:
		fprintf(stderr, "undefined ioctl fd %d cmd %#x arg %d\n", fd, cmd, arg);
		error("Unimplemented system call 54 _ioctl\n");
		break;
	}
}

/* 59 _exece */
/*
 * Exec with environment
 * 8086: BX=59; AX=name; DX=argv; CX=envp; int 0xf1
 *
 * So with n args and m env vars:
 * sp ->	nargs
 *		arg0
 *		...
 *		argn
 *		0
 *		env0
 *		...
 *		envm
 *		0
 * arg0:	<arg0\0>
 *		...
 * argn:	<argn\0>
 * env0:	<env0\0>
 *		...
 * envm:	<envm\0>
 *		0
 *
 * There's no padding on the stings and apparently an extra '0' word at
 * the top of the stack.
 *
 * NB: env isn't implemented yet, but it should be. It shouldn't be all
 * of the host's env, though, so I need to figure out how much to filter.
 * Which likely will depend on what /bin/sh and other shells use.
 */
void
venix_exece(ucontext_t *uc)
{
	Word name = arg1(uc);
	Word argv = arg2(uc);
	Word envp = arg3(uc);
	char host_name[HOST_MAXPATHLEN];
	Word args[100];
	Word envs[100];
	char *str_args[100];
	char *str_envs[100];
	int i;
	int err = EFAULT;

	if (copyinfn(uc, name, host_name, sizeof(host_name))) {
		sys_error(uc, EFAULT);
		return;
	}
	debug(dbg_syscall, "exece(%#x %s %#x %#x)\n", name, host_name, argv, envp);
	memset(args, 0, sizeof(*args));
	memset(str_args, 0, sizeof(*str_args));
	memset(envs, 0, sizeof(*envs));
	memset(str_envs, 0, sizeof(*str_envs));
	i = 0;
	do {
		if (copyin(uc, argv + 2 * i, &args[i], sizeof(Word)))
			goto errout;
//		fprintf(stderr, "---args[%d]=%#x\n", i, args[i]);
		if (args[i] == 0) {
			break;
		}
		str_args[i] = (char *)malloc(1024);
		if (copyinstr(uc, args[i], str_args[i], 1024))
			goto errout;
		debug(dbg_syscall, "---arg[%d]=\"%s\"\n", i, str_args[i] ? str_args[i] : "NULL");
		i++;
	} while (1);
#if 0
	i = 0;
	do {
		if (copyin(uc, envp + 2 * i, envs + i, sizeof(Word)))
			goto errout;
		str_envs[i] = (char *)malloc(1024);
		if (copyinstr(uc, envs[i], str_envs + i, 1024))

			goto errout;
		debug(dbg_syscall, "---env[%d]=\"%s\"\n", i, str_envs[i] ? str_envs[i] : "NULL");
	} while (envs[i] != 0);
#endif
	if (venix_load(uc, load_addr, host_name, i, str_args, NULL) == 0)
		return;
	err = ENOENT;
errout:
#if 0
	for (i = 0; str_args[i]; i++)
		free(str_args[i]);
	for (i = 0; str_envs[i]; i++)
		free(str_envs[i]);
#endif
	sys_error(uc, err);
}

/* 60 _umask */
void
venix_umask(ucontext_t *uc)
{
	int numask = arg1(uc);

	sys_retval_int(uc, umask(numask));
}

/* 61 _chroot */
void
venix_chroot(ucontext_t *uc)
{

	error("Unimplemented system call 61 _chroot\n");
}

/* 64 _locking */
void
venix_locking(ucontext_t *uc)
{

	error("Unimplemented system call 64 _locking\n");
}

void
venix_nosys(ucontext_t *uc)
{
	fprintf(stderr, "Venix unimplemented system call %d\n", scall);
	exit(1);
}

typedef void (*sysfn)(ucontext_t *uc);

#define NSYS 72
sysfn sysent[NSYS] = {
	&venix_nosys,		// 0
	&venix_rexit,
	&venix_fork,
	&venix_read,
	&venix_write,
	&venix_open,
	&venix_close,
	&venix_wait,
	&venix_creat,
	&venix_link,
	&venix_unlink,		// 10
	&venix_exec,
	&venix_chdir,
	&venix_gtime,
	&venix_mknod,
	&venix_chmod,
	&venix_chown,
	&venix_sbreak,
	&venix_stat,
	&venix_seek,
	&venix_getpid,		// 20
	&venix_smount,
	&venix_sumount,
	&venix_setuid,
	&venix_getuid,
	&venix_stime,
	&venix_ptrace,
	&venix_alarm,
	&venix_fstat,
	&venix_pause,
	&venix_utime,		// 30
	&venix_nosys,		// 31
	&venix_nosys,		// 32
	&venix_saccess,
	&venix_nice,
	&venix_ftime,
	&venix_sync,
	&venix_kill,
	&venix_nosys,		// 38
	&venix_nosys,		// 39
	&venix_nosys,		// 40
	&venix_dup,
	&venix_pipe,
	&venix_times,
	&venix_profil,
	&venix_syssema,
	&venix_setgid,
	&venix_getgid,
	&venix_ssig,
	&venix_sysdata,
	&venix_suspend,		// 50
	&venix_nosys,		// 51
	&venix_sysphys,
	&venix_syslock,
	&venix_ioctl,		// 54
	&venix_nosys,		// 55
	&venix_nosys,		// 56
	&venix_nosys,		// 57
	&venix_nosys,		// 58
	&venix_exece,
	&venix_umask,		// 60
	&venix_chroot,
	&venix_nosys,		// 62
	&venix_nosys,		// 63
	&venix_locking,
	&venix_nosys,		// 65
	&venix_nosys,		// 66
	&venix_nosys,		// 67
	&venix_nosys,		// 68
	&venix_nosys,		// 69
	&venix_nosys,		// 70
	&venix_nosys,		// 71
};

const char *sysname[NSYS] = {
	"sycall",		// 0
	"rexit",
	"fork",
	"read",
	"write",
	"open",
	"close",
	"wait",
	"creat",
	"link",
	"unlink",		// 10
	"exec",
	"chdir",
	"gtime",
	"mknod",
	"chmod",
	"chown",
	"sbreak",
	"stat",
	"seek",
	"getpid",		// 20
	"smount",
	"sumount",
	"setuid",
	"getuid",
	"stime",
	"ptrace",
	"alarm",
	"fstat",
	"pause",
	"utime",		// 30
	"sys-31",		// 31
	"sys-32",		// 32
	"saccess",
	"nice",
	"ftime",
	"sync",
	"kill",
	"sys-38",		// 38
	"sys-39",		// 39
	"sys-40",		// 40
	"dup",
	"pipe",
	"times",
	"profil",
	"syssema",
	"setgid",
	"getgid",
	"ssig",
	"sysdata",
	"suspend",		// 50
	"sys-51",		// 51
	"sysphys",
	"syslock",
	"ioctl",		// 54
	"sys-55",		// 55
	"sys-56",		// 56
	"sys-57",		// 57
	"sys-58",		// 58
	"exece",
	"umask",		// 60
	"chroot",
	"sys-62",		// 62
	"sys-63",		// 63
	"locking",
	"sys-65",		// 65
	"sys-66",		// 66
	"sys-67",		// 67
	"sys-68",		// 68
	"sys-69",		// 69
	"sys-70",		// 70
	"sys-71",		// 71
};

void
venix_cd(ucontext_t *uc)
{
	uint8_t *ptr;
	mcontext_t *mc;
	uint16_t scall;

	mc = &uc->uc_mcontext;
	ptr = (uint8_t *)(uintptr_t)((mc->mc_cs << 4) | mc->mc_eip);
	mc->mc_eip += 2;

	switch (ptr[1]) {
	case 0xf4:
		/* Ignore FPU emulation */
		break;
	case 0xf3:
	case 0xf2:
		printf("abort / emt\n");
		exit(0);
	case 0xf1:
		scall = callno(uc);
		if (scall == 0)
			scall = arg1(uc);
		if (scall < NSYS) {
			debug(dbg_syscall, "System call %d %s(%#x, %#x, %#x, %#x)\n", scall,
			    sysname[scall], arg1(uc), arg2(uc), arg3(uc), arg4(uc));
			(sysent[scall])(uc);
		} else {
			printf("Unimplemented system call %d\n", scall);
			exit(0);
		}
		break;
	default:
		printf("Unimplemented interrupt %#x\n", ptr[1]);
		TODO(uc);
		exit(0);
	}
}

static void
sig_handler(int signo, siginfo_t *si __unused, void *ucp)
{
	ucontext_t *uc;
	mcontext_t *mc;

	uc = ucp;
	mc = &uc->uc_mcontext;

	/*
	 * Reload pointer to the TLS base, so that malloc inside
	 * printf() works.
	 */
	load_gs(gs);

	if (signo == SIGSEGV) {
		uint8_t *ptr;

		ptr = (uint8_t *)(uintptr_t)((mc->mc_cs << 4) | mc->mc_eip);
		if (ptr[0] == 0xcd) {
			venix_cd(uc);
			return;
		}
	}

	printf("sig %d %%eax %#x %%ecx %#x %%eip %#x:%#x\n", signo,
	    mc->mc_eax, mc->mc_ecx, mc->mc_cs, mc->mc_eip);
}

#define KiB	(1UL << 10)
#define MiB	(1UL << 20)

int
main(int argc, char **argv)
{
	ucontext_t uc;
	struct sigaction sa;
	stack_t ssa;
	char *vm86_code;

	gs = rgs();

	venix_init();

	/*
	 * Setup the alternative stack for our trap handler.
	 */
	memset(&ssa, 0, sizeof(ssa));
	ssa.ss_size = PAGE_SIZE * 128;
	ssa.ss_sp = mmap(NULL, ssa.ss_size, PROT_READ | PROT_WRITE |
	    PROT_EXEC, MAP_ANON, -1, 0);
	if (ssa.ss_sp == MAP_FAILED)
		err(1, "mmap sigstack");
	if (sigaltstack(&ssa, NULL) == -1)
		err(1, "sigaltstack");

	/*
	 * Setup the signal handlers that will be used to catch
	 * SIGSEGV and others to drive this.
	 */
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = sig_handler;
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
	if (sigaction(SIGBUS, &sa, NULL) == -1)
		err(1, "sigaction SIGBUS");
	if (sigaction(SIGSEGV, &sa, NULL) == -1)
		err(1, "sigaction SIGSEGV");
	if (sigaction(SIGILL, &sa, NULL) == -1)
		err(1, "sigaction SIGILL");

	/*
	 * Start the vm86 subsystem...
	 */
	memset(&va, 0, sizeof(va));
	if (i386_vm86(VM86_INIT, &va) == -1)
		err(1, "VM86_INIT");

	/*
	 * Map 1MiB-64kiB at offset 64kiB. We're unable to map at
	 * location 0, but do need to grab low memory. On x86, the
	 * default location is 0x402000, which is just above 4MB so
	 * this should always work.
	 */
	load_addr = (void *)(64 * KiB);
	vm86_code = mmap(load_addr, 1 * MiB - 64 * KiB,
	    PROT_READ | PROT_WRITE | PROT_EXEC,
	    MAP_ANON | MAP_FIXED, -1, 0);
	if (vm86_code == MAP_FAILED)
		err(1, "mmap");

	/*
	 * Now setup the context to start the program.
	 */
	venix_load(&uc, load_addr, argv[1], argc - 1, argv + 1, NULL);

	sigreturn(&uc);
}
