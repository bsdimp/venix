#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debug.hh"

#include "machos.h"

void MachineOS::runtimeError(const char* message)
{
    debug(dbg_error, "%s\nCS:IP = %04x:%04x\n", message, cs(), ip);
    exit(1);
}

void* alloc(size_t bytes)
{
    void* r = malloc(bytes);
    if (r == 0) {
        debug(dbg_error, "Out of memory\n");
        exit(1);
    }
    memset(r, 0, bytes);
    return r;
}

void MachineOS::init(void)
{
    ram = (Byte*)alloc(0x100000);
    initialized = (Byte*)alloc(0x20000);
}


int MachineOS::run(void)
{
    MachineOS *mos = this;
    Word t;
    Byte* byteData = (Byte*)&registers[0];
    int bigEndian = 0;
    int byteNumbers[8] = {0, 2, 4, 6, 1, 3, 5, 7};
    for (int i = 0 ; i < 8; ++i)
        byteRegisters[i] = &byteData[byteNumbers[i] ^ bigEndian];
    running = true;
    bool prefix = false;
    while (1) {
        if (!repeating) {
            if (!prefix) {
                segmentOverride = -1;
                rep = 0;
            }
            prefix = false;
	    mos->start_of_instruction();
            opcode = fetchByte();
        }
        if (rep != 0 && (opcode < 0xa4 || opcode >= 0xb0 || opcode == 0xa8 ||
            opcode == 0xa9))
            runtimeError("REP prefix with non-string instruction");
        wordSize = ((opcode & 1) != 0);
        bool sourceIsRM = ((opcode & 2) != 0);
        int operation = (opcode >> 3) & 7;
        bool jump;
        switch (opcode) {
            case 0x00: case 0x01: case 0x02: case 0x03:
            case 0x08: case 0x09: case 0x0a: case 0x0b:
            case 0x10: case 0x11: case 0x12: case 0x13:
            case 0x18: case 0x19: case 0x1a: case 0x1b:
            case 0x20: case 0x21: case 0x22: case 0x23:
            case 0x28: case 0x29: case 0x2a: case 0x2b:
            case 0x30: case 0x31: case 0x32: case 0x33:
            case 0x38: case 0x39: case 0x3a: case 0x3b:  // alu rmv,rmv
                data = readEA();
                if (!sourceIsRM) {
                    destination = data;
                    source = getReg();
                }
                else {
                    destination = getReg();
                    source = data;
                }
                aluOperation = operation;
                doALUOperation();
                if (aluOperation != 7) {
                    if (!sourceIsRM)
                        finishWriteEA(data);
                    else
                        setReg(data);
                }
                break;
            case 0x04: case 0x05: case 0x0c: case 0x0d:
            case 0x14: case 0x15: case 0x1c: case 0x1d:
            case 0x24: case 0x25: case 0x2c: case 0x2d:
            case 0x34: case 0x35: case 0x3c: case 0x3d:  // alu accum,i
                destination = getAccum();
                source = !wordSize ? fetchByte() : fetchWord();
                aluOperation = operation;
                doALUOperation();
                if (aluOperation != 7)
                    setAccum();
                break;
            case 0x06: case 0x0e: case 0x16: case 0x1e:  // PUSH segreg
                push(registers[operation + 8]);
                break;
            case 0x07: case 0x17: case 0x1f:  // POP segreg
                registers[operation + 8] = pop();
                break;
            case 0x26: case 0x2e: case 0x36: case 0x3e:  // segment override
                segmentOverride = operation - 4;
                o("e%ZE"[segmentOverride]);
                prefix = true;
                break;
            case 0x27: case 0x2f:  // DA
                if (af() || (al() & 0x0f) > 9) {
                    data = al() + (opcode == 0x27 ? 6 : -6);
                    setAL(data);
                    setAF(true);
                    if ((data & 0x100) != 0)
                        setCF(true);
                }
                setCF(cf() || al() > 0x9f);
                if (cf())
                    setAL(al() + (opcode == 0x27 ? 0x60 : -0x60));
                wordSize = false;
                data = al();
                setPZS();
                o(opcode == 0x27 ? 'y' : 'Y');
                break;
            case 0x37: case 0x3f:  // AA
                if (af() || (al() & 0xf) > 9) {
                    setAL(al() + (opcode == 0x37 ? 6 : -6));
                    setAH(ah() + (opcode == 0x37 ? 1 : -1));
                    setCA();
                }
                else
                    clearCA();
                setAL(al() & 0x0f);
                o(opcode == 0x37 ? 'A' : 'u');
                break;
            case 0x40: case 0x41: case 0x42: case 0x43:
            case 0x44: case 0x45: case 0x46: case 0x47:
            case 0x48: case 0x49: case 0x4a: case 0x4b:
            case 0x4c: case 0x4d: case 0x4e: case 0x4f:  // incdec rw
                destination = rw();
                wordSize = true;
                setRW(incdec((opcode & 8) != 0));
                o((opcode & 8) != 0 ? 'i' : 'd');
                break;
            case 0x50: case 0x51: case 0x52: case 0x53:
            case 0x54: case 0x55: case 0x56: case 0x57:  // PUSH rw
                push(rw());
                break;
            case 0x58: case 0x59: case 0x5a: case 0x5b:
            case 0x5c: case 0x5d: case 0x5e: case 0x5f:  // POP rw
                setRW(pop());
                break;
            case 0xd8: case 0xd9: case 0xda: case 0xdb:
            case 0xdc: case 0xdd: case 0xde: case 0xdf:  // escape
                data = readEA();
		debug(dbg_emul, "Ignorning FPU\n");
		break;
            case 0x60: case 0x61: case 0x62: case 0x63:
            case 0x64: case 0x65: case 0x66: case 0x67:
            case 0x68: case 0x69: case 0x6a: case 0x6b:
            case 0x6c: case 0x6d: case 0x6e: case 0x6f:
            case 0xc0: case 0xc1: case 0xc8: case 0xc9:  // invalid
            case 0xcc: case 0xf0: case 0xf1: case 0xf4:  // INT 3, LOCK, HLT
            case 0xce: case 0x0f:  // INTO, POP CS
            case 0xe4: case 0xe5: case 0xe6: case 0xe7:
            case 0xec: case 0xed: case 0xee: case 0xef:  // IN, OUT
                debug(dbg_error, "Invalid opcode %02x", opcode);
                runtimeError("");
                break;
            case 0x70: case 0x71: case 0x72: case 0x73:
            case 0x74: case 0x75: case 0x76: case 0x77:
            case 0x78: case 0x79: case 0x7a: case 0x7b:
            case 0x7c: case 0x7d: case 0x7e: case 0x7f:  // Jcond cb
                switch (opcode & 0x0e) {
                    case 0x00: jump = of(); break;
                    case 0x02: jump = cf(); break;
                    case 0x04: jump = zf(); break;
                    case 0x06: jump = cf() || zf(); break;
                    case 0x08: jump = sf(); break;
                    case 0x0a: jump = pf(); break;
                    case 0x0c: jump = sf() != of(); break;
                    default:   jump = sf() != of() || zf(); break;
                }
                jumpShort(fetchByte(), jump == ((opcode & 1) == 0));
                o("MK[)=J(]GgpP<.,>"[opcode & 0xf]);
                break;
            case 0x80: case 0x81: case 0x82: case 0x83:  // alu rmv,iv
                destination = readEA();
                data = fetch(opcode == 0x81);
                if (opcode != 0x83)
                    source = data;
                else
                    source = signExtend(data);
                aluOperation = modRMReg();
                doALUOperation();
                if (aluOperation != 7)
                    finishWriteEA(data);
                break;
            case 0x84: case 0x85:  // TEST rmv,rv
                data = readEA();
                test(data, getReg());
                o('t');
                break;
            case 0x86: case 0x87:  // XCHG rmv,rv
                data = readEA();
                finishWriteEA(getReg());
                setReg(data);
                o('x');
                break;
            case 0x88: case 0x89:  // MOV rmv,rv
                ea();
                finishWriteEA(getReg());
                o('m');
                break;
            case 0x8a: case 0x8b:  // MOV rv,rmv
                setReg(readEA());
                o('m');
                break;
            case 0x8c:  // MOV rmw,segreg
                ea();
                wordSize = 1;
                finishWriteEA(registers[modRMReg() + 8]);
                o('m');
                break;
            case 0x8d:  // LEA
                address = ea();
                if (!useMemory)
                    runtimeError("LEA needs a memory address");
                setReg(address);
                o('l');
                break;
            case 0x8e:  // MOV segreg,rmw
                wordSize = 1;
                data = readEA();
                registers[modRMReg() + 8] = data;
                o('m');
                break;
            case 0x8f:  // POP rmw
                writeEA(pop());
                break;
            case 0x90: case 0x91: case 0x92: case 0x93:
            case 0x94: case 0x95: case 0x96: case 0x97:  // XCHG AX,rw
                data = ax();
                setAX(rw());
                setRW(data);
                o(";xxxxxxx"[opcode & 7]);
                break;
            case 0x98:  // CBW
                setAX(signExtend(al()));
                o('b');
                break;
            case 0x99:  // CWD
                setDX((ax() & 0x8000) == 0 ? 0x0000 : 0xffff);
                o('w');
                break;
            case 0x9a:  // CALL cp
                savedIP = fetchWord();
                savedCS = fetchWord();
                o('c');
                farCall();
                break;
	    case 0x9b:  // WAIT
		break;
            case 0x9c:  // PUSHF
                o('U');
                push((flags & 0x0fd7) | 0xf000);
                break;
            case 0x9d:  // POPF
                o('O');
                flags = pop() | 2;
                break;
            case 0x9e:  // SAHF
                flags = (flags & 0xff02) | ah();
                o('s');
                break;
            case 0x9f:  // LAHF
                setAH(flags & 0xd7);
                o('L');
                break;
            case 0xa0: case 0xa1:  // MOV accum,xv
                segment = DSeg;
                data = read(fetchWord());
                setAccum();
                o('m');
                break;
            case 0xa2: case 0xa3:  // MOV xv,accum
                segment = DSeg;
                write(getAccum(), fetchWord());
                o('m');
                break;
            case 0xa4: case 0xa5:  // MOVSv
                if (rep == 0 || cx() != 0)
                    stoS(lodS());
                doRep(false);
                o('4' + (opcode & 1));
                break;
            case 0xa6: case 0xa7:  // CMPSv
                if (rep == 0 || cx() != 0) {
                    destination = lodS();
                    source = lodDIS();
                    sub();
                }
                doRep(true);
                o('0' + (opcode & 1));
                break;
            case 0xa8: case 0xa9:  // TEST accum,iv
                data = fetch(wordSize);
                test(getAccum(), data);
                o('t');
                break;
            case 0xaa: case 0xab:  // STOSv
                if (rep == 0 || cx() != 0)
                    stoS(getAccum());
                doRep(false);
                o('8' + (opcode & 1));
                break;
            case 0xac: case 0xad:  // LODSv
                if (rep == 0 || cx() != 0) {
                    data = lodS();
                    setAccum();
                }
                doRep(false);
                o('2' + (opcode & 1));
                break;
            case 0xae: case 0xaf:  // SCASv
                if (rep == 0 || cx() != 0) {
                    destination = getAccum();
                    source = lodDIS();
                    sub();
                }
                doRep(true);
                o('6' + (opcode & 1));
                break;
            case 0xb0: case 0xb1: case 0xb2: case 0xb3:
            case 0xb4: case 0xb5: case 0xb6: case 0xb7:
                setRB(fetchByte());
                o('m');
                break;
            case 0xb8: case 0xb9: case 0xba: case 0xbb:
            case 0xbc: case 0xbd: case 0xbe: case 0xbf:  // MOV rv,iv
                setRW(fetchWord());
                o('m');
                break;
            case 0xc2: case 0xc3: case 0xca: case 0xcb:  // RET
                savedIP = pop();
                savedCS = (opcode & 8) == 0 ? cs() : pop();
                if (!wordSize)
                    setSP(sp() + fetchWord());
                o('R');
                farJump();
                break;
            case 0xc4: case 0xc5:  // LES/LDS
                ea();
                farLoad();
                *modRMRW() = savedIP;
                registers[FirstS + (!wordSize ? 0 : 3)] = savedCS;
                o("NT"[opcode & 1]);
                break;
            case 0xc6: case 0xc7:  // MOV rmv,iv
                ea();
                finishWriteEA(fetch(wordSize));
                o('m');
                break;
            case 0xcd:
		mos->int_cd();
                o('$');
                break;
            case 0xcf:  // IRET
                o('I');
                doJump(pop());
                setCS(pop());
                flags = pop() | 2;
                break;
            case 0xd0: case 0xd1: case 0xd2: case 0xd3:  // rot rmv,n
                data = readEA();
                if ((opcode & 2) == 0)
                    source = 1;
                else
                    source = cl();
                while (source != 0) {
                    destination = data;
                    switch (modRMReg()) {
                        case 0:  // ROL
                            data <<= 1;
                            doCF();
                            data |= (cf() ? 1 : 0);
                            setOFRotate();
                            break;
                        case 1:  // ROR
                            setCF((data & 1) != 0);
                            data >>= 1;
                            if (cf())
                                data |= (!wordSize ? 0x80 : 0x8000);
                            setOFRotate();
                            break;
                        case 2:  // RCL
                            data = (data << 1) | (cf() ? 1 : 0);
                            doCF();
                            setOFRotate();
                            break;
                        case 3:  // RCR
                            data >>= 1;
                            if (cf())
                                data |= (!wordSize ? 0x80 : 0x8000);
                            setCF((destination & 1) != 0);
                            setOFRotate();
                            break;
                        case 4:  // SHL
                        case 6:
                            data <<= 1;
                            doCF();
                            setOFRotate();
                            setPZS();
                            break;
                        case 5:  // SHR
                            setCF((data & 1) != 0);
                            data >>= 1;
                            setOFRotate();
                            setAF(true);
                            setPZS();
                            break;
                        case 7:  // SAR
                            setCF((data & 1) != 0);
                            data >>= 1;
                            if (!wordSize)
                                data |= (destination & 0x80);
                            else
                                data |= (destination & 0x8000);
                            setOFRotate();
                            setAF(true);
                            setPZS();
                            break;
                    }
                    --source;
                }
                finishWriteEA(data);
                o("hHfFvVvW"[modRMReg()]);
                break;
            case 0xd4:  // AAM
                data = fetchByte();
                if (data == 0)
                    divideOverflow();
                setAH(al() / data);
                setAL(al() % data);
                wordSize = true;
                setPZS();
                o('n');
                break;
            case 0xd5:  // AAD
                data = fetchByte();
                setAL(al() + ah()*data);
                setAH(0);
                setPZS();
                o('k');
                break;
            case 0xd6:  // SALC
                setAL(cf() ? 0xff : 0x00);
                o('S');
                break;
            case 0xd7:  // XLATB
                setAL(readByte(bx() + al()));
                o('@');
                break;
            case 0xe0: case 0xe1: case 0xe2:  // LOOPc cb
                setCX(cx() - 1);
                jump = (cx() != 0);
                switch (opcode) {
                    case 0xe0: if (zf()) jump = false; break;
                    case 0xe1: if (!zf()) jump = false; break;
                }
                o("Qqo"[opcode & 3]);
                jumpShort(fetchByte(), jump);
                break;
            case 0xe3:  // JCXZ cb
                o('z');
                jumpShort(fetchByte(), cx() == 0);
                break;
            case 0xe8:  // CALL cw
                data = fetchWord();
                o('c');
                call(ip + data);
                break;
            case 0xe9:  // JMP cw
                o('j');
		t = fetchWord();
                doJump(ip + t);
                break;
            case 0xea:  // JMP cp
                o('j');
                savedIP = fetchWord();
                savedCS = fetchWord();
                farJump();
                break;
            case 0xeb:  // JMP cb
                o('j');
                jumpShort(fetchByte(), true);
                break;
            case 0xf2: case 0xf3:  // REP
                o('r');
                rep = opcode == 0xf2 ? 1 : 2;
                prefix = true;
                break;
            case 0xf5:  // CMC
                o('\"');
                flags ^= 1;
                break;
            case 0xf6: case 0xf7:  // math rmv
                data = readEA();
                switch (modRMReg()) {
                    case 0: case 1:  // TEST rmv,iv
                        test(data, fetch(wordSize));
                        o('t');
                        break;
                    case 2:  // NOT iv
                        finishWriteEA(~data);
                        o('~');
                        break;
                    case 3:  // NEG iv
                        source = data;
                        destination = 0;
                        sub();
                        finishWriteEA(data);
                        o('_');
                        break;
                    case 4: case 5:  // MUL rmv, IMUL rmv
                        source = data;
                        destination = getAccum();
                        data = destination;
                        setSF();
                        setPF();
                        data *= source;
                        setAX(data);
                        if (!wordSize) {
                            if (modRMReg() == 4)
                                setCF(ah() != 0);
                            else {
                                if ((source & 0x80) != 0)
                                    setAH(ah() - destination);
                                if ((destination & 0x80) != 0)
                                    setAH(ah() - source);
                                setCF(ah() ==
                                    ((al() & 0x80) == 0 ? 0 : 0xff));
                            }
                        }
                        else {
                            setDX(data >> 16);
                            if (modRMReg() == 4) {
                                data |= dx();
                                setCF(dx() != 0);
                            }
                            else {
                                if ((source & 0x8000) != 0)
                                    setDX(dx() - destination);
                                if ((destination & 0x8000) != 0)
                                    setDX(dx() - source);
                                data |= dx();
                                setCF(dx() ==
                                    ((ax() & 0x8000) == 0 ? 0 : 0xffff));
                            }
                        }
                        setZF();
                        setOF(cf());
                        o("*#"[opcode & 1]);
                        break;
                    case 6: case 7:  // DIV rmv, IDIV rmv
                        source = data;
                        if (source == 0)
                            divideOverflow();
                        if (!wordSize) {
                            destination = ax();
                            if (modRMReg() == 6) {
                                div();
                                if (data > 0xff)
                                    divideOverflow();
                            }
                            else {
                                destination = ax();
                                if ((destination & 0x8000) != 0)
                                    destination |= 0xffff0000;
                                source = signExtend(source);
                                div();
                                if (data > 0x7f && data < 0xffffff80)
                                    divideOverflow();
                            }
                            setAH((Byte)remainder);
                            setAL(data);
                        }
                        else {
                            destination = (dx() << 16) + ax();
                            div();
                            if (modRMReg() == 6) {
                                if (data > 0xffff)
                                    divideOverflow();
                            }
                            else {
                                if (data > 0x7fff && data < 0xffff8000)
                                    divideOverflow();
                            }
                            setDX(remainder);
                            setAX(data);
                        }
                        o("/\\"[opcode & 1]);
                        break;
                }
                break;
            case 0xf8: case 0xf9:  // STC/CLC
                setCF(wordSize);
                o("\'`"[opcode & 1]);
                break;
            case 0xfa: case 0xfb:  // STI/CLI
                setIF(wordSize);
                o("!:"[opcode & 1]);
                break;
            case 0xfc: case 0xfd:  // STD/CLD
                setDF(wordSize);
                o("CD"[opcode & 1]);
                break;
            case 0xfe: case 0xff:  // misc
                ea();
                if ((!wordSize && modRMReg() >= 2 && modRMReg() <= 6) ||
                    modRMReg() == 7) {
                    debug(dbg_error, "Invalid instruction %02x %02x", opcode,
                        modRM);
                    runtimeError("");
                }
                switch (modRMReg()) {
                    case 0: case 1:  // incdec rmv
                        destination = readEA2();
                        finishWriteEA(incdec(modRMReg() != 0));
                        o("id"[modRMReg() & 1]);
                        break;
                    case 2:  // CALL rmv
                        o('c');
                        call(readEA2());
                        break;
                    case 3:  // CALL mp
                        o('c');
                        farLoad();
                        farCall();
                        break;
                    case 4:  // JMP rmw
                        o('j');
                        doJump(readEA2());
                        break;
                    case 5:  // JMP mp
                        o('j');
                        farLoad();
                        farJump();
                        break;
                    case 6:  // PUSH rmw
                        push(readEA2());
                        break;
                }
                break;
        }
    }
    return (1);
}
