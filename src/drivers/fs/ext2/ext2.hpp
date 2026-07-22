#pragma once
#include <drivers/fs/driver.hpp>
#include <drivers/storage/storage.hpp>

#include <stdint.h>

struct Ext2 : FileSystemDriver {
    struct Ext2File {

    };

    struct [[gnu::packed]] Ext2SuperBlockLow {
        uint32_t totalInnodes;
        uint32_t totalBlocks;
        uint32_t blockForSuperUser;
        uint32_t totalUnallocatedBlocks;
        uint32_t totalUnallocatedInnodes;
        uint32_t firstDataBlock;
        uint32_t blockSize; // Tf? log2 (block size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the block size) 
        uint32_t fragmentSize; // Same as above?
        uint32_t blocksInBlockGroup;
        uint32_t fragmentsInBlockGroup;
        uint32_t innodesInBlockGroup;
        uint32_t posixTime_lastMountTime; // Don't bother yet
        uint32_t posixTime_lastWrittenTime;
        uint16_t mountsBeforeConsistencyCheck; // Don't bother also
        uint16_t maxMountsBeforeConsistencyCheck;
        uint16_t signature;
    };
    struct [[gnu::packed]] Ext2SuperBlockHigh {
        
    };

    struct Ext2SuperBlock {
        Ext2SuperBlockLow low;
        Ext2SuperBlockHigh high;
    };

    static constexpr uint16_t EXT2_MAGIC = 0xEF53;
    int getPriority() override {
        uint16_t lowSuperBlock[256];

        if (Storage::readSector((uint16_t*) lowSuperBlock, 256, 2) == StorageDriver::SuccessCodes::Error) {
            kpanic("Unable to read sector");
        }
        
        Ext2SuperBlock suprBlock;
        suprBlock.low = *((Ext2SuperBlockLow*) lowSuperBlock);

        return suprBlock.low.signature == EXT2_MAGIC;
    }

    auto getDriverName() -> const char* override {
        return "RivOSFs_Ext2";
    }
    
    auto init() -> void override {
    }

    auto open(const char* fp) -> File override {
        File ret;

        return ret;
    }

    auto close(File fd) -> void override {
         
    }
};

static inline Ext2 ext2fs;
