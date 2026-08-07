#include <mem/alloc.hpp>

#include <terminal/terminal.hpp>

#include <gen/serial.hpp>

#include <int.h>

#include <sys/sysmod.hpp>

#include <drivers/fs/fs.hpp>

#include <proc/process.hpp>

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

enum class SyscallNumbers : u8 {
    Open = 0,
    Write = 1,
    Read = 2,
    Close = 3,
    Claim = 4,
    SetFunc = 5,
    Release = 6,
    Mmap = 7,
    Munmap = 8,
    Exit = 9,
    Filesize = 10,
};

extern "C" auto syscallHandler(SyscInterruptFrame* ifrm) -> void {
    const SyscallNumbers syscNum = (SyscallNumbers) ifrm->eax;
    switch (syscNum) {
        case SyscallNumbers::Open: {
            ifrm->eax = FileSystem::open((const char*) ifrm->edi);
            break;
        }
        case SyscallNumbers::Write: {
            ifrm->eax = (u8) FileSystem::write(ifrm->edi, (char*) ifrm->esi, ifrm->edx);
            break;
        }
        case SyscallNumbers::Read: {
            ifrm->eax = (u8) FileSystem::read(ifrm->edi, (char*) ifrm->esi, ifrm->edx);
            break;
        }
        case SyscallNumbers::Close: {
            FileSystem::close(ifrm->edi);
            break;
        }
        case SyscallNumbers::Claim: {
            ifrm->eax = SysModuleHandler::claim((const char*) ifrm->edi);
            break;
        }
        case SyscallNumbers::SetFunc: {
            ifrm->eax = SysModuleHandler::setFunc((const char*) ifrm->edi, (void(*)()) ifrm->esi);
            break;
        }
        case SyscallNumbers::Release: {
            ifrm->eax = SysModuleHandler::release((const char*) ifrm->edi);
            break;
        }
        case SyscallNumbers::Mmap: {
            //TODO: Replace with pageAlloc()
            ifrm->eax = (u32) KernelAllocator::alloc(ifrm->edi);
            break;
        } 
        case SyscallNumbers::Munmap: {
            // TODO: This is not safe
            KernelAllocator::free((void*) ifrm->edi);
            break;
        }
        case SyscallNumbers::Exit: {
            for (u32 i = 0; i < processes.size(); i++) {
                if (processes[i].val()->pid == activeProcessPid) {
                    processes[i].val()->exit((u8) ifrm->edi);
                }
            }

            break;
        }
        case SyscallNumbers::Filesize: {
            ifrm->eax = (u32) FileSystem::fileSize(ifrm->edi);
            break;
        }
        default: {
            ifrm->eax = -1;
            break;
        }
    }
}
