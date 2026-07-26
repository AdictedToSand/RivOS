#pragma once
#include <drivers/fs/driver.hpp>
#include <drivers/storage/storage.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <gen/alpha.hpp>
#include <terminal/terminal.hpp>

#include <stdint.h>

struct Fat32 : FileSystemDriver {
private:
    struct [[gnu::packed]] BootSector {
        char jumpInstruction[3];
        char oemIdentifier[8];
        uint16_t bytesPerSector;
        uint8_t sectorsPerCluster;
        uint16_t reservedSectors;
        uint8_t fatsOnMedia;
        uint16_t rootDirectoryEntries;
        uint16_t totalSectorsPerLogicalVolume;
        uint8_t mediaDescriptorType;
        uint16_t sectorsPerFat16;
        uint16_t sectorsPerTrack;
        uint16_t headsPerTrack;
        uint32_t amountOfHiddenSectors;
        uint32_t largeSectorCount;
        uint32_t sectorsPerFat32;
        uint16_t flags;
        uint16_t versionNumber;
        uint32_t rootDirClusterNum;
        uint16_t sectorNumOfFsinfo;
        uint16_t sectorNumberOfBackupBootSector;
        char reserved[12];
        uint8_t driveNumber;
        uint8_t reserved2;
        uint8_t signature;
        uint32_t volumeSerialNumberId;
        char volumeLabelString[11];
        char bootcode[420];
    };

    struct FileData {
        uint32_t cluster;
        bool isDirectory;
    };

    struct [[gnu::packed]] DirEntry {
        char name[8];
        char ext[3];
        uint8_t attributes;
        uint8_t reserved;
        uint8_t creationTimeTenths;
        uint16_t creationTime;
        uint16_t creationDate;
        uint16_t lastAccessData;
        uint16_t firstClusterHigh;
        uint16_t writeTime;
        uint16_t writeDate;
        uint16_t firstClusterLow;
        uint32_t filesize;
    };

    static inline BootSector* bootSector;
    static inline uint32_t dataRegionStart;

    auto clusterToSector(size_t cluster) -> size_t {
        return dataRegionStart + (cluster - 2) * bootSector->sectorsPerCluster;
    }

    auto clusterOf(const DirEntry& e) -> uint32_t {
        return ((uint32_t)e.firstClusterHigh << 16) | e.firstClusterLow;
    }
    auto isRootPath(const char* fp) -> bool {
        if (fp == nullptr || *fp == '\0') return true;
        while (*fp == '/' || *fp == '\\') fp++;
        return *fp == '\0';
    }

    auto readFatEntry(uint32_t cluster) -> uint32_t {
        uint32_t fatOffset = cluster * 4;
        uint32_t fatSector = bootSector->reservedSectors + (fatOffset / bootSector->bytesPerSector);
        uint32_t entryOffsetInSector = fatOffset % bootSector->bytesPerSector;

        uint16_t buf[256];
        Storage::readSector(buf, 256, fatSector);

        uint32_t entry = *(uint32_t*)((uint8_t*)buf + entryOffsetInSector);
        return entry & 0x0FFFFFFF;
    }

    auto makeFatName(const char* normalName, char outbuf[12]) -> bool {
        for (int i = 0; i < 11; i++) outbuf[i] = ' ';
        outbuf[11] = '\0';

        int i = 0;
        int nameLen = 0;
        int extLen = 0;

        while (normalName[i] && normalName[i] != '/' && normalName[i] != '\\' && normalName[i] != '.') {
            if (nameLen >= 8) return false;
            outbuf[nameLen++] = normalName[i++];
        }

        if (nameLen == 0) return false;

        if (normalName[i] == '.') {
            i++;
            while (normalName[i] && normalName[i] != '/' && normalName[i] != '\\') {
                if (extLen >= 3) return false;
                outbuf[8 + extLen++] = normalName[i++];
            }
        }

        strToUpper(outbuf);
        return true;
    }

    auto readCluster(uint32_t cluster, uint16_t* buffer) -> void {
        uint32_t firstSector = clusterToSector(cluster);
        for (uint32_t s = 0; s < bootSector->sectorsPerCluster; s++) {
            Storage::readSector(buffer + (s * 256), 256, firstSector + s);
        }
    }

