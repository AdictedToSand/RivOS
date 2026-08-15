#pragma once
#include <int.h>

#include "../sys/sys.h"

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
} RegisterState;

typedef struct RegisterStatePIT {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp; // original ESP before PUSHA
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 eip;
    u32 cs;
    u32 eflags;
} RegisterStatePIT;


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
    u32 heapBrk;
} Process;

typedef struct ProcessList {
    Process** arr;
    u32 len;
    u32 capacity;
} ProcessList;

extern const ProcessList* procList;
extern u64 ticks;
void pitHandler(RegisterStatePIT*);
