class IBMPC_DOS : public MachineOS
{
private:
char* pathBuffers[2];
int* fileDescriptors;
int fileDescriptorCount;
public:
	IBMPC_DOS() : fileDescriptorCount(6) { }
	~IBMPC_DOS() {}
private:
	int length;

int getDescriptor()
{
    for (int i = 0; i < fileDescriptorCount; ++i)
        if (fileDescriptors[i] == -1)
            return i;
    int newCount = fileDescriptorCount << 1;
    int* newDescriptors = (int*)alloc(newCount*sizeof(int));
    for (int i = 0; i < fileDescriptorCount; ++i)
        newDescriptors[i] = fileDescriptors[i];
    free(fileDescriptors);
    int oldCount = fileDescriptorCount;
    fileDescriptorCount = newCount;
    fileDescriptors = newDescriptors;
    return oldCount;
}

char* initString(Word offset, int seg, bool write, int buffer,
    int bytes = 0x10000)
{
    for (int i = 0; i < bytes; ++i) {
        char p;
        if (write) {
            p = pathBuffers[buffer][i];
            ram[physicalAddress(offset + i, seg, true)] = p;
        }
        else {
            p = ram[physicalAddress(offset + i, seg, false)];
            pathBuffers[buffer][i] = p;
        }
        if (p == 0 && bytes == 0x10000)
            break;
    }
    if (!write)
        pathBuffers[buffer][0xffff] = 0;
    return pathBuffers[buffer];
}

char* dsdx(bool write = false, int bytes = 0x10000)
{
    return initString(dx(), 3, write, 0, bytes);
}
void dos_display_string()
{
	Word offset = dx();
	char ch;

	while ((ch = ram[physicalAddress(offset, 3, false)]) != '$')  {
		putchar(ch);
		offset++;
	}
}
int dosError(int e)
{
    if (e == ENOENT)
        return 2;
    fprintf(stderr, "%s\n", strerror(e));
    runtimeError("");
    return 0;
}

void load(int argc, char **argv)
{
    filename = argv[1];
    FILE* fp = fopen(filename, "rb");
    if (fp == 0)
        error("opening");
    pathBuffers[0] = (char*)alloc(0x10000);
    pathBuffers[1] = (char*)alloc(0x10000);
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
     * Setup the environment and command line args
     */
    int envSegment = loadSegment - 0x1c;
    registers[ES] = envSegment;
    writeByte(0, 0);  // No environment for now
    writeWord(1, 1);
    int i;
    for (i = 0; filename[i] != 0; ++i)
        writeByte(filename[i], i + 3);
    if (i + 4 >= 0xc0) {
        fprintf(stderr, "Program name too long.\n");
        exit(1);
    }
    writeWord(0, i + 3);
    registers[ES] = loadSegment - 0x10;
    writeWord(envSegment, 0x2c);
    i = 0x81;
    for (int a = 2; a < argc; ++a) {
        if (a > 2) {
            writeByte(' ', i);
            ++i;
        }
        char* arg = argv[a];
        bool quote = strchr(arg, ' ') != 0;
        if (quote) {
            writeByte('\"', i);
            ++i;
        }
        for (; *arg != 0; ++arg) {
            int c = *arg;
            if (c == '\"') {
                writeByte('\\', i);
                ++i;
            }
            writeByte(c, i);
            ++i;
        }
        if (quote) {
            writeByte('\"', i);
            ++i;
        }
    }
    if (i > 0xff) {
        fprintf(stderr, "Arguments too long.\n");
        exit(1);
    }
    writeWord(0x9fff, 2);
    writeByte(i - 0x81, 0x80);
    writeByte(13, i);
    if (fread(&ram[loadOffset], length, 1, fp) != 1)
        error("reading");
    fclose(fp);
    for (int i = 0; i < length; ++i) {
        registers[ES] = loadSegment + (i >> 4);
        physicalAddress(i & 15, 0, true);
    }
    for (int i = 0; i < 4; ++i)
	registers[FirstS + i] = loadSegment - 0x10;
    if (length >= 2 && readWord(0x100) == 0x5a4d) {  // .exe file?
        if (length < 0x21) {
            fprintf(stderr, "%s is too short to be an .exe file\n", filename);
            exit(1);
        }
        Word bytesInLastBlock = readWord(0x102);
        int exeLength = ((readWord(0x104) - (bytesInLastBlock == 0 ? 0 : 1))
            << 9) + bytesInLastBlock;
        int headerParagraphs = readWord(0x108);
        int headerLength = headerParagraphs << 4;
        if (exeLength > length || headerLength > length ||
            headerLength > exeLength) {
            fprintf(stderr, "%s is corrupt\n", filename);
            exit(1);
        }
        int relocationCount = readWord(0x106);
        Word imageSegment = loadSegment + headerParagraphs;
        int relocationData = readWord(0x118);
        for (int i = 0; i < relocationCount; ++i) {
            int offset = readWord(relocationData + 0x100);
            registers[CS] = readWord(relocationData + 0x102) + imageSegment;
            writeWord(readWord(offset, 1) + imageSegment, offset, 1);
            relocationData += 4;
        }
        loadSegment = imageSegment;  // Prevent further access to header
        Word ss = readWord(0x10e) + loadSegment;  // SS
        registers[SS] = ss;
        setSP(readWord(0x110));
        stackLow =
            ((((exeLength - headerLength + 15) >> 4) + loadSegment) - ss) << 4;
        if (stackLow < 0x10)
            stackLow = 0x10;
        ip = readWord(0x114);
        registers[CS] = readWord(0x116) + loadSegment;  // CS
    }
    else {
        if (length > 0xff00) {
            fprintf(stderr, "%s is too long to be a .com file\n", filename);
            exit(1);
        }
        setSP(0xFFFE);
        stackLow = length + 0x100;
    }
    // Some testcases copy uninitialized stack data, so mark as initialized
    // any locations that could possibly be stack.
    if (sp()) {
        for (Word d = stackLow; d < sp(); ++d)
            writeByte(0, d, 2);
    } else {
        Word d = 0;
        while (--d >= stackLow)
            writeByte(0, d, 2);
    }
#if 1
    // Fill up parts of the interrupt vector table, the BIOS clock tick count,
    // and parts of the BIOS ROM area with stuff, for the benefit of the far
    // pointer tests.
    registers[ES] = 0x0000;
    writeWord(0x0000, 0x0080);
    writeWord(0xFFFF, 0x0082);
    writeWord(0x0058, 0x046C);
    writeWord(0x000C, 0x046E);
    writeByte(0x00, 0x0470);
    registers[ES] = 0xF000;
    for (i = 0; i < 0x100; i += 2)
        writeWord(0xF4F4, 0xFF00 + (unsigned)i);
    // We need some variety in the ROM BIOS content...
    writeByte(0xEA, 0xFFF0);
    writeWord(0xFFF0, 0xFFF1);
    writeWord(0xF000, 0xFFF3);
#endif
    ios = 0;
    registers[ES] = loadSegment - 0x10;
    setAX(0x0000);
    setCX(0x00FF);
    setDX(segment);
    registers[BX] = 0x0000;  // BX
    registers[BP] = 0x091C;  // BP
    setSI(0x0100);
    setDI(0xFFFE);
    fileDescriptors = (int*)alloc(6*sizeof(int));
    fileDescriptors[0] = STDIN_FILENO;
    fileDescriptors[1] = STDOUT_FILENO;
    fileDescriptors[2] = STDERR_FILENO;
    fileDescriptors[3] = STDOUT_FILENO;
    fileDescriptors[4] = STDOUT_FILENO;
    fileDescriptors[5] = -1;
}

void int_cd(void)
{
        int fileDescriptor;

	data = fetchByte();
	switch (data << 8 | ah()) {
        case 0x1a00:
		data = registers[ES];
		registers[ES] = 0;
		setDX(readWord(0x046c, 0));
		setCX(readWord(0x046e, 0));
		setAL(readByte(0x0470, 0));
		registers[ES] = data;
		break;
        case 0x2109:
		dos_display_string();
		setCF(false);
		break;
        case 0x2130:
		setAX(0x1403);
		setBX(0xff00);
		setCX(0);
		break;
        case 0x2139:
		if (mkdir(dsdx(), 0700) == 0)
			setCF(false);
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
        case 0x213a:
		if (rmdir(dsdx()) == 0)
			setCF(false);
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
        case 0x213b:
		if (chdir(dsdx()) == 0)
			setCF(false);
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	case 0x213c:
		fileDescriptor = creat(dsdx(), 0700);
		if (fileDescriptor != -1) {
			setCF(false);
			int guestDescriptor = getDescriptor();
			setAX(guestDescriptor);
			fileDescriptors[guestDescriptor] = fileDescriptor;
		}
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	case 0x213d:
		fileDescriptor = open(dsdx(), al() & 3, 0700);
		if (fileDescriptor != -1) {
			setCF(false);
			setAX(getDescriptor());
			fileDescriptors[ax()] = fileDescriptor;
		}
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	case 0x213e:
		fileDescriptor = fileDescriptors[bx()];
		if (fileDescriptor == -1) {
			setCF(true);
			setAX(6);  // Invalid handle
			break;
		}
		if (fileDescriptor >= 5 &&
		    close(fileDescriptor) != 0) {
			setCF(true);
			setAX(dosError(errno));
		}
		else {
			fileDescriptors[bx()] = -1;
			setCF(false);
		}
		break;
	case 0x213f:
		fileDescriptor = fileDescriptors[bx()];
		if (fileDescriptor == -1) {
			setCF(true);
			setAX(6);  // Invalid handle
			break;
		}
		data = read(fileDescriptor, pathBuffers[0], cx());
		dsdx(true, cx());
		if (data == (DWord)-1) {
			setCF(true);
			setAX(dosError(errno));
		}
		else {
			setCF(false);
			setAX(data);
		}
		break;
	case 0x2140:
		fileDescriptor = fileDescriptors[bx()];
		if (fileDescriptor == -1) {
			setCF(true);
			setAX(6);  // Invalid handle
			break;
		}
		data = write(fileDescriptor, dsdx(false, cx()), cx());
		if (data == (DWord)-1) {
			setCF(true);
			setAX(dosError(errno));
		}
		else {
			setCF(false);
			setAX(data);
		}
		break;
	case 0x2141:
		if (unlink(dsdx()) == 0)
			setCF(false);
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	case 0x2142:
		fileDescriptor = fileDescriptors[bx()];
		if (fileDescriptor == -1) {
			setCF(true);
			setAX(6);  // Invalid handle
			break;
		}
		data = lseek(fileDescriptor, (cx() << 16) + dx(),
		    al());
		if (data != (DWord)-1) {
			setCF(false);
			setDX(data >> 16);
			setAX(data);
		}
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	case 0x2144:
		if (al() != 0) {
			fprintf(stderr, "Unknown IOCTL 0x%02x", al());
			runtimeError("");
		}
		fileDescriptor = fileDescriptors[bx()];
		if (fileDescriptor == -1) {
			setCF(true);
			setAX(6);  // Invalid handle
			break;
		}
		data = isatty(fileDescriptor);
		if (data == 1) {
			setDX(0x80);
			setCF(false);
		}
		else {
			if (errno == ENOTTY) {
                                setDX(0);
                                setCF(false);
			}
			else {
                                setAX(dosError(errno));
                                setCF(true);
			}
		}
		break;
	case 0x2147:
		if (getcwd(pathBuffers[0], 64) != 0) {
			setCF(false);
			initString(si(), 3, true, 0);
		}
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	case 0x214c:
		printf("*** Bytes: %i\n", length);
		printf("*** Cycles: %i\n", ios);
		printf("*** EXIT code %i\n", al());
		exit(0);
		break;
	case 0x2156:
		if (rename(dsdx(), initString(di(), 0, false, 1)) == 0)
			setCF(false);
		else {
			setCF(true);
			setAX(dosError(errno));
		}
		break;
	default:
		fprintf(stderr, "Unknown DOS/BIOS call: int 0x%02x, "
		    "ah = 0x%02x", (unsigned)data, (unsigned)ah());
		runtimeError("");
	}
}

};
