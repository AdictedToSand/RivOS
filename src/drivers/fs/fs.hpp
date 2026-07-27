#pragma once
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/driver.hpp>
#include <drivers/fs/mountpoint.hpp>

#include <gen/vec.hpp>
#include <gen/map.hpp>

#include <cstring.hpp>
#include <str.hpp>

typedef int fd_t;

struct MountPointCandidate {
    const char* mp;
    MountPoint* conts;

    inline auto shouldActivate() -> bool {
        return conts->shouldActivate();
    }
    inline auto getmp() -> MountPoint* {
        return conts;
    }
};

static inline MountPointCandidate ext2Candidate = {
    .mp = "/",
    .conts = &Ext2RootMp,
};
static inline MountPointCandidate fat32Candidate = {
    .mp = "/",
    .conts = &fat32Root,
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
    static inline Vector<MountPointCandidate*> mpCandidates;
    static inline Vector<MountPointEntry*> mountpoints;

    static inline auto registerMountpoint(const char* mp, MountPointCandidate* drv) -> void {
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
            if (mp[i] != fp[i]) return false;
        }

        return true;
    }

public:
    static auto init() -> void {
        fdMapping = Map<fd_t, GlobalFile>();
        mpCandidates = Vector<MountPointCandidate*>();
        
        registerMountpoint("/", &ext2Candidate);
        registerMountpoint("/", &fat32Candidate);

        for (auto mp : mpCandidates) {
            if (mp->shouldActivate()) {
                Terminal::printf("Driver won: %s for mp: %s\n", mp->conts->fsDriver->getDriverName(), mp->mp);
                MountPointEntry* entry = (MountPointEntry*) KernelAllocator::alloc(sizeof(MountPointEntry));
                entry->k = (char*) KernelAllocator::alloc(strlen(mp->mp));
                memset(entry->k, 0, strlen(mp->mp));
                strcpy(entry->k, mp->mp);
                entry->v = mp->getmp();
                mountpoints.pushBack(entry);
                mp->getmp()->init();
            }
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

        for (auto& mp : mountpoints) {
            if (getFolderNestage(mp->k) > lastFolderNestage && mountPointMatches(mp->k, fp)) {
                lastFolderNestage = getFolderNestage(mp->k);
                lastMountpoint = mp->v;
            }
        }
        added.mp = lastMountpoint;
        added.f = lastMountpoint->fsDriver->open(fp, &added.f.exists);

        fdMapping[currentFd] = added;

        return added.f.exists ? currentFd : 0;
    }
    static auto close(fd_t fd) -> void {
        auto f = fdMapping[fd];

        f.mp->fsDriver->close(f.f);
        KernelAllocator::free(f.mp);
        fdMapping.rmkey(fd);
    }
    static auto read(fd_t fd, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes {
        const auto f = fdMapping[fd];

        return f.mp->fsDriver->read(f.f, obuf, len);
    }
};
