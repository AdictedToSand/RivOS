#pragma once
#include <int.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct [[gnu::packed]] Fat32BootSector {
    i8 jumpInstruction[3];
    i8 oemIdentifier[8];
    u16 bytesPerSector;
    u8 sectorsPerCluster;
    u16 reservedSectors;
    u8 fatsOnMedia;
    u16 rootDirectoryEntries;
    u16 totalSectorsPerLogicalVolume;
    u8 mediaDescriptorType;
    u16 sectorsPerFat16;
    u16 sectorsPerTrack;
    u16 headsPerTrack;
    u32 amountOfHiddenSectors;
    u32 largeSectorCount;
    u32 sectorsPerFat32;
    u16 flags;
    u16 versionNumber;
    u32 rootDirClusterNum;
    u16 sectorNumOfFsinfo;
    u16 sectorNumberOfBackupBootSector;
    i8 reserved[12];
    u8 driveNumber;
    u8 reserved2;
    u8 signature;
    u32 volumeSerialNumberId;
    i8 volumeLabelString[11];
    i8 bootcode[420];
    u16 bootSignature;
} Fat32BootSector;

typedef struct Fat32FileData {
    u32 cluster;
    bool isDir;
} Fat32FileData;

typedef struct [[gnu::packed]] DirEntry {
    char name[8];
    char ext[3];
    u8 attributes;
    u8 reserved;
    u8 creationTimeTenths;
    u8 creationTime;
    u16 creationDate;
    u16 lastAccessData;
    u16 firstClusterHigh;
    u16 writeTime;
    u16 writeDate;
    u16 firstClusterLow;
    u32 filesize;
} DirEntry;

typedef struct File {
    void* fsData;
    size_t size;
    bool exists;
} File;

extern Fat32BootSector* bootSector;
extern u32 dataRegionStart;

static inline size_t clusterToSector(size_t cluster) {
    return dataRegionStart + (cluster - 2) * bootSector->sectorsPerCluster;
}
static inline u32 clusterOf(const DirEntry* const e) {
    return ((u32) e->firstClusterHigh << 16) | e->firstClusterLow;
}
static inline bool isRootPath(const char* fp) {
    if (!fp || *fp == '\0') return true;
    while (*fp == '/' || *fp == '\\') fp++;
    return *fp == '\0';
}

void fsInit();
File open(const char* fp);
void read(File f, char* obuf, size_t len);
