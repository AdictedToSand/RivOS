#pragma once
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/driver.hpp>

#include <gen/vec.hpp>
#include <gen/map.hpp>

typedef int fd_t;

struct FileSystem {
private:
    static inline FileSystemDriver* activeDriver;
    static inline Vector<FileSystemDriver*> driverCandidates;

    static inline auto registerDriver(FileSystemDriver* drv) -> void {
        driverCandidates.pushBack(drv);
    }

    static inline size_t latestFd = 1;
    static inline Map<fd_t, File> fdMapping;

public:
    static auto init() -> void {
        fdMapping = Map<fd_t, File>();

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
    static auto open(const char* fp) -> fd_t {
        const fd_t currentFd = latestFd++;
        bool fileExists;
        fdMapping[currentFd] = activeDriver->open(fp, &fileExists);
        return fileExists ? currentFd : 0;
    }
    static auto read(fd_t fd, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes {
        const File f = fdMapping[fd];
        return activeDriver->read(f, obuf, len);
    }
};
