#include "drver.h"

#include "../io.h"
#include "../utils.h"

#define DATA_PORT 0x1F0
#define SECTORCOUNT_PORT 0x1F2
#define LBALO_PORT 0x1F3
#define LBAMI_PORT 0x1F4
#define LBAHI_PORT 0x1F5
#define DRIVESEL_PORT 0x1F6
#define CMD_PORT 0x1F7
#define STATUS_PORT 0x1F7

#define CMD_IDENTIFY 0xEc
#define CMD_READSECTORS 0x20

#define DRIVESEL_MASTERDRIVE 0xE0
#define DRIVESEL_SLAVEDRIVE 0xF0

#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08

void ataInit() {}

u8 ataReadSector(u32 sector, u16 maxlen, char* obuf) {
    //TODO: This is very bad!
    outb(DRIVESEL_PORT, DRIVESEL_SLAVEDRIVE | ((sector >> 24) & 0x0F));

    // ~400ns delay
    // This is recommended.
    inb(STATUS_PORT);
    inb(STATUS_PORT);
    inb(STATUS_PORT);
    inb(STATUS_PORT);

    outb(SECTORCOUNT_PORT, 1);
    outb(LBALO_PORT,  sector & 0xFF);
    outb(LBAMI_PORT, (sector >> 8) & 0xFF);
    outb(LBAHI_PORT, (sector >> 16) & 0xFF);
    outb(CMD_PORT, 0x20); 

    if (inb(STATUS_PORT) == 0) {
        panic("Drive did not exist");
    }
    while (inb(STATUS_PORT) & 0x80) ;
    while (!(inb(STATUS_PORT) & 0x08)) ;

    for (u16 i = 0; i < 256 && (i * 2 + 1) < maxlen; i++) {
        u16 data = inw(DATA_PORT);

        obuf[i * 2] = data & 0xFF;
        obuf[i * 2 + 1] = data >> 8;
    }
    return 0;
}

int ataGetPrior() {
    return 1; // For now
}

StorageDriver candidates[] = {
    {
        .getPrior = ataGetPrior,
        .init = ataInit,
        .readSector = ataReadSector,
        .name = "RivBoot_ATAPIO",
    },
};

u8 strgDriverInd = 0;

void storageInit(void) {
    u8 currentWinnerInd = 0;
    u8 latestPriority = 0;
    for (u8 i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        if (candidates[i].getPrior() > latestPriority) {
            latestPriority = candidates[i].getPrior();
            currentWinnerInd = i;
        }
    }

    strgDriverInd = currentWinnerInd;
    candidates[strgDriverInd].init();
}

u8 readSector(u32 sector, i16 max, char* obuf) {
    return candidates[strgDriverInd].readSector(sector, max, obuf);
}
const char* getStorageDriverName(void) {
    return candidates[strgDriverInd].name;
}
