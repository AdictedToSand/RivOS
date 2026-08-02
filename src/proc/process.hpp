#pragma once
#include <str.hpp>
#include <int.h>

#include <gen/vec.hpp>

#include <mem/page.hpp>
#include <mem/mmu/mmu.hpp>

using pid_t = u32;

static inline pid_t activeProcessPid = 0;
static inline pid_t latestPid = 1;

enum class ProcessPriveledgeLevel {
    User,
    Driver,
    Kernel,
};

struct RegisterState {
    /*
    u32 eip;
    u32 esp;
    u32 ebp;
    Right now, not required, however this should be done later
    */
};

#define STACK_BEGIN ((void*) 0xBFFF0000)
#define STACK_SIZE (0x10000)
#define STACK_END ((u32) STACK_BEGIN - STACK_SIZE)

extern "C" [[gnu::noreturn]] void finalRun(void* pEntry, void* sStart);

struct Process {
    pid_t pid;
    const char* pname;
    Str srcFp;
    ProcessPriveledgeLevel priveledge;
    void* memStart;
    void* memEnd;
    RegisterState state;
    void* entryPoint;

    [[gnu::noreturn]]
    void run() {

        u32 stackPages = (STACK_SIZE + 4095) / 4096;

        for (u32 i = 0; i < stackPages; i++) {
            void* frame = PhysicalFrameAllocator::allocFrame();

            Mmu::mapPage(frame, (void*) ((u32) STACK_BEGIN - i * 4096), Mmu::FLAGS_PRESENT | Mmu::FLAGS_WRITABLE);
        }
        activeProcessPid = pid;

        finalRun(entryPoint, STACK_BEGIN);
    }
};

static inline Vector<Process*> processes;

static auto getNewPid() -> pid_t {
    return latestPid++;
}
static inline auto addProcess(Process* proc) -> void {
    processes.pushBack(proc);
}
