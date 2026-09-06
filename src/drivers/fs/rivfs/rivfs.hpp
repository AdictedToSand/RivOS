#pragma once
#include <sum.hpp>
#include <int.h>
#include <sv.hpp>
#include <cstring.hpp>

#include <mem/utils.hpp>
#include <mem/alloc.hpp>

#include <gen/err.hpp>
#include <gen/vec.hpp>
#include <gen/serial.hpp>

// NOTE: This is not part of the greater "drivers/fs/fs.hpp" mountpoint stuff, it is self contained.

struct RivFs {
private:
    struct [[gnu::packed]] BaseHdr {
        char magic[5]; 
        u32 fphSize;
        u32 fphPos;
        u8 vers[2];
        enum class Flags : u32 {
            None,
            Ronly,
        } flgs;
    };

    struct [[gnu::packed]] FilePositionHeader {
        u16 dirHdrAm;
        u16 fAm;
        u32 dirstart;
        u32 fstart;
        enum class Flags : u32 {
            None
        } flgs;
    };

    struct [[gnu::packed]] DirEntryLatter {
        // Normally sv would be here.
        u16 dirHdrAm;
        u16 fileAm;
        u32 dirStart;
        u32 fStart;
        enum class Flags : u32 {
            None,
        } flgs;
    };

    void* rawFsMemloc;
    u32 rawFsSize;
    FilePositionHeader* fph;

    static constexpr char INITIAL_MAGIC[] = "rivfs";

    static auto findChar(const char* str, char target) -> u32 {
        for (u32 i = 0; str[i]; i++) {
            if (str[i] == target)
                return i;
        }

        return 0;
    }

    static auto advancePastFileEntry(char*& offs) -> StringView {
        u32 nameLen = *(u32*) offs;
        offs += sizeof(u32);
        StringView name(offs, nameLen);
        offs += nameLen;
        u32 contsLen = *(u32*) offs;
        offs += sizeof(u32) + contsLen;
        return name;
    }

    static auto advancePastDirEntry(char*& offs, DirEntryLatter*& latterOut) -> StringView {
        u32 nameLen = *(u32*) offs;
        offs += sizeof(u32);
        StringView name(offs, nameLen);
        offs += nameLen;
        latterOut = (DirEntryLatter*) offs;
        offs += sizeof(DirEntryLatter);
        return name;
    }

public:
    struct File {
        friend struct RivFs;

        private:
        u32 startPos; // Name sv starts here
        u32 contsSvPos; // filesize starts here (and conts after it)
    };

    struct Dir {
        friend struct RivFs;

        private:
        u32 startPos;
        u32 constPos;
    };

    struct DirIterator {
        friend struct RivFs;

        private:
        u32 dirPos;
        u32 filePos;
        u16 dirsRemaining;
        u16 filesRemaining;
    };

    RivFs(void* memloc, u32 size) :
        rawFsMemloc(memloc), rawFsSize(size), fph(nullptr) {}

    auto checkCorrectness() -> bool {
        auto* hdr = (BaseHdr*) rawFsMemloc;

        if (memcmp(hdr->magic, INITIAL_MAGIC, sizeof(INITIAL_MAGIC) - 1) != 0) {
            Serial::logf("Invalid magic");
            return false;
        }

        if (hdr->fphSize != sizeof(FilePositionHeader)) {
            Serial::logf(
                "fphSize != sizeof(FilePositionHeader): sizeof(fph)=%u and fphSize=%u", sizeof(FilePositionHeader),  hdr->fphSize);
            return false;
        }

        if (!hdr->fphPos) {
            Serial::log("FphPos=0");
            return false;
        }

        if (!((u32) hdr->flgs & (u32) BaseHdr::Flags::Ronly)) {
            Serial::log("Fs did not have Ronly flag enabled");
            return false;
        }

        fph = (FilePositionHeader*) ((u8*) rawFsMemloc + hdr->fphPos);

        if (!fph->dirstart) {
            Serial::log("fph.dirstart=0");
            return false;
        }

        if (!fph->fstart) {
            Serial::log("fph.fstart=0");
            return false;
        }

        return true;
    }