    auto findDirEntryInDir(uint32_t startCluster, const char* filename, DirEntry* outEntry) -> bool {
        char fatName[12];
        if (!makeFatName(filename, fatName)) return false;

        const uint32_t bytesPerCluster = bootSector->bytesPerSector * bootSector->sectorsPerCluster;
        uint16_t* clusterBuf = (uint16_t*)KernelAllocator::alloc(bytesPerCluster);
        if (clusterBuf == nullptr) return false;

        uint32_t currentCluster = startCluster;

        while (true) {
            readCluster(currentCluster, clusterBuf);

            DirEntry* entries = (DirEntry*)clusterBuf;
            uint32_t entriesPerCluster = bytesPerCluster / sizeof(DirEntry);

            for (uint32_t i = 0; i < entriesPerCluster; i++) {
                DirEntry* e = &entries[i];

                if ((uint8_t)e->name[0] == 0x00) {
                    KernelAllocator::free(clusterBuf);
                    return false;
                }
                if ((uint8_t)e->name[0] == 0xE5) continue;
                if ((e->attributes & 0x0F) == 0x0F) continue;

                char onDiskName[12];
                for (int j = 0; j < 8; j++) onDiskName[j] = e->name[j];
                for (int j = 0; j < 3; j++) onDiskName[8 + j] = e->ext[j];
                onDiskName[11] = '\0';

                strToUpper(onDiskName);

                if (streqi(onDiskName, fatName)) {
                    *outEntry = *e;
                    KernelAllocator::free(clusterBuf);
                    return true;
                }
            }

            uint32_t next = readFatEntry(currentCluster);
            if (next >= 0x0FFFFFF8) {
                KernelAllocator::free(clusterBuf);
                return false;
            }
            currentCluster = next;
        }
    }

    auto splitNextPathComponent(const char*& path, char out[64]) -> bool {
        while (*path == '/' || *path == '\\') path++;
        if (*path == '\0') return false;

        int i = 0;
        while (*path && *path != '/' && *path != '\\') {
            if (i >= 63) return false;
            out[i++] = *path++;
        }
        out[i] = '\0';
        return true;
    }

    auto resolvePath(const char* fp, DirEntry* outEntry) -> bool {
        if (fp == nullptr || *fp == '\0') return false;

        uint32_t currentDirCluster = bootSector->rootDirClusterNum;
        const char* p = fp;
        char component[64];

        while (true) {
            bool hasComponent = splitNextPathComponent(p, component);
            if (!hasComponent) return false;

            DirEntry found{};
            if (!findDirEntryInDir(currentDirCluster, component, &found)) return false;

            while (*p == '/' || *p == '\\') p++;

            if (*p == '\0') {
                *outEntry = found;
                return true;
            }

            if ((found.attributes & 0x10) == 0) return false;
            currentDirCluster = clusterOf(found);
        }
    }

public:
    virtual auto getPriority() -> int override {
        bootSector = (BootSector*)KernelAllocator::alloc(sizeof(BootSector));
        if (bootSector == nullptr) return 0;

        Storage::readSector((uint16_t*)bootSector, 256, 0);
        return bootSector->signature == 0x28 || bootSector->signature == 0x29;
    }

    virtual auto init() -> void override {
        dataRegionStart = bootSector->reservedSectors + (bootSector->fatsOnMedia * bootSector->sectorsPerFat32);
    }

    auto open(const char* fp) -> File override {
        File ret{};
        ret.fsData = KernelAllocator::alloc(sizeof(FileData));
        if (ret.fsData == nullptr) return ret;

        FileData* fd = (FileData*)ret.fsData;

        if (isRootPath(fp)) {
            fd->cluster = bootSector->rootDirClusterNum;
            fd->isDirectory = true;
            ret.exists = true;
            ret.size = 0;
            return ret;
        }

        DirEntry direntry{};
        if (resolvePath(fp, &direntry)) {
            fd->cluster = clusterOf(direntry);
            fd->isDirectory = (direntry.attributes & 0x10) != 0;
            ret.size = direntry.filesize;
            ret.exists = true;
        }

        return ret;
    }

    auto close(File f) -> void override {
        if (f.fsData != nullptr) KernelAllocator::free(f.fsData);
    }

    auto read(File f, char* obuf, size_t len) -> SuccessCodes override {
        FileData* fd = (FileData*)f.fsData;
        if (fd == nullptr || fd->isDirectory) return SuccessCodes::Error;

        uint32_t bytesPerCluster = bootSector->bytesPerSector * bootSector->sectorsPerCluster;
        uint16_t* clusterBuf = (uint16_t*)KernelAllocator::alloc(bytesPerCluster);
        if (clusterBuf == nullptr) return SuccessCodes::Error;

        size_t bytesRemaining = len < f.size ? len : f.size;
        size_t bytesWritten = 0;
        uint32_t currentCluster = fd->cluster;

        while (bytesRemaining > 0) {
            readCluster(currentCluster, clusterBuf);

            size_t chunk = bytesRemaining < bytesPerCluster ? bytesRemaining : bytesPerCluster;
            memcpy(obuf + bytesWritten, clusterBuf, chunk);

            bytesWritten += chunk;
            bytesRemaining -= chunk;

            if (bytesRemaining == 0) break;

            uint32_t next = readFatEntry(currentCluster);
            if (next >= 0x0FFFFFF8) break;
            currentCluster = next;
        }

        KernelAllocator::free(clusterBuf);
        return SuccessCodes::Sucess;
    }

    auto getDriverName() -> const char* override {
        return "RivOSFs_FAT32";
    }
};

static inline Fat32 fat32Fs;
