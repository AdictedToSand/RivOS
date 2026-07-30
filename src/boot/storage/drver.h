#pragma once
#include <stddef.h>
#include <int.h>

#define SECTOR_SIZE 512 
// If a storage device exceeds this, simulate it by splitting a sector up. (Same for the other way around)

typedef void (*StorageDriverReadSectorFunc)(u32 sector, i16 max, char* obuf);
typedef struct StorageDriver {
    StorageDriverReadSectorFunc readSector;
} StorageDriver;

extern StorageDriver* activeDriver;

void storageInit(void);
