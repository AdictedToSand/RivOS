#pragma once
#include <str.hpp>
#include <int.h>

#include <gen/vec.hpp>

#include <mem/page.hpp>
#include <mem/mmu/mmu.hpp>

using pid_t = u32;

extern pid_t activeProcessPid;
extern pid_t latestPid;

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

enum class ProcessImportance {
    REQ,
    OPT,
};

struct Process {
    pid_t pid;
    const char* pname;
    Str srcFp;
    ProcessPriveledgeLevel priveledge;
    void* memStart;
    void* memEnd;
    RegisterState state;
    void* entryPoint;
    ProcessImportance importance;

    [[gnu::noreturn]]
    auto run(ProcessImportance iimportance) -> void;
    auto exit(u8 code) -> void;
};

extern Vector<Process*> processes;

auto getNewPid() -> pid_t;
