#include <stddef.h>

#include "pitHandler.h"

#include "../sys/stdio.h"

#include "../gen/string.h"

#define PROCESS_CTXTSWITCH_INTERVAL 50

extern void (*procCtxtSwitch)(Process* proc);
extern pid_t (*getpid)(void);
extern const ProcessList* procList;

u32 currentProcess = 0;
u64 ticks = 0;

void pitHandler(RegisterStatePIT* state) {
    ticks++;

    const pid_t procPid = getpid != NULL ? getpid() : 0;
    if (procPid) {
        // Get the process: the Process MUST exist so no safety checks
        Process* proc;
        for (u32 i = 0; i < procList->len; i++) {
            if (procList->arr[i]->pid == procPid) {
                proc = procList->arr[i];
                break;
            } 
        }
        proc->state->edi = state->edi;
        proc->state->esi = state->esi;
        proc->state->ebp = state->ebp;
        proc->state->esp = state->esp + 12; // The CPU pushes errcode and such, account for them.
        proc->state->ebx = state->ebx;
        proc->state->edx = state->edx;
        proc->state->ecx = state->ecx;
        proc->state->eax = state->eax;
        proc->state->eip = state->eip;
        proc->state->cs  = state->cs;
        proc->state->eflags = state->eflags;
    }
    if (procList && procList->len > 1 && ticks % PROCESS_CTXTSWITCH_INTERVAL == 0) {
        currentProcess ^= 1;

        procCtxtSwitch(procList->arr[currentProcess]);
    }
}
