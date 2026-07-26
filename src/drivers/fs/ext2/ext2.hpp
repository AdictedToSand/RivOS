#pragma once
#include <drivers/fs/driver.hpp>
#include <drivers/storage/storage.hpp>

#include <mem/alloc.hpp>

#include <stdint.h>

struct Ext2 : FileSystemDriver {
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
        uint16_t magic;

        uint16_t fsState;
        uint16_t errHandleMethod;
        uint16_t minorPortionVersion;
        uint32_t posixTime_lastConsistencyCheck;
        uint32_t intervalForcedConsistencyChecks;
        uint32_t osId;
        uint32_t majorPortionVersion;
        uint16_t userIdReservedBlocks;
        uint16_t groupIdReservedBlocks;
    };
    struct [[gnu::packed]] Ext2SuperBlockHigh {
        
    };

    struct Ext2SuperBlock {
        Ext2SuperBlockLow low;
        Ext2SuperBlockHigh high;
    };

    struct [[gnu::packed]] BgdtEntry {
        uint32_t blockAddrBlockUsageBitmap;
        uint32_t blockAddrInodeUsageBitmap;
        uint32_t startingBlockAddrOfInodeTable;
        uint16_t unallocatedBlocks;
        uint16_t unallocatedInodes;
        uint16_t directories;
        uint16_t padding;
        char _unused[14];
    };

    static inline Ext2SuperBlock* suprBlock;

    static constexpr uint16_t EXT2_MAGIC = 0xEF53;
    int getPriority() override {
        uint16_t lowSuperBlock[256];

        if (Storage::readSector((uint16_t*) lowSuperBlock, 256, 2) == StorageDriver::SuccessCodes::Error) {
            kpanic("Unable to read sector");
        }
        suprBlock = (Ext2SuperBlock*) KernelAllocator::alloc(sizeof(Ext2SuperBlock));

        if (!suprBlock) {
            kpanic("Allocated wrong");
        }

        suprBlock->low = *((Ext2SuperBlockLow*) lowSuperBlock);
        //TODO: Make better check (specifically in init)

        return suprBlock->low.magic == EXT2_MAGIC;
    }

    auto getDriverName() -> const char* override {
        return "RivOSFs_Ext2";
    }
    
    auto init() -> void override {

    }

    auto blockToSector(const size_t block) -> size_t {
        const size_t blockSize = 1024 << suprBlock->low.blockSize;

        if (blockSize % Storage::SECTOR_SIZE != 0) {
            kpanic("BlockSize did not match sector_size");
        }

        const size_t sectorsPerBlock = blockSize / Storage::SECTOR_SIZE;

        return block * sectorsPerBlock;
    }

    auto open(const char* _fp, bool* _fileExistsObuf) -> File override {
        static constexpr size_t BGDT_ENTRY_SIZE = 32;
        File ret;

        size_t bgdtStartSector;
        const size_t blockSize = 1024 << suprBlock->low.blockSize;
        if (blockSize == 1024) {
            bgdtStartSector = blockToSector(2);
        }
        else {
            bgdtStartSector = blockToSector(1);
        }
        
        const size_t amountOfBgdEntries = (suprBlock->low.totalBlocks + suprBlock->low.blocksInBlockGroup - 1) /
            suprBlock->low.blocksInBlockGroup;

        char* const bgdtBuffer = (char*) KernelAllocator::alloc(amountOfBgdEntries * BGDT_ENTRY_SIZE);

        for (size_t offset = 0; offset < amountOfBgdEntries * BGDT_ENTRY_SIZE; offset += Storage::SECTOR_SIZE) {
            Storage::readSector(
                (uint16_t*) (bgdtBuffer + offset),
                256,
                bgdtStartSector + (offset / Storage::SECTOR_SIZE)
            );
        }
        (void) _fileExistsObuf; (void) _fp;

        KernelAllocator::free(bgdtBuffer);
        return ret;
    }

    auto close(File f) -> void override {
        KernelAllocator::free(f.fsData);
    }

    auto free() -> void override {
        KernelAllocator::free(suprBlock);
    }
};

static inline Ext2 ext2fs;
