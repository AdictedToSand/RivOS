#include <int.h>
#include <stdbool.h>
#include <stddef.h>

#include "sys/sys.h"
#include "sys/stdio.h"

#include "rap/rap.h"

#include "scheduling/pitHandler.h"

typedef struct Str {
    char* cStr;
    u32 len;
    u32 capacity;
} Str;

enum {
    PROC_PRIV_LVL_User,
    PROC_PRIV_LVL_Driver,
    PROC_PRIV_LVL_Kernel,
};

typedef u8 ProcessPriveledgeLevel;

#define VEC(T) struct { \
    T* arr; \
    u32 len; \
    u32 capacity; \
}

typedef struct PhysToVirt {
    void* phys;
    void* virt;
} PhysToVirt;

// MAKE SURE THIS IS SYNCED
typedef struct RegisterState {
    /*
    u32 eip;
    u32 esp;
    u32 ebp;
    Right now, not required, however this should be done later
    */
} RegisterState;

enum {
    PROCIMPT_REQ,
    PROCIMPT_OPT,
};

typedef u8 ProcessImportance;

typedef struct Process {
    pid_t pid;
    const char* pname;
    Str srcFp;
    ProcessPriveledgeLevel priveledge;
    VEC(PhysToVirt) pages;
    VEC(PhysToVirt) stackPages;
    RegisterState* state;
    void* entryPoint;
    ProcessImportance importance;
    u32* pageDirectory;
} Process;

typedef struct ProcessList {
    Process** arr;
    u32 len;
    u32 capacity;
} ProcessList;

RapFile rap;
const ProcessList* procList = NULL;
VEC(int) s;

typedef Process* (*LoadProcessT)(const char* fp, const char* pname, ProcessPriveledgeLevel privlvl);

[[gnu::noreturn]]
void _start() {
    sysInit();
    if (claim("PIT")) {
        puts("Unable to claim PIT");
        exit(1);
    }
    /*
    if (claim("SyscallDone")) {
        puts("Unable to claim SyscallDone");
        exit(1);
    }*/
    setFunc("PIT", pitHandler);
    rap = parseRap();

    LoadProcessT loadProcess = getRapAddr(&rap, "loadProcess");
    void (*runProcess)(const Process* proc) = getRapAddr(&rap, "runProcess");
    const ProcessList* (*getProcessList)(void) = getRapAddr(&rap, "getProcessList");
    //isInSyscall = getRapAddr(&rap, "isInSyscall");

    procList = getProcessList(); 

    for (u32 i = 0; i < procList->len; i++) {
        const Process* const proc = procList->arr[i];
        printf("Process (pid=%u) name: %s\n", proc->pid, proc->pname);
    }
    const Process* const de = loadProcess("/krn/bin/de", "Vela", PROC_PRIV_LVL_Kernel);
    runProcess(de);
    for (;;) ;
}
