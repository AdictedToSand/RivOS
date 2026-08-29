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
        Expected<RivFs::File> fe = fs->open("/in/subdir/subdir2/in.txt");
        if (fe.isErr()) {
            kpanic("Unable to open file /in.txt in initramfs");
        }
        RivFs::File f = fe.val();
        char* const buf = (char*) KernelAllocator::alloc(fs->filesize(f));
        memset(buf, 0, fs->filesize(f));
        fs->read(f, buf);
        Serial::logf("file contents: '%s'", buf);

        KernelAllocator::free(buf);
    }
};





