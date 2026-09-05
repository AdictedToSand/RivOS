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

        fs = (RivFs*) KernelAllocator::alloc(sizeof(RivFs));
        if (!fs) {
            kpanic("MemAlloc failed on initramfs");
        }

        *fs = RivFs(data, size);

        if (!fs->checkCorrectness()) {
            kpanic("Corrrupt initramfs. Serial log might have more information as to why.");
        }

        auto drvfsDiritExpected = fs->getDirIt("/drv/fs");
        if (drvfsDiritExpected.isErr()) {
            kpanic("Corrupt initramfs: /drv/fs did not exist");
        }
        auto drvfsDirit = drvfsDiritExpected.valUnchecked();

        while (true) {
            auto expected = fs->dirItGetNextDir(drvfsDirit);

            if (expected.isErr()) {
                break;
            }
            auto dirdata = expected.valUnchecked();

            Serial::logf("Dir='%s'", dirdata.dirname);

            const u32 len = strlen("/drv/fs/") + dirdata.dirname.len + strlen("/conf.cfg") + 1;
            char* filenameBuf = (char*) KernelAllocator::alloc(len);
            if (!filenameBuf) {
                kpanic("Alloc failed");
            }
            memset(filenameBuf, 0, len);
            strcat(filenameBuf, "/drv/fs/");
            char* tmp = (char*) dirdata.dirname.toCStr();
            strcat(filenameBuf, tmp);
            KernelAllocator::free(tmp);
            strcat(filenameBuf, "/conf.cfg");
            Terminal::printf(filenameBuf);

            KernelAllocator::free(filenameBuf);
        }
    }
};