    // Should prolly be private
    auto findInDir(void* dirEntry, StringView fn, bool isDir) -> Expected<File> {
        const char* const _fnCStrTmp = fn.toCStr();

        const u32 slashPos = findChar(_fnCStrTmp, '/');
        const bool hasSlash = slashPos != 0;
        const u32 nameLen = hasSlash ? slashPos : fn.len;

        StringView neededFilenameToFind(fn.raw, nameLen);

        u32 ownFilenamePart = *(u32*) dirEntry;

        DirEntryLatter* latterDirEntry = (DirEntryLatter*) ((char*) dirEntry + sizeof(u32) + ownFilenamePart);

        if (!hasSlash && !isDir) {
            KernelAllocator::free((void*) _fnCStrTmp);

            char* currentOffs = (char*) rawFsMemloc + latterDirEntry->fStart;

            for (u32 i = 0; i < latterDirEntry->fileAm; i++) {
                u32 currentSvLen = *(u32*) currentOffs;

                if (!currentSvLen) {
                    Serial::log("Hit file EOF marker");
                    return ExpectedErr<File>();
                }
                currentOffs += sizeof(u32);

                StringView sv(currentOffs, currentSvLen);

                if (sv.len == neededFilenameToFind.len) {
                    bool same = true;

                    for (u32 j = 0; j < sv.len; j++) {
                        if (sv.raw[j] != neededFilenameToFind.raw[j]) {
                            Serial::logf(
                                "BYTE MISMATCH i=%u file=%x wanted=%x",
                                j,
                                (u32)(u8) sv.raw[j],
                                (u32)(u8) neededFilenameToFind.raw[j]
                            );

                            same = false;
                            break;
                        }
                    }

                    if (same) {
                        File ret;

                        ret.startPos = (u32)(currentOffs - sizeof(u32));

                        ret.contsSvPos = (u32)(currentOffs + currentSvLen);

                        return ret;
                    }
                }

                currentOffs += currentSvLen;

                u32 contsLen = *(u32*) currentOffs;

                currentOffs += sizeof(u32);
                currentOffs += contsLen;
            }
        }
        else {
            KernelAllocator::free((void*) _fnCStrTmp);

            char* currentOffs = (char*) rawFsMemloc + latterDirEntry->dirStart;

            for (u32 i = 0; i < latterDirEntry->dirHdrAm; i++) {
                char* childDirEntry = currentOffs;

                u32 currentSvLen = *(u32*) currentOffs;

                if (!currentSvLen) {
                    Serial::log("Hit directory EOF marker");
                    break;
                }

                currentOffs += sizeof(u32);

                StringView sv(currentOffs, currentSvLen);

                bool same = sv.len == neededFilenameToFind.len;

                if (same) {
                    for (u32 j = 0; j < sv.len; j++) {
                        if (sv.raw[j] != neededFilenameToFind.raw[j]) {
                            same = false;
                            break;
                        }
                    }
                }

                currentOffs += currentSvLen;

                if (same) {
                    const u32 consumed = nameLen + 1;

                    if (fn.len <= consumed) {
                        return ExpectedErr<File>();
                    }

                    StringView remaining(fn.raw + consumed, fn.len - consumed);

                    const bool remainingIsDir = findChar(remaining.raw, '/') != 0;

                    return findInDir(childDirEntry, remaining, remainingIsDir);
                }

                currentOffs += sizeof(DirEntryLatter);
            }
        }

        return ExpectedErr<File>();
    }

