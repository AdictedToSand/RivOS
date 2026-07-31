#pragma once
#include <stddef.h>
#include <int.h>

#define SECTOR_SIZE 512 
// If a storage device exceeds this, simulate it by splitting a sector up. (Same for the other way around)

typedef u8 (*StorageDriverReadSectorFunc)(u32 sector, u16 max, char* obuf);
typedef void (*StorageDriverInitFunc)(void);
typedef int (*StorageDriverGetPriority)();
typedef struct StorageDriver {
    StorageDriverGetPriority getPrior;
    StorageDriverInitFunc init;
    StorageDriverReadSectorFunc readSector;
    const char* name;
} StorageDriver;

extern StorageDriver* activeDriver;

void storageInit(void);
u8 readSector(u32 sector, i16 max, char* obuf);
const char* getStorageDriverName(void);

u8 ataReadSector(u32 sector, u16 maxlen, char* obuf);
