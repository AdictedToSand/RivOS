#include <int.h>

#include "lang.h"

#include "../sys/stdio.h"

//const u8 opcodeWithoutSubOpcode = inputted & OPCODE_FULL
// if (opcodeWithoutSubOpcode == (MOV << 2))) (2 == the size of a subopcode)
u8 toOpcode(u8 byte) {
    const u8 opcodeRaw = byte & OPCODE_FULL;    
    return opcodeRaw >> 2;
}
u8 toSubOpcode(u8 byte) {
    const u8 subopcodeRaw = byte & SUBOPCODE_FULL;
    return subopcodeRaw;
}

const char* opcodeToStr(u8 op) {
    const u8 withoutSub = toOpcode(op);

    switch (withoutSub) {
        case BIN: return "BIN";
        case LOAD: return "LOAD";
        case SET_FRAG: return "SET_FRAG";
        case PUSH: return "PUSH";
        case POP: return "POP";
        case RET: return "RET";
        case CALL: return "CALL";
        case LOAD_X: return "LOAD_X";
        case LOAD_Y: return "LOAD_Y";
        case JMP: return "JMP";
        case JCOR: return "JCOR";
        case JNCOR: return "JNCOR";
        case CMP: return "CMP";
        case CALL_BUILTIN: return "CALL_BUILTIN";
        case CVAR: return "CVAR";
        case DVAR: return "DVAR";
        case MOV: return "MOV";
        default: return "UNDEFINED_OPCODE";
    }
}
const char* subopcodeToStr(u8 opcode) {
    const u8 sub = toSubOpcode(opcode);
    const u8 op = toOpcode(opcode);

    switch (op) {
        case BIN:
            switch (sub) {
                case BIN_ONLY: return "var";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case LOAD:
            switch (sub) {
                case LOAD_CONST: return "const";
                case LOAD_REG: return "reg";
                case LOAD_VAR: return "var";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case SET_FRAG:
            switch (sub) {
                case SET_FRAG_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case PUSH:
            switch (sub) {
                case PUSH_CONST: return "const";
                case PUSH_REG: return "reg";
                case PUSH_VAR: return "var";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case POP:
            switch (sub) {
                case POP_REG: return "reg";
                case POP_VAR: return "var";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case RET:
            switch (sub) {
                case RET_ONLY: return "none";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case CALL:
            switch (sub) {
                case CALL_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case LOAD_X:
            switch (sub) {
                case LOAD_X_ONLY: return "reg";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case LOAD_Y:
            switch (sub) {
                case LOAD_Y_ONLY: return "reg";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case JMP:
            switch (sub) {
                case JMP_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case JCOR:
            switch (sub) {
                case JCOR_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case JNCOR:
            switch (sub) {
                case JNCOR_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case CMP:
            switch (sub) {
                case CMP_ONLY: return "none";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case CALL_BUILTIN:
            switch (sub) {
                case CALL_BUILTIN_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case CVAR:
            switch (sub) {
                case CVAR_ONLY: return "(u8) const, (u32=default) const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case DVAR:
            switch (sub) {
                case DVAR_ONLY: return "const";
                default: return "UNDEFINED_SUBOPCODE";
            }

        case MOV:
            switch (sub) {
                case MOV_REGTOCONST: return "regToConst";
                case MOV_REGTOMEM: return "regToMem";
                case MOV_REGTOREG: return "regToReg";
                case MOV_MEMTOREG: return "memToReg";
                default: return "UNDEFINED_SUBOPCODE";
            }

        default:
            return "UNDEFINED_OPCODE";
    }
}

// Returns the size in BYTES
u8 getOperandSize(u8 opcode) {
    const u8 subOpcode = toSubOpcode(opcode);
    const u8 mainOpcode = toOpcode(opcode);

    switch (mainOpcode) {
        case RET: case CMP: return 0; // None
        case DVAR: return 4; // Ptr
        case CVAR: return 5; // Size,Place
        case CALL_BUILTIN: return 1; // funcNum
        case LOAD_X: case LOAD_Y: return 1; // Register
        case POP: {
            switch (subOpcode) {
                case POP_REG: return 1;
                case POP_VAR: return 4;
                default: return -1;
            }
        }
        case PUSH: {
            switch (subOpcode) {
                case PUSH_CONST: return 4;
                case PUSH_REG: return 1;
                case PUSH_VAR: return 4;
            }
        }
        case BIN: return 1; // Register
        case LOAD: {
            switch (subOpcode) {
                case LOAD_CONST: return 4;
                case LOAD_VAR: return 4;
                case LOAD_REG: return 1;
            }
        }
        case SET_FRAG: case CALL: case JMP: case JCOR: case JNCOR: return 4; // Line number
        case MOV: {
            switch (subOpcode) {
                case MOV_REGTOCONST: return 5; 
                case MOV_REGTOMEM: return 5;
                case MOV_REGTOREG: return 2;
                case MOV_MEMTOREG: return 5;
            }
        }
    }

    return -1;
}

void decompile(const u8* src) {
    while (*src) {
        const u8 opcode = *src;

        const char* opcodeAsStr = opcodeToStr(opcode);
        const char* subopcodeAsStr = subopcodeToStr(opcode);

        printf("%s (%s) ", opcodeAsStr, subopcodeAsStr);
        src++;

        const u32 opSize = getOperandSize(opcode);
        const u8 intAm = opSize / sizeof(u32); // 4
        u8 byteAm = opSize; // n / sizeof(u8) == n / 1 == n
        byteAm = byteAm - intAm * 4;
        u8 i = 0;
        for (; i < intAm; i += sizeof(u32)) {
            printf("0x%x ", *(u32*) src);
            src += sizeof(u32); // 4
        }
        for (i = 0; i < byteAm; i++) {
            printf("%u ", *src);
            src += sizeof(u8); // 1
        }

        puts("\n");
    } 
}

void interpret(const u8* src) {
    u32 regL1 = 0, regL2 = 0, regP1 = 0, regP2 = 0, regTmp1 = 0, regTmp2 = 0, regOCR = 0, regRes = 0;

    while (*src) {
        switch (*src) {
            case BIN: {
                src++; // Opcode
                const u8 outreg = *src;
                switch (regOCR) {
                    case BIN_PLUS: {
                        
                        break;
                    } 
                }
                break;
            }
            default: src++; break;
        }
    }
}
