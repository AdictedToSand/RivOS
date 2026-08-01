#pragma once
#include <terminal/terminal.hpp>

#include <drivers/fs/mountpoint.hpp>
#include <drivers/fs/driver.hpp>
#include <drivers/storage/storage.hpp>

#include <mem/alloc.hpp>

#include <cstring.hpp>

/*#pragma once
#include <drivers/fs/file.hpp>

#include <stddef.h>

// Inherit from this class if you want to make a driver 
struct FileSystemDriver {
    virtual auto init() -> void    
    virtual auto getPriority() -> int 

    virtual auto open(const char* _fp, bool* _obufFileExists) -> File

    virtual auto write(File _f, char* _conts, size_t _len) -> SuccessCodes
    virtual auto read(File _f, char* _out, size_t _len) -> SuccessCodes
    virtual auto close(File _f) -> void
    virtual auto mkdir(const char* _fp) -> SuccessCodes
    virtual auto dirExists(const char* _fp) -> bool
    virtual auto fileExists(const char* _fp) -> bool

    virtual auto getDriverName() -> const char* 
    virtual auto free() -> void 
};
*/

struct BootFileSystem : FileSystemDriver {
private:
    struct FsData {
        i32 sector;
    };

public:
     auto init() -> void override {
        
     }
     virtual auto getPriority() -> int override {
         return 0;
     }
     auto open(const char* fp, bool* obufFileExists) -> File override {
        File ret;

        fp += strlen("/boot/");
        if (!strBeginsWith(fp, "sector=")) {
            ret.exists = false;
            *obufFileExists = false;
            return ret;
        }
        fp += strlen("sector=");
        if (!strIsNumber(fp)) {
            ret.exists = false;
            *obufFileExists = false;
            return ret;
        }
        const i32 sector = stoi(fp);
        if (sector < 0) {
            ret.exists = false;
            *obufFileExists = false;
            return ret;
        }
        FsData* fsData = (FsData*) KernelAllocator::alloc(sizeof(FsData));
        fsData->sector = sector;
        ret.exists = true;
        ret.size = Storage::SECTOR_SIZE;
        ret.fsData = fsData;

        return ret;
     }
     // Idk why but writing to this if fucking weird
     // (sector=0 doesn't erase boot sector but does cause a forever loop of #UD?)
     auto write(File f, char* conts, size_t len) -> FileSystemDriver::SuccessCodes override {
         Storage::selectNextDrive();
        const i32 sector = ((FsData*) f.fsData)->sector;

        Storage::writeSector((u16*) conts, len / 2, sector);

        Storage::selectNextDrive();
        return FileSystemDriver::SuccessCodes::Sucess;
     }
     auto read(File f, char* out, size_t len) -> FileSystemDriver::SuccessCodes override {
        Storage::selectNextDrive();

        const i32 sector = ((FsData*) f.fsData)->sector;

        Storage::readSector((u16*) out, len, sector);

        Storage::selectNextDrive();
        return FileSystemDriver::SuccessCodes::Sucess;
     }
     auto close(File f) -> void override {
        KernelAllocator::free(f.fsData);
     }

     auto getDriverName() -> const char* override {
         return "RivOSFs_BootFilesDriver";
     }
};

static inline BootFileSystem bootFs;

struct BootMp : MountPoint {
    auto shouldActivate() -> bool {
        return true;
    }
    BootMp() {
        fsDriver = &bootFs;
    }
};

static inline BootMp bootMp;
