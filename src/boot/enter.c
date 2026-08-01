#include <int.h>

#include "io.h"
#include "storage/drver.h"
#include "term/term.h"
#include "string.h"
#include "utils.h"
#include "menu/menu.h"

#define RIVBOOT_MAGICSTR "BOOTABLE"

typedef struct [[gnu::packed]] RivBootHeader {
    char magic[9];
    union {
        u32 full;
        u16 minorThenMajor[2];
    } version;
    u32 entry;
    u32 kernelSize;
    u32 kernelStart;
    u32 osNamePtr;
    u32 bssStartVers0_2;
    u32 bssEndVers0_2;
} RivBootHeader;

RivBootHeader* rivbootEntry;

void bootintoOS(MenuItem* this) {
    print("Booting into %s", this->str);
    void (*kernelStart)(void) = (void*) (rivbootEntry->entry + x86InstructionLength((u8*) rivbootEntry->entry) - 2); // What the fuck
    kernelStart();
}

void reboot(MenuItem* this) {
    (void) this; // Remove the param not used warning
    asm volatile ("UD2"); // Since we have not set up a IDT this will triple fault and reboot
}
void ioWaitAm(u32 am) {
    for (u32 i = 0; i < am; i++) ioWait();
}

void shutdown(MenuItem* this) {
    (void) this;
    clearTerm();
    print("Shutdown not supported currently. Press any key to continue ...");

    u8 sc;
    do {
        sc = getSc();
    } while (sc & 0x80);
}

static char buf[512];
void startBoot() {
    initTerm();
    storageInit();

    size_t sectorInd = -1;
    for (size_t i = 0; i < 256; i++) {
        memset(buf, 0, 512);
        if (readSector(i, SECTOR_SIZE, buf)) panic("Unable to read sector");
        if (streq(RIVBOOT_MAGICSTR, buf)) {
            sectorInd = i;
            break;
        }
    }
    rivbootEntry = (RivBootHeader*) buf;

    const char* osName = (char*) &buf[rivbootEntry->osNamePtr - rivbootEntry->kernelStart];

    print("An operating system was found: %s\n", osName);

    const u32 kernelSectors = ((u32) rivbootEntry->kernelSize + SECTOR_SIZE - 1) / SECTOR_SIZE;

    for (u32 i = 0; i < kernelSectors; i++) {
        const u8* loadedAddr = (u8*) (rivbootEntry->kernelStart + (i * 512));

        if (readSector(sectorInd + i, SECTOR_SIZE, (char*) loadedAddr))
            panic("Unable to load kernel");
    }

    if (rivbootEntry->version.minorThenMajor[0] == 2) {
        memset((void*) rivbootEntry->bssStartVers0_2, 0,
               rivbootEntry->bssEndVers0_2 - rivbootEntry->bssStartVers0_2);
    }
    
    MenuItem menuitems[] = {
        {
            .fmt = "%c) Boot into %s",
            .isLastItem = false,
            .onPressed = bootintoOS,
            .str = osName,
        },
        {
            .fmt = "%c) Reboot",
            .isLastItem = false,
            .onPressed = reboot,
        },
        {
            .fmt = "%c) Shutdown",
            .isLastItem = false,
            .onPressed = shutdown,
        },
        {
            .isLastItem = true,
        }
    };
    Menu men = {
        .str = osName,
        .items = menuitems,
    };
    displayMenu(&men,
    "Welcome to RivBoot\n"
    "An OS was found: %s\n"
    "Please select one of the following options:\n");

    for (;;) ;
}
