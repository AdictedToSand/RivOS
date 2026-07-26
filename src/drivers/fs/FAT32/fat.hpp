#pragma once
#include <drivers/fs/driver.hpp>
#include <drivers/storage/storage.hpp>

#include <mem/alloc.hpp>

#include <terminal/terminal.hpp>

#include <stdint.h>

struct Fat32 : FileSystemDriver {
private:
    struct [[gnu::packed]] BootSector {
        char jumpInstruction[3]; // What the fuck
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
        uint16_t versionNumber; // High byte major low byte minor
        uint32_t rootDirClusterNum;
        uint16_t sectorNumOfFsinfo;
        uint16_t sectorNumberOfBackupBootSector;
        char reserved[12]; // Should be 0 when a volume is formatted
        uint8_t driveNumber;
        uint8_t reserved2;
        uint8_t signature;
        uint32_t volumeSerialNumberId;
        char volumeLabelString[11]; // Padded with spaces
        char bootcode[420];
    };

    // For some fucking reason this needs to be global bc per class hangs forever like tf?
    // Oh fuck it's because vector is hardcoded to sizeof(baseClass)
    // Oh well
    static inline BootSector* bootSector;

public:
    virtual auto getPriority() -> int override {
        bootSector = (BootSector*) KernelAllocator::alloc(sizeof(BootSector));

        if (bootSector == nullptr) return 0;

        Storage::readSector((uint16_t*) bootSector, 256, 0);

        return bootSector->signature == 0x28 || bootSector->signature == 0x29;
    }

    auto open(const char* fp) -> File override {
        File ret;

        

        return ret;
    }

    auto getDriverName() -> const char* override {
        return "RivOSFs_FAT32";
    }
};

static inline Fat32 fat32Fs;
