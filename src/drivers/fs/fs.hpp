#pragma once
#include <drivers/fs/file.hpp>
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/dev/dev.hpp>
#include <drivers/fs/driver.hpp>
#include <drivers/fs/mountpoint.hpp>
#include <drivers/fs/boot/boot.hpp>
#include <drivers/fs/virtkrn/virtkrn.hpp>

#include <gen/vec.hpp>
#include <gen/map.hpp>

#include <cstring.hpp>
#include <str.hpp>

#include <mem/alloc.hpp>

typedef int fd_t;

struct MountPointCandidate {
    const char* mp = "";
    MountPoint* conts;

    inline auto shouldActivate() -> bool {
        return conts->shouldActivate();
    }
    inline auto getmp() -> MountPoint* {
        return conts;
    }
};

static inline MountPointCandidate ext2Candidate = {
    .conts = &Ext2RootMp,
};
static inline MountPointCandidate fat32Candidate = {
    .conts = &fat32Root,
};
static inline MountPointCandidate devCandidate = {
    .conts = &devMp,
};
static inline MountPointCandidate bootCandidate = {
    .conts = &bootMp,
};
static inline MountPointCandidate virtKrnCandidate = {
    .conts = &virtKrnMp,
};

struct GlobalFile {
    File f;
    MountPoint* mp;
};

struct MountPointEntry {
    char* k;
    MountPoint* v;
};

struct FileSystem {
private:
    //TODO: Duplicate filedescriptors can exist
    static inline Vector<MountPointCandidate*> mpCandidates;
    static inline Vector<MountPointEntry*> mountpoints;

    // An mp command should be permanent and never leave scope
    // (
    //      Good: registerMountpoint("/", drv);
    //      Bad: char mp[] = "/"; registerMountpoint(mp, drv); 
    // )
    // Because mp is a stack variable and will leave scope and become garbage!
    static inline auto registerMountpoint(const char* mp, MountPointCandidate* drv) -> void {
        drv->mp = mp;
        mpCandidates.pushBack(drv);
    }

    static inline size_t latestFd = 1;
    static inline Map<fd_t, GlobalFile> fdMapping;

    static inline auto getFolderNestage(const char* s) -> size_t {
        size_t ret = 0;
        for (; *s; s++) {
            if (*s == '/') ret++;
        }
        return ret;
    }
    static inline auto mountPointMatches(const char* mp, const char* fp) -> bool {
        for (size_t i = 0; i < strlen(mp); i++) {
            if (strlen(fp) >= strlen(mp) && mp[i] != fp[i]) return false;
        }

        return true;
    }

public:
    static auto init() -> void {
        registerMountpoint("/", &ext2Candidate);
        registerMountpoint("/dev/", &devCandidate);
        registerMountpoint("/", &fat32Candidate);
        registerMountpoint("/boot/", &bootCandidate);
        registerMountpoint("/krn/virt/", &virtKrnCandidate);

        size_t i = 0;
        for (auto mp : mpCandidates) {
            if (mp->shouldActivate()) {
                MountPointEntry* entry = (MountPointEntry*) KernelAllocator::alloc(sizeof(MountPointEntry));
                entry->k = (char*) KernelAllocator::alloc(strlen(mp->mp) + 1);
                memset(entry->k, 0, strlen(mp->mp));
                strcpy(entry->k, mp->mp);
                entry->v = mp->getmp();
                mountpoints.pushBack(entry);
            }
            i++;
        }
        bool rootFound = false;
        for (auto& mpentry : mountpoints) {
            if (streq(mpentry->k, "/")) { rootFound = true; break; }
        }
        if (!rootFound) {
            kpanic("No mountpoint was found for root (/)");
        }

        for (auto& kv : mountpoints) {
            kv->v->init();
        }

        mpCandidates.~Vector<MountPointCandidate*>();

        for (auto& mp : mountpoints) {
            Terminal::printf("Driver: '%s' won for the mountpoint: '%s'\n", mp->v->fsDriver->getDriverName(), mp->k);
        }
    }

    static auto open(const char* fp) -> fd_t {
        const fd_t currentFd = latestFd++;

        GlobalFile added;
        MountPoint* lastMountpoint = (MountPoint*) KernelAllocator::alloc(sizeof(MountPoint));
        size_t lastFolderNestage = 0;
        size_t lastMpLen = 0;
        
        if (fp[0] != '/') return 0;

        size_t i;
        for (i = 0; i < mountpoints.size(); i++) {
            auto mp = mountpoints[i];
            if (mountPointMatches(mp.val()->k, fp)) break;

            if (i == mountpoints.size() - 1) i = -1;
        }
        if (i == (size_t) -1) return 0;

        for (i = 0; i < mountpoints.size(); i++) {
            auto mp = mountpoints[i];
            if ((getFolderNestage(mp.val()->k) > lastFolderNestage && mountPointMatches(mp.val()->k, fp))
                || (getFolderNestage(mp.val()->k) == lastFolderNestage && mountPointMatches(mp.val()->k, fp) && strlen(mp.val()->k) > lastMpLen)) {
                lastFolderNestage = getFolderNestage(mp.val()->k);
                lastMountpoint = mp.val()->v;
            }
        }
        if (lastFolderNestage == 0) return 0;
        added.mp = lastMountpoint;
        added.f = lastMountpoint->fsDriver->open(fp, &added.f.exists);

        fdMapping[currentFd] = added;

        return added.f.exists ? currentFd : 0;
    }
    static auto close(fd_t fd) -> void {
        if (!fd) return;
        auto f = fdMapping[fd];

        f.mp->fsDriver->close(f.f);
        fdMapping.rmkey(fd);
    }
    static auto read(fd_t fd, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes {
        if (!fd) return FileSystemDriver::SuccessCodes::Error; 
        const auto gf = fdMapping[fd];

        return gf.mp->fsDriver->read(gf.f, obuf, len);
    }
    static auto write(fd_t fd, char* conts, size_t len) -> FileSystemDriver::SuccessCodes {
        if (!fd) return FileSystemDriver::SuccessCodes::Error;
        const auto gf = fdMapping[fd];

        return gf.mp->fsDriver->write(gf.f, conts, len);
    }
    static auto fileSize(fd_t fd) -> u32 {
        if (!fd) return 0;

        return fdMapping[fd].f.size;
    }
};

using FsSuccessCodes = FileSystemDriver::SuccessCodes;
