#include <sys/time.h>

class Venix : public MachineOS
{
public:
	Venix() : brk(0), length(0) {}
	~Venix() {}
private:
	uint32_t brk;
	int length;
	static const int OMAGIC = 0407;
	static const int NMAGIC = 0411;

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

void
venix_time()
{
	uint32_t t;

	t = time(NULL);
	setAX(t & 0xffff);
	setDX(t >> 16);
}

void
venix_ftime(Word ax)
{
	int rv;
	struct timeval tv;
	uint32_t t;
	uint16_t ms;

	rv = gettimeofday(&tv, NULL);
	if (rv) {
		printf("Failed gettimeofday\n");
		setCX(errno);
		setAX(0xffff);
		return;
	}
	t = (uint32_t)tv.tv_sec;
	writeByte(t & 0xff, ax++, DSeg);
	t >>= 8;
	writeByte(t & 0xff, ax++, DSeg);
	t >>= 8;
	writeByte(t & 0xff, ax++, DSeg);
	t >>= 8;
	writeByte(t & 0xff, ax++, DSeg);
	ms = (uint16_t)(tv.tv_usec / 1000);
	writeWord(ms, ax, DSeg);
	ax += 2;
	writeWord(0, ax, DSeg);		// minutes west of UTC
	ax += 2;
	writeWord(1, ax, DSeg);		// DST
	setAX(0);
	return;
}

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
		switch (bx()) {
		case 1:	/* exit */
			printf("exit(%d)\n", ax());
			exit(ax());
			break;
		case 4: /* write */
			printf("write(%d, %#x, %d)\n", ax(), dx(), cx());
			write(1, &ram[physicalAddress(dx(), DSeg, false)], cx());
			setAX(cx());
			break;
		case 6: /* close */
			setAX(0);	// XXX
			break;
		case 13: /* time */
			venix_time();
			break;
		case 35: /* ftime */
			venix_ftime(ax());
			break;
		case 36: /* sync */
			printf("sync\n");
			sync();
			setCX(0);
			break;
		case 54: /* ioctl */
			setAX(0);	// XXX
			break;
		default:
			printf("Unimplemented system call %d\n", bx());
			exit(0);
		}
		break;
	default:
		printf("Unimplemented interrupt %#x\n", data);
		exit(0);
	}
}

};
