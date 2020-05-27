#include <stdlib.h>
#include <string.h>
#include "debug.hh"

typedef unsigned char Byte;
typedef unsigned short int Word;
typedef unsigned int DWord;

extern void runtimeError(const char* message);
extern void* alloc(size_t bytes);

#define AX 0
#define CX 1
#define DX 2
#define BX 3
#define SP 4
#define BP 5
#define SI 6
#define DI 7
#define ES 8
#define CS 9
#define SS 10
#define DS 11

#define FirstS 8
#define ESeg 0
#define CSeg 1
#define SSeg 2
#define DSeg 3

#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define AH 4
#define CH 5
#define DH 6
#define BH 7

class MachineOS
{
protected:
	MachineOS()
	{
		flags = 2;
		ip = 0x100;
		loadSegment = 0x0212;
		segmentOverride = -1;
		emu_base = NULL;
	}
	virtual ~MachineOS() { ::free((void *)emu_base); }

public:
	virtual void load(int argc, char **argv) = 0;
	virtual void int_cd() = 0;
	virtual void start_of_instruction() = 0;
	int run(void);
	void init(void);
	void set_emu_base(const char *base)	{ emu_base = ::strdup(base); }
protected:
	const char *emu_base;
	Byte* ram;
	Byte* initialized;
	bool wordSize;
	Word flags;
	Word ip;
	Word loadSegment;
	Word registers[12];
	Byte *byteRegisters[8];
	int stackLow;
	int ios;
	int segment;
	int segmentOverride;
	DWord data;
	Word remainder;
	Byte opcode;
	int aluOperation;
	bool running;
	int oCycle;
	DWord source;
	DWord destination;
	Byte modRM;
	Word address;
	int rep;
	bool repeating;
	bool useMemory;
	Word savedCS;
	Word savedIP;
protected:
Word ss() { return registers[SS]; }
Word es() { return registers[ES]; }
Word ds() { return registers[DS]; }
Word cs() { return registers[CS]; }
Word getFlags() { return flags; }
bool cf() { return (flags & 1) != 0; }
bool pf() { return (flags & 4) != 0; }
bool af() { return (flags & 0x10) != 0; }
bool zf() { return (flags & 0x40) != 0; }
bool sf() { return (flags & 0x80) != 0; }
void setIF(bool intf) { flags = (flags & ~0x200) | (intf ? 0x200 : 0); }
void setDF(bool df) { flags = (flags & ~0x400) | (df ? 0x400 : 0); }
bool df() { return (flags & 0x400) != 0; }
bool of() { return (flags & 0x800) != 0; }
Word rw() { return registers[opcode & 7]; }
Word ax() { return registers[AX]; }
Word cx() { return registers[CX]; }
Word dx() { return registers[DX]; }
Word bx() { return registers[BX]; }
Word sp() { return registers[SP]; }
Word bp() { return registers[BP]; }
Word si() { return registers[SI]; }
Word di() { return registers[DI]; }
Byte al() { return *byteRegisters[AL]; }
Byte cl() { return *byteRegisters[CL]; }
Byte dl() { return *byteRegisters[DL]; }
Byte bl() { return *byteRegisters[BL]; }
Byte ah() { return *byteRegisters[AH]; }
Byte ch() { return *byteRegisters[CH]; }
Byte dh() { return *byteRegisters[DH]; }
Byte bh() { return *byteRegisters[BH]; }
void setRW(Word value) { registers[opcode & 7] = value; }
void setAX(Word value) { registers[AX] = value; }
void setCX(Word value) { registers[CX] = value; }
void setDX(Word value) { registers[DX] = value; }
void setBX(Word value) { registers[BX] = value; }
void setSP(Word value) { registers[SP] = value; }
void setSI(Word value) { registers[SI] = value; }
void setDI(Word value) { registers[DI] = value; }
void setAL(Byte value) { *byteRegisters[AL] = value; }
void setCL(Byte value) { *byteRegisters[CL] = value; }
void setAH(Byte value) { *byteRegisters[AH] = value; }
void setRB(Byte value) { *byteRegisters[opcode & 7] = value; }
void setCS(Word value) { registers[CS] = value; }
	Byte readByte(Word offset, int seg = -1)
	{
	    return ram[physicalAddress(offset, seg, false)];
	}

