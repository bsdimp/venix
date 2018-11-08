typedef unsigned char Byte;
typedef unsigned short int Word;
typedef unsigned int DWord;

extern Word ip;
extern Word loadSegment;
extern Byte *ram;
extern Byte *initialized;
extern Word registers[];
extern Byte *byteRegisters[];
extern int stackLow;
extern int ios;
extern int segment;
extern DWord data;
extern const char *filename;

extern DWord physicalAddress(Word offset, int seg, bool write);
extern Byte readByte(Word offset, int seg = -1);
extern Word readWord(Word offset, int seg = -1);
extern void writeByte(Byte value, Word offset, int seg = -1);
extern void writeWord(Word value, Word offset, int seg = -1);
extern Byte fetchByte();
extern Word fetchWord();
extern void setAX(Word value);
extern void setBX(Word value);
extern void setCX(Word value);
extern void setDX(Word value);
extern void setDI(Word value);
extern void setSI(Word value);
extern void setSP(Word value);
extern void setAH(Byte value);
extern void setAL(Byte value);
extern void setCF(bool);
extern Word getFlags();
extern Byte al();
extern Byte ah();
extern Word ax();
extern Word bx();
extern Word cx();
extern Word dx();
extern Word cs();
extern Word ds();
extern Word es();
extern Word ss();
extern Word bp();
extern Word sp();
extern Word si();
extern Word di();
extern void error(const char* operation);
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
	MachineOS()	     {}
	virtual ~MachineOS() {}

public:
	virtual void load(int argc, char **argv) = 0;
	virtual void int_cd() = 0;
	virtual void start_of_instruction() = 0;
};