    auto open(const char* fp) -> Expected<File> {
        if (!fp) {
            return ExpectedErr<File>();
        }

        File ret;

        const char* const orginFp = fp;

        struct FilePart {
            StringView sv;
            bool isDir;

            FilePart(StringView sv, bool isDir) :
                sv(sv), isDir(isDir) {}
        };

        Vector<FilePart> svArr = Vector<FilePart>();

        if (*fp != '/') {
            return ExpectedErr<File>();
        }

        fp++;

        while (*fp) {
            const char* start = fp;
            u32 len = 0;

            while (fp[len] && fp[len] != '/')
                len++;

            if (!len) {
                fp++;
                continue;
            }

            bool isDir = fp[len] == '/';

            StringView sv(start, len);

            svArr.pushBack(FilePart(sv, isDir));

            fp += len;

            if (*fp == '/')
                fp++;
        }

        if (svArr.size() == 0) {
            return ExpectedErr<File>();
        }

        if (svArr.size() == 1) {
            // use the FPH

            char* currentOffs = (char*) rawFsMemloc + (u32) fph->fstart;

            for (u32 i = 0; i < fph->fAm; i++) {
                u32 currentSvLen = *(u32*) currentOffs;

                if (!currentSvLen) {
                    return ExpectedErr<File>();
                }

                currentOffs += sizeof(u32);

                StringView sv((char*) currentOffs, currentSvLen);

                if (sv.len == svArr[0].val().sv.len) {
                    bool same = true;

                    for (u32 j = 0; j < sv.len; j++) {
                        if (sv.raw[j] != svArr[0].val().sv.raw[j]) {
                            same = false;
                            break;
                        }
                    }

                    if (same) {
                        ret.startPos = (u32) currentOffs - sizeof(u32);

                        ret.contsSvPos = (u32) currentOffs + currentSvLen;

                        return ret;
                    }
                }

                currentOffs += currentSvLen;

                u32 contsLen = *(u32*) currentOffs;

                currentOffs += sizeof(u32);
                currentOffs += contsLen;
            }

            return ExpectedErr<File>();
        }

        //TODO: ...
        else if (svArr[0].val().isDir) {
            char* currentOffs = (char*) rawFsMemloc + fph->dirstart;

            for (u32 i = 0; i < fph->dirHdrAm; i++) {
                u32* currentDirNameLen = (u32*) currentOffs;

                if (!*currentDirNameLen) {
                    break;
                }

                currentOffs += sizeof(u32);

                StringView dname(currentOffs, *currentDirNameLen);

                bool same = dname.len == svArr[0].val().sv.len;

                if (same) {
                    for (u32 j = 0; j < dname.len; j++) {
                        if (dname.raw[j] != svArr[0].val().sv.raw[j]) {
                            same = false;
                            break;
                        }
                    }
                }

                currentOffs += *currentDirNameLen;

                if (same) {
                    void* newDirEntry = (void*) currentDirNameLen;

                    const u32 firstSlash =
                        findChar(orginFp + 1, '/');

                    if (!firstSlash) {
                        return ExpectedErr<File>();
                    }

                    const char* tmp = orginFp + 1 + firstSlash + 1;

                    const u32 remainingLen = strlen(orginFp) - 1 - firstSlash - 1;

                    StringView newFp(tmp, remainingLen);

                    const bool remainingIsDir = findChar(newFp.raw, '/') != 0;

                    return findInDir(newDirEntry, newFp, remainingIsDir);
                }

                currentOffs += sizeof(DirEntryLatter);
            }
        }

        return ExpectedErr<File>();
    }

    auto filesize(File f) -> uint32_t {
        u32* filesizeStart = (u32*) f.contsSvPos;

        return *filesizeStart;
    }

    auto read(File f, char* obuf) -> void {
        u32* lenptr = (u32*) f.startPos;
        Serial::logf("Filesize=%u", filesize(f));

        strcpyLen(obuf, ((char*) lenptr + *lenptr + (sizeof(u32) * 2)),filesize(f));
    }

