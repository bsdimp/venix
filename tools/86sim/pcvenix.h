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
	registers[SP] = hdr.a_text + hdr.a_stack - 2;
	ip = 0;
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
		printf("Unimplemented system call %d\n", bx());
		exit(0);
	default:
		printf("Unimplemented interrupt %#x\n", data);
		exit(0);
	}
}

};
