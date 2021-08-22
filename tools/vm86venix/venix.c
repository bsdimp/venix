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

#include <machine/cpufunc.h>
#include <machine/psl.h>
#include <machine/sysarch.h>
#include <machine/vm86.h>

#include <err.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"

typedef uint16_t Word;

static u_int gs;

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


int copyout(ucontext_t *uc, void *kptr, Word uptr, size_t len);

static void
venix_init(void)
{
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

void
venix_load(ucontext_t *uc, uint8_t *memory, int argc, char **argv)
{
	struct venix_exec hdr;
	mcontext_t *mc;
	Word sp;
	uint32_t b;
	char *filename;

	mc = &uc->uc_mcontext;
	filename = argv[1];
	FILE* fp = fopen(filename, "rb");
	if (fp == 0)
		err(1, "opening");
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
#if 0
		if (hdr.a_stack != 0)
			endMem = (loadOffset + ptr) << 4;
		else
			endMem = (loadOffset + 0x10000) << 4;
#endif
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
#if 0
		if (hdr.a_stack != 0)
			endMem = (dataOffset + ptr) << 4;
		else
			endMem = (dataOffset + 0x10000) << 4;
#endif
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
	debug(dbg_load, "%d args\n", argc - 1);
	/* Note: argv[1] is the program name or argv[0] in the target */
	for (int i = 1; i < argc; i++) {
		int len;

		len = strlen(argv[i]) + 1;
		sp -= len;
		if (sp & 1) sp--;
		copyout(uc, argv[i], sp, len);
		args[i - 1] = sp;
		debug(dbg_load, "argv[%d] = %#x '%s'\n", i - 1, sp, argv[i]);
	}
	args[argc - 1] = 0;
	if (sp & 1) sp--;
	Word env = 0;				// Push 1 words for environ
	sp -= 2;
	copyout(uc, &env, sp, 2);
	sp -= argc * 2;
	copyout(uc, args, sp, argc * 2);
	Word vargc = argc - 1;
	sp -= 2;
	copyout(uc, &vargc, sp, 2);

	mc->mc_esp = sp;
	debug(dbg_load, "Launching at %#x:0 with ds %#x\n", mc->mc_cs, mc->mc_ds);
	debug(dbg_load, "sp is %#x\n", mc->mc_esp);
}

void
venix_cd(ucontext_t *uc)
{
	err(1, "TODO");
}
	

/*
 * Copy data from the 'kernel' kptr to 'userland' uptr for len bytes.
 */
int copyout(ucontext_t *uc, void *kptr, Word uptr, size_t len)
{
	uint8_t *p = kptr;
	mcontext_t *mc = &uc->uc_mcontext;
	uint8_t *up = (uint8_t*)(uintptr_t)(uptr + (mc->mc_ds << 4));

	for (int i = 0; i < len; i++) {
		up[i] = p[i];
	}
	return 0;
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
		if (ptr[0] == 0xcd)
			venix_cd(uc);
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
	struct vm86_init_args va;
	stack_t ssa;
	char *vm86_code;
	void *load_addr;

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
	memset(&uc, 0, sizeof(uc));
	uc.uc_mcontext.mc_eflags = PSL_VM | PSL_USER;
	venix_load(&uc, load_addr, argc, argv);

	sigreturn(&uc);
}
