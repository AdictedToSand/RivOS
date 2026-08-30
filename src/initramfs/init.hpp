#pragma once
#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <gen/err.hpp>

#include <sv.hpp>
#include <int.h>

#include <terminal/terminal.hpp>

#include <drivers/fs/rivfs/rivfs.hpp>

struct InitRamFs {
private:
    struct [[gnu::packed]] ModuleTag {
        u32 type;
        u32 size;

        u32 modStart;
        u32 modEnd;
        char cmdline[];
    };
    struct [[gnu::packed]] MultibootTag {
        u32 type;
        u32 size;
    };
    static inline ModuleTag* initramfsTag;

    static inline RivFs* fs;
public:
    static auto init(u32 mbiAddr) -> void {
        initramfsTag = nullptr;
        MultibootTag* mbTag = (MultibootTag*) (mbiAddr + 8);
        
        while (mbTag->type != 0) {
            if (mbTag->type == 3) {
                initramfsTag = (ModuleTag*) mbTag;
                break;
            }

            mbTag = (MultibootTag*) ((u8*) mbTag + ((mbTag->size + 7) & ~7));
        }

        if (!initramfsTag) {
            kpanic("Unable to find initramfs module!");
        }

        Serial::logf("CmdLine='%s'", initramfsTag->cmdline);

        if (!streq(initramfsTag->cmdline, "IS_INITRAMFS")) {
            kpanic("A module was found, however the command line does NOT eq IS_INITRAMFS. Please check for extra modules or whether /boot/initramfs.img exists (and/or is loaded somewhere.) NOTE: This is TODO");            
        }

        u8* data = (u8*) initramfsTag->modStart;
        u32 size = initramfsTag->modEnd - initramfsTag->modStart;

        Serial::logf("[CONTS OF INITRAMFS ON NEXT LINE]: ");
        for (u32 i = 0; i < size; i++) {
            Serial::write(data[i]);            
        }
        Serial::write('\n');

        fs = (RivFs*) KernelAllocator::alloc(sizeof(RivFs));
        if (!fs) {
            kpanic("MemAlloc failed on initramfs");
        }

        *fs = RivFs((void*) initramfsTag->modStart, initramfsTag->modEnd - initramfsTag->modStart);

        if (!fs->checkCorrectness()) {
            kpanic("Corrrupt initramfs. Serial log might have more information as to why.");
        }
        Expected<RivFs::DirIterator> eDirit = fs->getDirIt("/drv");
        if (eDirit.isErr()) {
            kpanic("Unable to get /drv");
        }
        RivFs::DirIterator dirIt = eDirit.val();
        
        auto dirItNextFile = fs->dirItGetNextFile(dirIt);
        while (!dirItNextFile.isErr()) {
            RivFs::FileData sum = dirItNextFile;

            char* const data = (char*) sum.filename.toCStr();
            Serial::logf("file: %s", data);
            KernelAllocator::free(data);

            dirItNextFile = fs->dirItGetNextFile(dirIt);
        }
        auto dirItNextDir = fs->dirItGetNextDir(dirIt);
        while (!dirItNextDir.isErr()) {
            RivFs::DirData dir = dirItNextDir.val();

            char* const data = (char*) dir.dirname.toCStr();
            Serial::logf("Dir: %s", data);
            if (dir.dirname == StringView("fs")) {
                Expected<RivFs::File> fe = fs->openFileInDirData(dir, StringView("idk.txt"));

                if (fe.isErr()) {
                    Serial::logf("NOOOOOO");
                    break;
                }
                RivFs::File f = fe.val();
                char* buf = (char*) KernelAllocator::alloc(fs->filesize(f) + 1);
                memset(buf, fs->filesize(f) + 1, 0);
                fs->read(f, buf);
                Serial::logf("Conts='%s'", buf);
                KernelAllocator::free(buf);
            }
            KernelAllocator::free(data);

            dirItNextDir = fs->dirItGetNextDir(dirIt);
        }
    }
};





