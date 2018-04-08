class Venix : public MachineOS
{
public:
	Venix() : length(0) {}
	~Venix() {}
private:
	int length;

void load(int argc, char **argv)
{
    filename = argv[1];
    FILE* fp = fopen(filename, "rb");
    if (fp == 0)
        error("opening");
    if (fseek(fp, 0, SEEK_END) != 0)
        error("seeking");
    length = ftell(fp);
    if (length == -1)
        error("telling");
    if (fseek(fp, 0, SEEK_SET) != 0)
        error("seeking");
    int loadOffset = loadSegment << 4;
    if (length > 0x100000 - loadOffset)
        length = 0x100000 - loadOffset;
    /*
     * Read the whole program into memory
     */
    if (fread(&ram[loadOffset], length, 1, fp) != 1)
        error("reading");
    fclose(fp);

    printf("Read in %d/%#x bytes at %#x\n", length, length, loadOffset);

    /*
     * Mark the memory in use.
     */
    for (int i = 0; i < length; ++i) {
        registers[ES] = loadSegment + (i >> 4);
        physicalAddress(i & 15, 0, true);
    }

    /*
     * Initialize all the segment registers to be the same.
     */
    for (int i = 0; i < 4; ++i)
	registers[FirstS + i] = loadSegment;

    int magic = readWord(0x0);
    printf("Magic is %x\n", magic);
    if (magic == 0x109)
	    error("Separate I/D binaries not yet supported\n");
    if (magic != 0x107)
	    error("Unknown type of binary\n");
    error("Not yet");
}

void int_cd(void)
{
}

};
