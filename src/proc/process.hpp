#pragma once
#include <str.hpp>
#include <int.h>

#include <gen/vec.hpp>

using pid_t = u32;

enum class ProcessPriveledgeLevel {
    User,
    Driver,
    Kernel,
};

struct Process {
    pid_t pid;
    const char* pname;
    Str srcFp;
    ProcessPriveledgeLevel priveledge;
};

static inline Vector<Process*> processes;

static inline pid_t latestPid = 1;

static auto getNewPid() -> pid_t {
    return latestPid++;
}
static inline auto addProcess(Process* proc) -> void {
    processes.pushBack(proc);
}