    auto getDirIt(const char* fp) -> Expected<DirIterator> {
        if (*fp != '/') {
            return ExpectedErr<DirIterator>();
        }
        fp++;

        DirIterator ret;
        // Root directory
        if (!*fp) {
            ret.dirPos = fph->dirstart;
            ret.filePos = fph->fstart;
            ret.dirsRemaining = fph->dirHdrAm;
            ret.filesRemaining = fph->fAm;
            return ret;
        }

        char* currentDir = (char*) rawFsMemloc + fph->dirstart;
        u16 dirCount = fph->dirHdrAm;
        while (*fp) {
            u32 len = strlenSpecChar(fp, '/');

            if (!len) {
                return ExpectedErr<DirIterator>();
            }

            StringView wanted(fp, len);

            bool found = false;

            for (u16 i = 0; i < dirCount; i++) {
                char* entry = currentDir;

                u32 nameLen = *(u32*) entry;

                if (!nameLen)
                    break;

                entry += sizeof(u32);

                StringView name(entry, nameLen);

                entry += nameLen;

                if (name == wanted) {
                    DirEntryLatter* latter = (DirEntryLatter*) entry;

                    currentDir = (char*) rawFsMemloc + latter->dirStart;

                    ret.dirPos = latter->dirStart;
                    ret.filePos = latter->fStart;
                    ret.dirsRemaining = latter->dirHdrAm;
                    ret.filesRemaining = latter->fileAm;

                    found = true;
                    break;
                }

                currentDir = entry + sizeof(DirEntryLatter);
            }

            if (!found) {
                return ExpectedErr<DirIterator>();
            }

            fp += len;

            if (*fp == '/') {
                fp++;
            }
        }

        return ret;
    }

    struct FileData {
        StringView filename;
        File file;
    };

    struct DirData {
        StringView dirname;
        Dir dir;
    };

    auto dirItGetNextFile(DirIterator& dirit) -> Expected<FileData> {
        if (dirit.filesRemaining) {
            FileData ret = {};

            dirit.filesRemaining--;

            u32* svLen = (u32*) ((u32) rawFsMemloc + dirit.filePos);

            if (!*svLen) {
                // EOF
                dirit.filesRemaining = 0;

                goto outOfInitialIf;
            }

            ret.file.startPos = dirit.filePos;

            dirit.filePos += sizeof(u32);

            ret.filename = StringView((char*) rawFsMemloc + dirit.filePos, *svLen);

            dirit.filePos += *svLen;

            u32* contsLen = (u32*) ((u32) rawFsMemloc + dirit.filePos);

            ret.file.contsSvPos = (u32) contsLen;

            dirit.filePos += sizeof(u32);
            dirit.filePos += *contsLen;

            return ret;
        }

outOfInitialIf:
        return ExpectedErr<FileData>();
    }

    auto dirItGetNextDir(DirIterator& dirit) -> Expected<DirData> {
        if (dirit.dirsRemaining) {
            DirData ret = {};

            dirit.dirsRemaining--;

            u32* svLen = (u32*) ((u32) rawFsMemloc + dirit.dirPos);

            if (!*svLen) {
                dirit.dirsRemaining = 0;

                return ExpectedErr<DirData>();
            }

            ret.dir.startPos =  dirit.dirPos;

            dirit.dirPos += sizeof(u32);

            ret.dirname = StringView((char*) rawFsMemloc + dirit.dirPos, *svLen);

            dirit.dirPos += *svLen;

            ret.dir.constPos = dirit.dirPos;

            dirit.dirPos += sizeof(DirEntryLatter);

            return ret;
        }

        return ExpectedErr<DirData>();
    }

    auto openFileInDirData(DirData& dd, StringView fn) -> Expected<File> {
        DirEntryLatter* latter = (DirEntryLatter*) ((char*) rawFsMemloc + dd.dir.constPos);

        char* currentOffs = (char*) rawFsMemloc + latter->fStart;

        for (u32 i = 0; i < latter->fileAm; i++) {
            u32 currentSvLen = *(u32*) currentOffs;

            if (!currentSvLen) {
                // EOF marker: fileAm doesn't match actual entries on disk
                return ExpectedErr<File>();
            }

            currentOffs += sizeof(u32);

            StringView sv(currentOffs, currentSvLen);

            u32* contsLen = (u32*) (currentOffs + currentSvLen);

            if (sv == fn) {
                File ret;

                ret.startPos = (u32) (currentOffs - sizeof(u32));

                ret.contsSvPos = (u32) contsLen;

                return ret;
            }

            currentOffs += currentSvLen + sizeof(u32) + *contsLen;
        }

        return ExpectedErr<File>();
    }
};