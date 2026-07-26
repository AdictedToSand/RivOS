#pragma once
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/driver.hpp>

#include <gen/vec.hpp>

struct FileSystem {
private:
    static inline FileSystemDriver* activeDriver;
    static inline Vector<FileSystemDriver*> driverCandidates;
    
    static inline auto registerDriver(FileSystemDriver* drv) -> void {
        driverCandidates.pushBack(drv);
    }

public:
    static auto init() -> void {
        driverCandidates = Vector<FileSystemDriver*>();

        activeDriver = &defaultFsDriver;
        
        registerDriver(&ext2fs);
        registerDriver(&fat32Fs);

        for (auto& drv : driverCandidates) {
            if (drv->getPriority() > activeDriver->getPriority()) {
                activeDriver = drv;
            }
            else {
                drv->free();
            }
        }
        activeDriver->init();

        Terminal::printf("Filesystem driver: %s\n", activeDriver->getDriverName());
    }
    static auto open(const char* fp) -> File {
        return activeDriver->open(fp);
    }
    static auto read(File f, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes {
        return activeDriver->read(f, obuf, len);
    }
};
