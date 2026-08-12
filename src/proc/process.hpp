#pragma once
#include <str.hpp>
#include <int.h>

#include <gen/vec.hpp>

#include <mem/page.hpp>
#include <mem/mmu/mmu.hpp>

using pid_t = u32;

extern pid_t activeProcessPid;
extern pid_t latestPid;

enum class ProcessPriveledgeLevel : u8 {
    User,
    Driver,
    Kernel,
};

struct RegisterState {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;

    u32 esi;
    u32 edi;
    u32 ebp;
    u32 esp;

    u32 eip;
    u32 eflags;

    u32 cs;
    u32 ss;

    u32 ds;
    u32 es;
    u32 fs;
    u32 gs;
};

#define STACK_BEGIN ((void*) 0xBFFF0000)
#define STACK_SIZE (0x10000)
#define STACK_END ((u32) STACK_BEGIN - STACK_SIZE)

extern "C" [[gnu::noreturn]] void finalRun(void* pEntry, void* sStart, void* pagedir);

enum class ProcessImportance : u8 {
    REQ,
    OPT,
};

struct PhysToVirt {
    void* phys;
    void* virt;
};

struct Process {
    pid_t pid;
    char* pname;
    Str srcFp;
    ProcessPriveledgeLevel priveledge;
    Vector<PhysToVirt> pages;
    Vector<PhysToVirt> stackPages;
    RegisterState* state;
    void* entryPoint;
    ProcessImportance importance;
    u32* pageDirectory;

    [[gnu::noreturn]]
    auto run(ProcessImportance iimportance) -> void;
    auto exit(u8 code) -> void;
    [[gnu::noreturn]]
    auto ctxtSwitch() -> void;

    Process() = default;
};

extern Vector<Process*> processes;

auto getNewPid() -> pid_t;

auto loadProcessFromFile(const char* fp, const char* procname, ProcessPriveledgeLevel priv) -> Process*;
auto runProcess(Process* proc) -> void;
auto procCtxtSwitch(Process* proc) -> void;

auto getProcessList() -> Vector<Process*>*;
auto getActiveProcessPid() -> pid_t;
