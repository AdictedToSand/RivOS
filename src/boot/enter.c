#include <int.h>

#include "storage/drver.h"
#include "term/term.h"
#include "string.h"
#include "utils.h"

#define RIVBOOT_MAGICSTR "BOOTABLE"

typedef struct [[gnu::packed]] RivBootHeader {
    char magic[9];
    u32 version;
    u32 entry;
    u32 kernelSize;
    u32 kernelStart;
    u32 osNamePtr;
} RivBootHeader;

int x86InstructionLength(const uint8_t* code) {
    int len = 0;

    // Prefixes
    while (true) {
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

static char buf[512];
void startBoot() {
    initTerm();
    storageInit();

    size_t sectorInd;
    for (size_t i = 0; i < 256; i++) {
        memset(buf, 0, 512);
        if (readSector(i, SECTOR_SIZE, buf)) panic("Unable to read sector");
        if (streq(RIVBOOT_MAGICSTR, buf)) {
            sectorInd = i;
            break;
        }
    }
    RivBootHeader* rivbootEntry = (RivBootHeader*) buf;

    const char* osName = (char*) &buf[rivbootEntry->osNamePtr - rivbootEntry->kernelStart];

    print("An operating system was found: %s\n", osName);

    const u32 kernelSectors = ((u32) rivbootEntry->kernelSize + SECTOR_SIZE - 1) / SECTOR_SIZE;

    for (u32 i = 0; i < kernelSectors; i++) {
        const u8* loadedAddr = (u8*) (rivbootEntry->kernelStart + (i * 512));
 
        if (readSector(sectorInd + i, SECTOR_SIZE, (char*) loadedAddr)) panic("Unable to load kernel");
    } 
    print("KernelEntry: %p\n", rivbootEntry->entry);
    print("RawInstr: %x", *(u32*) rivbootEntry->entry);

    void (*kernelStart)(void) = (void*) (rivbootEntry->entry + x86InstructionLength((u8*) rivbootEntry->entry) - 2); // What the fuck
    kernelStart();

    for (;;) ;
}
