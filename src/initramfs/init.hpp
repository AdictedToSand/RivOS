#pragma once
#include <mem/alloc.hpp>

#include <gen/err.hpp>

#include <sv.hpp>
#include <int.h>

#include <terminal/terminal.hpp>

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

        Serial::logf("'CmdLine=%s'", initramfsTag->cmdline);

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
    }
};