	Word readWord(Word offset, int seg = -1)
	{
	    Word r = readByte(offset, seg);
	    return r + (readByte(offset + 1, seg) << 8);
	}
	Word read(Word offset, int seg = -1)
	{
	    return wordSize ? readWord(offset, seg) : readByte(offset, seg);
	}
	void writeByte(Byte value, Word offset, int seg)
	{
		ram[physicalAddress(offset, seg, true)] = value;
	}
	void writeWord(Word value, Word offset, int seg)
	{
		writeByte((Byte)value, offset, seg);
		writeByte((Byte)(value >> 8), offset + 1, seg);
	}
	void write(Word value, Word offset, int seg = -1)
	{
		if (wordSize)
			writeWord(value, offset, seg);
		else
			writeByte((Byte)value, offset, seg);
	}
	Byte fetchByte() { Byte b = readByte(ip, CSeg); ++ip; return b; }
	Word fetchWord() { Word w = fetchByte(); w += fetchByte() << 8; return w; }
	Word fetch(bool wordSize)
	{
		if (wordSize)
			return fetchWord();
		return fetchByte();
	}
DWord physicalAddress(Word offset, int seg, bool write)
{
    ++ios;
    if (ios == 0)
        runtimeError("Cycle counter overflowed.");
    if (seg == -1) {
        seg = segment;
        if (segmentOverride != -1)
            seg = segmentOverride;
    }
    Word segmentAddress = registers[FirstS + seg];
    DWord a = (((DWord)segmentAddress << 4) + offset) & 0xfffff;
    bool bad = false;
    if (write) {
        if (a < ((DWord)loadSegment << 4) - 0x100 && running)
             bad = true;
        initialized[a >> 3] |= 1 << (a & 7);
    }
    if ((initialized[a >> 3] & (1 << (a & 7))) == 0 || bad) {
        debug(dbg_error, "Accessing invalid address %04x:%04x.\n",
            segmentAddress, offset);
        runtimeError("");
    }
    return a;
}
private:
	void setZF()
	{
		flags = (flags & ~0x40) |
		    ((data & (!wordSize ? 0xff : 0xffff)) == 0 ? 0x40 : 0);
	}
	void setSF()
	{
		flags = (flags & ~0x80) |
		    ((data & (!wordSize ? 0x80 : 0x8000)) != 0 ? 0x80 : 0);
	}
	void setCF(bool cf) { flags = (flags & ~1) | (cf ? 1 : 0); }
	void setAF(bool af) { flags = (flags & ~0x10) | (af ? 0x10 : 0); }
	void clearCA() { setCF(false); setAF(false); }
	void setOF(bool of) { flags = (flags & ~0x800) | (of ? 0x800 : 0); }
	void clearCAO() { clearCA(); setOF(false); }
	void setPF()
	{
		static Byte table[0x100] = {
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
			4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4};
		flags = (flags & ~4) | table[data & 0xff];
	}
	void setPZS() { setPF(); setZF(); setSF(); }
	void bitwise(Word value) { data = value; clearCAO(); setPZS(); }
void o(char c)
{
#if 0
    while (oCycle < ios) {
        ++oCycle;
        printf(" ");
    }
    ++oCycle;
    printf("%c", c);
#endif
}

void divideOverflow() { runtimeError("Divide overflow"); }
Word signExtend(Byte data) { return data + (data < 0x80 ? 0 : 0xff00); }
int modRMReg() { return (modRM >> 3) & 7; }
void div()
{
    bool negative = false;
    bool dividendNegative = false;
    if (modRMReg() == 7) {
        if ((destination & 0x80000000) != 0) {
            destination = (unsigned)-(signed)destination;
            negative = !negative;
            dividendNegative = true;
        }
        if ((source & 0x8000) != 0) {
            source = (unsigned)-(signed)source & 0xffff;
            negative = !negative;
        }
    }
    data = destination / source;
    DWord product = data * source;
    // ISO C++ 2003 does not specify a rounding mode, but the x86 always
    // rounds towards zero.
    if (product > destination) {
        --data;
        product -= source;
    }
    remainder = destination - product;
    if (negative)
        data = (unsigned)-(signed)data;
    if (dividendNegative)
        remainder = (unsigned)-(signed)remainder;
}
void doJump(Word newIP)
{
#if 0
    printf("\n");
#endif
    ip = newIP;
}
void jumpShort(Byte data, bool jump)
{
    if (jump)
        doJump(ip + signExtend(data));
}
void test(Word d, Word s)
{
    destination = d;
    source = s;
    bitwise(destination & source);
}
int stringIncrement()
{
    int r = (wordSize ? 2 : 1);
    return !df() ? r : -r;
}
Word lodS()
{
    address = si();
    setSI(si() + stringIncrement());
    segment = DSeg;
    return read(address);
}
void doRep(bool compare)
{
    if (rep == 1 && !compare)
        runtimeError("REPNE prefix with non-compare string instruction");
    if (rep == 0 || cx() == 0)
        return;
    setCX(cx() - 1);
    repeating = cx() != 0 && (!compare || zf() != (rep == 1));
}
Word lodDIS()
{
    address = di();
    setDI(di() + stringIncrement());
    return read(address, 0);
}
void stoS(Word data)
{
    address = di();
    setDI(di() + stringIncrement());
    write(data, address, 0);
}
void push(Word value)
{
    o('{');
    setSP(sp() - 2);
    if (sp() <= stackLow)
        runtimeError("Stack overflow");
    writeWord(value, sp(), 2);
}
Word pop() { Word r = readWord(sp(), 2); setSP(sp() + 2); o('}'); return r; }
void setCA() { setCF(true); setAF(true); }
void doAF() { setAF(((data ^ source ^ destination) & 0x10) != 0); }
void doCF() { setCF((data & (!wordSize ? 0x100 : 0x10000)) != 0); }
void setCAPZS() { setPZS(); doAF(); doCF(); }
void setOFAdd()
{
    Word t = (data ^ source) & (data ^ destination);
    setOF((t & (!wordSize ? 0x80 : 0x8000)) != 0);
}
void add() { data = destination + source; setCAPZS(); setOFAdd(); }
void setOFSub()
{
    Word t = (destination ^ source) & (data ^ destination);
    setOF((t & (!wordSize ? 0x80 : 0x8000)) != 0);
}
void sub() { data = destination - source; setCAPZS(); setOFSub(); }
void setOFRotate()
{
    setOF(((data ^ destination) & (!wordSize ? 0x80 : 0x8000)) != 0);
}
void doALUOperation()
{
    switch (aluOperation) {
        case 0: add(); o('+'); break;
        case 1: bitwise(destination | source); o('|'); break;
        case 2: source += cf() ? 1 : 0; add(); o('a'); break;
        case 3: source += cf() ? 1 : 0; sub(); o('B'); break;
        case 4: test(destination, source); o('&'); break;
        case 5: sub(); o('-'); break;
        case 7: sub(); o('?'); break;
        case 6: bitwise(destination ^ source); o('^'); break;
    }
}
Word* modRMRW() { return &registers[modRMReg()]; }
Byte* modRMRB() { return byteRegisters[modRMReg()]; }
Word getReg()
{
    if (!wordSize)
        return *modRMRB();
    return *modRMRW();
}
Word getAccum() { return !wordSize ? al() : ax(); }
void setAccum() { if (!wordSize) setAL(data); else setAX(data);  }
void setReg(Word value)
{
    if (!wordSize)
        *modRMRB() = (Byte)value;
    else
        *modRMRW() = value;
}
Word ea()
{
    modRM = fetchByte();
    useMemory = true;
    switch (modRM & 7) {
        case 0: segment = DSeg; address = bx() + si(); break;
        case 1: segment = DSeg; address = bx() + di(); break;
        case 2: segment = SSeg; address = bp() + si(); break;
        case 3: segment = SSeg; address = bp() + di(); break;
        case 4: segment = DSeg; address =        si(); break;
        case 5: segment = DSeg; address =        di(); break;
        case 6: segment = SSeg; address = bp();        break;
        case 7: segment = DSeg; address = bx();        break;
    }
    switch (modRM & 0xc0) {
        case 0x00:
            if ((modRM & 0xc7) == 6) {
                segment = DSeg;
                address = fetchWord();
            }
            break;
        case 0x40: address += signExtend(fetchByte()); break;
        case 0x80: address += fetchWord(); break;
        case 0xc0:
            useMemory = false;
            address = modRM & 7;
    }
    return address;
}
Word readEA2()
{
    if (!useMemory) {
        if (wordSize)
            return registers[address];
        return *byteRegisters[address];
    }
    return read(address);
}
Word readEA() { address = ea(); return readEA2(); }
void finishWriteEA(Word data)
{
    if (!useMemory) {
        if (wordSize)
            registers[address] = data;
        else
            *byteRegisters[address] = (Byte)data;
    }
    else
        write(data, address);
}
void writeEA(Word data) { ea(); finishWriteEA(data); }
void farLoad()
{
    if (!useMemory)
        runtimeError("This instruction needs a memory address");
    savedIP = readWord(address);
    savedCS = readWord(address + 2);
}
void farJump() { setCS(savedCS); doJump(savedIP); }
void farCall() { push(cs()); push(ip); farJump(); }
Word incdec(bool decrement)
{
    source = 1;
    if (!decrement) {
        data = destination + source;
        setOFAdd();
    }
    else {
        data = destination - source;
        setOFSub();
    }
    doAF();
    setPZS();
    return data;
}
void call(Word address) { push(ip); doJump(address); }
void runtimeError(const char* message);
};
