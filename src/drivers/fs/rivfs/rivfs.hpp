#pragma once
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
    };

    void* rawFsMemloc;
    u32 rawFsSize;

    static constexpr char INITIAL_MAGIC[] = "rivfs";
    static inline FilePositionHeader* fph;
public:
    struct File {
        friend struct RivFs;

        private:
        u32 startPos; // Name sv starts here
        u32 contsSvPos; // filesize starts here (and conts after it)
    };

    RivFs(void* memloc, u32 size) :
        rawFsMemloc(memloc), rawFsSize(size) {}

    auto checkCorrectness() -> bool {
        auto* hdr = (BaseHdr*) rawFsMemloc;  

        if (memcmp(hdr->magic, INITIAL_MAGIC, sizeof(INITIAL_MAGIC) - 1) != 0) {
            Serial::logf("Invalid magic");
            return false;
        }
        if (hdr->fphSize != sizeof(FilePositionHeader)) {
            Serial::logf("fphSize != sizeof(FilePositionHeader): sizeof(fph)=%u and fphSize=%u", 
                sizeof(FilePositionHeader), hdr->fphSize);
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
        fph = (FilePositionHeader*) ((u8*) rawFsMemloc + /* Alr u32 */hdr->fphPos);
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
    auto findInDir(void* dirEntry, StringView fn, bool isDir) -> Expected<File> {
        const char* const _fnCStrTmp = fn.toCStr();

        const u32 partUntilLen = strlenSpecChar(_fnCStrTmp, '/');

        u32 ownFilenamePart = *(u32*) dirEntry;
        DirEntryLatter* latterDirEntry = (DirEntryLatter*) ((char*) dirEntry + sizeof(u32) + ownFilenamePart);

        const u32 nameLen = partUntilLen ? partUntilLen : fn.len;
        StringView neededFilenameToFind(fn.raw, nameLen);

        if (!countOccurence(_fnCStrTmp, '/') && !isDir) {
            KernelAllocator::free((void*) _fnCStrTmp);
            char* currentOffs = (char*) rawFsMemloc + latterDirEntry->fStart;
            for (u32 i = 0; i < latterDirEntry->fileAm; i++) {
                u32 currentSvLen = *(u32*) currentOffs; 

                if (!currentSvLen) {
                    return ExpectedErr<File>();
                }

                currentOffs += sizeof(u32);
                StringView sv((char*) currentOffs, currentSvLen);
                if (sv == neededFilenameToFind) {
                    File ret;
                    ret.startPos = (u32) currentOffs - sizeof(u32);
                    ret.contsSvPos = (u32) currentOffs + currentSvLen;
                    return ret;
                }
                currentOffs += currentSvLen;
            }
        }
        else {
            KernelAllocator::free((void*) _fnCStrTmp);
            StringView neededDirnameToFind = neededFilenameToFind;

            char* currentOffs = (char*) rawFsMemloc + latterDirEntry->dirStart;
            for (u32 i = 0; i < latterDirEntry->dirHdrAm; i++) {
                char* childDirEntry = currentOffs;
                u32* currentSvLen = (u32*) currentOffs;
                if (!*currentSvLen) {
                    break;
                }

                currentOffs += sizeof(u32);
                StringView sv(currentOffs, *currentSvLen);
                currentOffs += *currentSvLen;
                if (sv == neededDirnameToFind) {
                    u32 consumed = neededDirnameToFind.len + 1;
                    StringView remaining(fn.raw + consumed, fn.len - consumed);

                    // This IS a memory leak but I aint gonna fix it
                    return findInDir(childDirEntry, remaining, countOccurence(remaining.toCStr(), '/') != 0);
                }
            }
        }
        return ExpectedErr<File>();
    }

    auto open(const char* fp) -> Expected<File> {
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


        for (; *fp;) {
            u32 lenUntilSlash = strlenSpecChar(fp, '/');

            if (!lenUntilSlash) {
                svArr.pushBack(FilePart(StringView(fp, strlen(fp)), false));
                break;
            }
            svArr.pushBack(FilePart(StringView(fp, lenUntilSlash), true)); 
            fp += lenUntilSlash;
        }

        if (svArr.size() == 0) {
            Serial::log("svArr.len == 0");
            return ExpectedErr<File>();
        }
        else if (svArr.size() == 1) {
            // use the FPH
            Serial::log("Using FPH");
            char* currentOffs = (char*) rawFsMemloc + (u32) fph->fstart;
            for (u32 i = 0; i < fph->fAm; i++) {
                u32 currentSvLen = *(u32*) currentOffs; 

                if (!currentSvLen) {
                    return ExpectedErr<File>();
                }

                currentOffs += sizeof(u32);
                StringView sv((char*) currentOffs, currentSvLen);
                Serial::logf("Cmp=%s,%s", sv.toCStr(), svArr[0].val().sv.toCStr());
                if (sv == svArr[0].val().sv) {
                    ret.startPos = (u32) currentOffs - sizeof(u32);
                    ret.contsSvPos = (u32) currentOffs + currentSvLen;
                    return ret;
                }
                currentOffs += currentSvLen;
            }
        }
        //TODO: ...
        else if (svArr[0].val().isDir) {
            char* currentOffs = (char*) rawFsMemloc + fph->dirstart;
            for (u32 i = 0; i < fph->dirHdrAm; i++) {
                u32* currentDirNameLen = (u32*) currentOffs;
                currentOffs += sizeof(u32);

                StringView dname(currentOffs, *currentDirNameLen);
                currentOffs += *currentDirNameLen;

                if (dname == svArr[0].val().sv) {
                    void* newDirEntry = currentDirNameLen;
                    const u32 lenUntil = strlenSpecChar(orginFp + 1, '/');
                    const char* tmp = orginFp + 1 + lenUntil + 1;
                    StringView newFp(tmp, strlen(orginFp) - 1 - lenUntil - 1);
                    // Not gonna fix the memleak, idc
                    return findInDir(newDirEntry, newFp, countOccurence(svArr[0].val().sv.toCStr(), '/'));
                }
            }
        }
        else {
            char* currentOffs = (char*) rawFsMemloc + fph->fstart;
            for (u32 i = 0; i < fph->fAm; i++) {
                u32* currentFnLen = (u32*) currentOffs;
                currentOffs += sizeof(u32);

                StringView fname(currentOffs, *currentFnLen);
                currentOffs += *currentFnLen;

                if (fname == svArr[0].val().sv) {
                    ret.contsSvPos =  (u32) currentOffs;
                    ret.startPos = (u32) currentFnLen;
                    return ret;
                }
            }
        }

        return ExpectedErr<File>();
    }
    auto filesize(File f) -> uint32_t {
        u32* filesizeStart = (u32*) f.contsSvPos;
        return *filesizeStart;
    }
    auto read(File f, char* obuf) -> void {
        strcpyLen(obuf, (const char*) (f.contsSvPos + sizeof(u32)), filesize(f));
    }
};
