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

    void (*kernelStart)(void) = (void*) (rivbootEntry->entry + x86InstructionLength((u8*) rivbootEntry->entry) - 2); // What the fuck
    kernelStart();

    for (;;) ;
}
