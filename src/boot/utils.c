#include "utils.h"

int x86InstructionLength(const uint8_t* code) {
    int len = 0;

    // Prefixes
    while (1) {
        switch (code[len]) {
            case 0xF0: case 0xF2: case 0xF3:
            case 0x2E: case 0x36: case 0x3E:
            case 0x26: case 0x64: case 0x65:
            case 0x66: case 0x67:
                len++;
                break;
            default:
                goto opcode;
        }
    }

opcode:
    (void) 0; // No idea but it makes compiler happi
    u8 op = code[len++];

    // One-byte opcodes with immediate bytes
    switch (op) {
        // push imm8
        case 0x6A:
            return len + 1;

        // push imm32
        case 0x68:
            return len + 4;

        // mov r32, imm32
        case 0xB8 ... 0xBF:
            return len + 4;

        // xor eax, eax
        case 0x31:
            return len + 1; // ModR/M

        // call rel32
        case 0xE8:
            return len + 4;

        // jmp rel32
        case 0xE9:
            return len + 4;

        // jmp rel8
        case 0xEB:
            return len + 1;

        // ret
        case 0xC3:
            return len;

        // cli
        case 0xFA:
            return len;

        // hlt
        case 0xF4:
            return len;

        default:
            return -1; // unknown
    }
}


void* memset(void* ptr, int value, u32 count) {
    u8* p = (u8*) ptr;

    for (u32 i = 0; i < count; i++)
        p[i] = (u8) value;

    return ptr;
}

u8 getSc(void) {
    while ((inb(0x64) & 0x01) == 0) {
        // wait
    }
    return inb(0x60);
}
