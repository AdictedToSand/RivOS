#include "fs.h"

#include "../storage/drver.h"
#include "../string.h"

#include "../mem/alloc.h"

#include "../term/term.h"

#include "../utils.h"

Fat32BootSector* bootSector;
u32 dataRegionStart;

u32 readFatEntry(u32 cluster) {
    u32 fatOffset = cluster * 4;
    u32 fatSector = bootSector->reservedSectors + (fatOffset / bootSector->bytesPerSector);
    u32 entryOffsetInSector = fatOffset % bootSector->bytesPerSector;

    u16 buf[256];
    // readSector(u32 sector, i16 max, char *obuf)
    readSector(fatSector, 512, (char*) buf);

    u32 entry = *(u32*) ((u8*) buf + entryOffsetInSector);
    return entry & 0x0FFFFFFF; 
}
void makeFatName(const char* normalName, char outbuf[12]) {
    for (u8 i = 0; i < 11; i++) outbuf[i] = ' ';
    outbuf[11] = '\0';

    i32 i = 0;
    u32 nameLen = 0;
    u32 extLen = 0;
    while (normalName[i] && normalName[i] != '/' && normalName[i] != '\\' && normalName[i] != '.') {
        if (nameLen >= 8) return;
        outbuf[nameLen++] = normalName[i++];
    }

    if (nameLen == 0) return;
    
    if (normalName[i] == '.') {
        i++;
        while (normalName[i] && normalName[i] != '/' && normalName[i] != '\\') {
            if (extLen >= 3) return;
            outbuf[8 + extLen++] = normalName[i++];
        }
    }

    strToUpper(outbuf);
}
void readCluster(u32 cluster, u16* buf) {
    u32 firstSector = clusterToSector(cluster);
    for (u32 s = 0; s < bootSector->sectorsPerCluster; s++) {
        readSector(firstSector + s, 512, (char*) (buf + (s * 256)));
    }
}
bool findDirEntryInDir(u32 startCluster, const char* filename, DirEntry* outEntry) {
    char fatName[12];
    makeFatName(filename, fatName);

    const u32 bytesPerCluster = bootSector->bytesPerSector * bootSector->sectorsPerCluster;
    u16* clusterBuf = alloc(bytesPerCluster);
    
    u32 currentCluster = startCluster;

    while (true) {
        readCluster(currentCluster, clusterBuf);

        DirEntry* entries = (DirEntry*) clusterBuf;
        u32 entriesPerCluster = bytesPerCluster / sizeof(DirEntry);

        for (u32 i = 0; i < entriesPerCluster; i++) {
            DirEntry* e = &entries[i];

            if ((u8) e->name[0] == 0x00) {
                return false;
            }
            if ((u8) e->name[0] == 0xE5) continue;
            if ((e->attributes & 0x0F) == 0x0F) continue;

            char onDiskName[12];
            for (int j = 0; j < 8; j++) onDiskName[j] = e->name[j];
            for (int j = 0; j < 3; j++) onDiskName[8 + j] = e->ext[j];
            onDiskName[11] = '\0';

            strToUpper(onDiskName);

            if (streqi(onDiskName, fatName)) {
                *outEntry = *e;
                return true;
            }
        }

        u32 next = readFatEntry(currentCluster);
        if (next >= 0x0FFFFFF8) return false;
        currentCluster = next;
    }
}
bool splitNextPathComponent(const char** path, char out[64]) {
    while (**path == '/' || **path == '\\') (*path)++;
    if (**path == '\0') return false;

    int i = 0;
    while (**path && **path != '/' && **path != '\\') {
        if (i >= 63) return false;
        out[i++] = *(*path)++;
    }

    out[i] = '\0';
    return true;
}
bool resolvePath(const char* fp, DirEntry* outEntry) {
    if (!fp || *fp == '\0') return false;

    u32 currentDirCluster = bootSector->rootDirClusterNum;
    const char* p = fp;
    char component[64];

    while (true) {
        bool hasComponent = splitNextPathComponent(&p, component);
        if (!hasComponent) return false;

        DirEntry found = {};
        if (!findDirEntryInDir(currentDirCluster, component, &found)) return false;

        while (*p == '/' || *p == '\\') p++;

        if (*p == '\0') {
            *outEntry = found;
            return true;
        }

        if ((found.attributes & 0x10) == 0) return false;
        currentDirCluster = clusterOf(&found);
    }
}

void fsInit(void) {
    u16* bootSectorBuf = alloc(sizeof(Fat32BootSector));
    
    readSector(0, 512, (char*) bootSectorBuf);
    bootSector = (Fat32BootSector*) bootSectorBuf;

    if (bootSector->signature != 0x28 && bootSector->signature != 0x29) {
        print("Invalid boot sector FAT32 signature. Execution will not continue. Signature: ");
        for (;;) asm volatile ("CLI; HLT");
    }

    print("We are here\n");
    dataRegionStart = bootSector->reservedSectors + (bootSector->fatsOnMedia * bootSector->sectorsPerFat32);

    print("Bytes/sector: %u, Sectors/Cluster: %u, root sector: %u, FAT size: %u\n", 
        bootSector->bytesPerSector, bootSector->sectorsPerCluster,
        bootSector->rootDirClusterNum, bootSector->sectorsPerFat32);
}
File open(const char* fp) {
    File ret = {};
    ret.fsData = alloc(sizeof(Fat32FileData));
    
    Fat32FileData* fd = (Fat32FileData*) ret.fsData;

    if (isRootPath(fp)) {
        fd->cluster = bootSector->rootDirClusterNum;
        fd->isDir = true;
        ret.exists = true;
        ret.size = 0;
        return ret;
    }

    DirEntry dirEntry = {};
    if (resolvePath(fp, &dirEntry)) {
        fd->cluster = clusterOf(&dirEntry);
        fd->isDir = (dirEntry.attributes & 0x10) != 0;
        ret.size = dirEntry.filesize;
        ret.exists = true;
    }
    return ret;
}
void read(File f, char* obuf, size_t len) {
    Fat32FileData* fd = (Fat32FileData*) f.fsData;
    if (!fd || fd->isDir) return;

    u32 bytesPerCluster = bootSector->bytesPerSector * bootSector->sectorsPerCluster;
    u16* clusterBuf = (u16*) alloc(bytesPerCluster);

    size_t bytesRemaining = len < f.size ? len : f.size;
    (void) obuf; (void) bytesRemaining; (void) len; (void) clusterBuf;
}
