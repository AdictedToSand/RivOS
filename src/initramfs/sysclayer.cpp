#include <initramfs/sysclayer.hpp>

#include <gen/serial.hpp>
#include <gen/map.hpp>

struct [[gnu::packed]] SyscInterruptFrame {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 eip;
    u32 cs;
    u32 eflags;
};

enum LiveSyscNumbers {
    // These are predefined and exposed by the kernel itself
    REG_SYSCALL = 0,
    REG_FS,
    ALLOC,
    FREE,

    KRN_SYSC_END_SPEC,

    IO_SERIAL_WRITE,
    IO_ICTR_WRITECAT,
    IO_ICTR_SETCURSPOS,
    IO_ICTR_CLS,

    IO_GPU_GETKIND,
    IO_GPU_GETCOMMSTRUCT,
    
    IO_MONITOR_GETMONITORCOUNT,
    IO_MONITOR_GETMONITORINFO,

    STORAGE_GETDRIVES,
    STORAGE_DRIVESEL,
    STORAGE_READSECTOR,
    STORAGE_WRITESECTOR,

    // The keyboard drivers should not be exposed via syscall 
    FS_OPEN,
    FS_WRITE,
    FS_READ,
    FS_CLOSE,

    SIZESPEC,
};

Map<u32, u32 (*)(u32 p1, u32 p2, u32 p3)> syscLayerMapping = {};

extern "C" u32 liveSyscHandlr(SyscInterruptFrame* frame) {
    switch (frame->eax) {
        case REG_SYSCALL: {
            if (frame->edi >= SIZESPEC || frame->edi < KRN_SYSC_END_SPEC) {
                Serial::logf("Invalid REG_SYSCALL command: tried to set syscall %u but max was %u (and minimum %u)", frame->edi, SIZESPEC, KRN_SYSC_END_SPEC);
                break;
            }
            if (syscLayerMapping.exists(frame->edi)) {
                syscLayerMapping.rmkey(frame->edi);
            }
            syscLayerMapping.insert(frame->edi, (u32 (*)(u32, u32, u32)) frame->esi);
            break;
        }
        case ALLOC: {
            return (u32) KernelAllocator::alloc(frame->edi);
        }
        case FREE: {
            KernelAllocator::free((void*) frame->edi);
            break;
        }
        default: {
            if (frame->eax >= SIZESPEC) { kpanic("Invalid syscall in driver"); }
            return syscLayerMapping[frame->eax](frame->edi, frame->esi, frame->edx);
        }
    }

    return 0;
}
