#pragma once
#include <drivers/fs/rivfs/rivfs.hpp>

#include <drivers/storage/storage.hpp>

#include <mem/alloc.hpp>

#include <gen/map.hpp>
#include <gen/conf/conf.hpp>

#include <arrv.hpp>
#include <int.h>

#include <rap/rap.hpp>

#include <proc/process.hpp>

typedef i32 fd_t;

struct VFS {
private:
    static inline u8 selectedDrive = 0;
    struct DriveEntry {
        u8 id; 
        char* fsName;
        DriveEntry(u8 iid, char* ifsName) : id(iid), fsName(ifsName) {}
    };
    struct FsEntry {
        Process* proc; 
        FsEntry(Process* iproc) : proc(iproc) {};
    };

    static inline Vector<DriveEntry> driveEntries;
    static inline Map<StringView, FsEntry> fsentries;
public:
    static auto initFromSrc(char* srcFsconf) -> void {
        Serial::logf("SrcFsConf=%s", srcFsconf);
        Config conf;
        conf.src = srcFsconf;
        conf.parseSrc();

        if (conf.isErr) {
            kpanic("Invalid filesystem config");      
        }
        const Vector<StringView> allDriveConfigs = conf.getAllSubsections(StringView("drives."));

#ifdef DEBUG
        for (auto driveSection : allDriveConfigs) {
            Serial::logf("Drive=%s", driveSection.toCStr());
        }
#endif
        for (auto driveSection : allDriveConfigs) {
            char* const driveSectionCStr = driveSection.toCStr();
            Str driveFullName = driveSectionCStr;
            driveFullName += ".drive";
            const Config::Value driveVal = conf.mapping[driveFullName.toCStr()];
            Str fsFullName = driveSectionCStr;
            fsFullName += ".fs";
            const Config::Value fsVal = conf.mapping[fsFullName.toCStr()]; 
            if (driveVal.isErr || driveVal.state != Config::Value::States::Int) kpanic("Invalid field (drives.*.drive)");
            if (fsVal.isErr || fsVal.state != Config::Value::States::String) kpanic("Invalid field (drives.*.fs)");
            StringView fsName = fsVal.strVal;
            const u32 driveId = driveVal.intVal;
            // toCStr does a permanent copy
            DriveEntry addedDriveEntry(driveId, fsName.toCStr());
            Serial::logf("Drive %u (fs=%s)", driveId, addedDriveEntry.fsName);
            driveEntries.pushBack(addedDriveEntry);

            KernelAllocator::free(driveSectionCStr);
        }

        conf.freeLeftover();
    }
    static auto registerForFs(const char* fstype, const char* drvfp, RivFs* const fs, Process* drvProc) -> void {
        Str fullRapFp = drvfp; fullRapFp += "/fn.rap";
        Expected<RivFs::File> expectedFnRap = fs->open(drvfp);

        if (expectedFnRap.isErr()) kpanic("Rap File not found for registering in ");

        const RivFs::File fnRap = expectedFnRap.valUnchecked();
        const u32 fnRapBufSize  = fs->filesize(fnRap) + 1;
        char* const fnRapBuf    = (char*) KernelAllocator::alloc(fnRapBufSize);
        memset(fnRapBuf, 0, fnRapBufSize);
        fs->read(fnRap, fnRapBuf);

        Serial::logf("FsRap=%s", fnRapBuf);

        RapFile rapf(fnRapBuf);
        rapf.parseFile();

        void* const openFunction = rapf.getRapEntry("open", ArrayView<RapParam>({RapParam("str", "fp")}));
        if (!openFunction) {
            kpanic("RapFile for fs driver did not contain open() function.");
        }
        FsEntry fsEntry(drvProc);

        fsentries.insert(StringView(heapCopyStr(fstype)), fsEntry); 

        KernelAllocator::free(fnRapBuf);
    }
    auto open(const char* fp) -> fd_t {
        return -1; 
    }
    auto errFromFd(fd_t fd) -> const char* {
        if (fd >= 0)
            return "NoError";
        else if (fd == -1) return "FileNotFound";
        else return "ErrorUnknown";
    }
};
