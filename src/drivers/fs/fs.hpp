#pragma once
#include <drivers/fs/ext2/ext2.hpp>
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

        for (auto& drv : driverCandidates) {
            if (drv->getPriority() > activeDriver->getPriority()) {
                activeDriver = drv;
            }
        }
        activeDriver->init();

        Terminal::printf("Filesystem driver: %s\n", activeDriver->getDriverName());
    }
};
